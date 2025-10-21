// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/aws_instance_metadata.h>

#include <common/cloud_platform.h>
#include <config.h>
#include <platform/platform.h>
#include <util/json.h>
#include <util/log.h>

#include <curlpp/Easy.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Types.hpp>

#include <sys/select.h>

#include <functional>
#include <sstream>
#include <stdexcept>
#include <list>

#define METADATA_PREFIX_V1 "http://169.254.169.254/2016-09-02/meta-data/"
#define METADATA_PREFIX_V2 "http://169.254.169.254/latest/meta-data/"

#define DYNAMIC_METADATA_PREFIX_V1 "http://169.254.169.254/2016-09-02/dynamic/"
#define DYNAMIC_METADATA_PREFIX_V2 "http://169.254.169.254/latest/dynamic/"

#define IMDSV2_TOKEN_URL "http://169.254.169.254/latest/api/token"
#define IMDSV2_TOKEN_TTL "21600" /* seconds */
#define MAX_LEN 4096
#define PAUSE_USEC_WHEN_NOT_READY (1000)

class Fetch {
public:
  /* c'tor */
  Fetch(const std::string &name, const char *url, const std::string &imdsv2_token = "");

  /* write functor callback from curlpp */
  size_t write(char *buf, size_t size, size_t nmemb);

  std::string key;
  std::string value;
  curlpp::Easy easy;
  int retcode;
};

std::string_view AwsMetadataValue::value() const
{
  if (retcode_ != 0) {
    return {};
  }
  return value_;
}

AwsNetworkInterface::AwsNetworkInterface(std::string interface_id) : interface_id_(interface_id) {}

void AwsNetworkInterface::set_info(FetchResult const &info)
{
  std::string vpc_id_key = interface_id_ + "_vpc_id";
  if (info.at(vpc_id_key).valid()) {
    auto vpc = info.at(vpc_id_key).value();
    // remove the slash at the end of the vpc-id, if one was given
    if ((vpc.length() > 0) && (vpc.back() == '/')) {
      vpc.remove_suffix(1);
    }
    vpc_id_ = vpc;
  }

  // we can't use set_multiline_info here since private_ipv4 is a set (and not a
  // vector)
  if (info.at(interface_id_ + "_private_ipv4").valid()) {
    std::istringstream stream(std::string(info.at(interface_id_ + "_private_ipv4").value()));
    std::string line;
    while (std::getline(stream, line)) {
      private_ipv4s_.emplace(line);
    }
  }
  set_multiline_info(info.at(interface_id_ + "_public_ipv4"), public_ipv4s_);
  set_multiline_info(info.at(interface_id_ + "_ipv6"), ipv6s_);
}

void AwsNetworkInterface::set_mapped_ipv4s(FetchResult const &info)
{
  for (auto public_ipv4 : public_ipv4s_) {
    std::string mapped_ipv4_key = interface_id_ + "_" + public_ipv4;
    if (info.at(mapped_ipv4_key).valid()) {
      mapped_ipv4s_[public_ipv4] = info.at(mapped_ipv4_key).value();
      // remove from our set of private_ipv4s
      private_ipv4s_.erase(std::string(info.at(mapped_ipv4_key).value()));
    }
  }
}

void AwsNetworkInterface::set_multiline_info(AwsMetadataValue const &info, std::vector<std::string> &dest)
{
  if (info.valid()) {
    std::istringstream stream(std::string(info.value()));
    std::string line;
    while (std::getline(stream, line)) {
      dest.emplace_back(line);
    }
  }
}

void AwsNetworkInterface::print_interface() const
{
  LOG::debug_in(CloudPlatform::aws, "Interface Id: {}", interface_id_);
  LOG::debug_in(CloudPlatform::aws, "\tVPC Id: {}", vpc_id_);

  LOG::debug_in(CloudPlatform::aws, "\tPrivate IPV4s:");
  for (auto private_ipv4 : private_ipv4s_) {
    LOG::debug_in(CloudPlatform::aws, "\t\t{}", private_ipv4);
  }

  LOG::debug_in(CloudPlatform::aws, "\tPublic IPV4s -> Private IPV4s:");
  for (auto mapped_ipv4 : mapped_ipv4s_) {
    LOG::debug_in(CloudPlatform::aws, "\t\t{} -> {}", mapped_ipv4.first, mapped_ipv4.second);
  }

  LOG::debug_in(CloudPlatform::aws, "\tIPV6s: ");
  for (auto ipv6 : ipv6s_) {
    LOG::debug_in(CloudPlatform::aws, "\t\t{}", ipv6);
  }
}

AwsMetadata::AwsMetadata(std::chrono::microseconds timeout) : timeout_(timeout) {}

AwsMetadata::FetchResult AwsMetadata::fetch(std::map<std::string, std::string> keys_to_endpoints,
                                            const std::string &imdsv2_token)
{
  std::vector<Fetch *> fetches;
  for (auto key_to_endpoint : keys_to_endpoints) {
    fetches.push_back(new Fetch(key_to_endpoint.first, key_to_endpoint.second.c_str(), imdsv2_token));
  }

  /* add fetches to curlpp::Multi for parallel fetch */
  curlpp::Multi multi;
  for (auto fetch : fetches)
    multi.add(&fetch->easy);

  /* loop until we're finished fetching */
  fd_set fdread;
  fd_set fdwrite;
  fd_set fdexcep;
  struct timeval timeout;
  int maxfd = -1;
  int still_running;
  int rc;

  /* set the timeout. select updates it with the remaining time */
  timeout.tv_sec = timeout_.count() / 1000000;
  timeout.tv_usec = (timeout_.count() % 1000000);

  /* loop while timeout has not passed, to fetch requests */
  while (timeout.tv_sec > 0 || timeout.tv_usec > 0) {
    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* get file descriptors from the transfers */
    multi.fdset(&fdread, &fdwrite, &fdexcep, &maxfd);

    if (maxfd == -1) {
      /* multi needs a bit of time, pause for PAUSE_USEC_WHEN_NOT_READY */
      u64 remaining_usecs = timeout.tv_sec * 1000000 + timeout.tv_usec;
      if (remaining_usecs <= PAUSE_USEC_WHEN_NOT_READY) {
        /* just sleep the rest of the timeout */
        rc = select(1, NULL, NULL, NULL, &timeout);
      } else {
        /* we'll sleep for PAUSE_USEC_WHEN_NOT_READY */
        struct timeval pause;
        pause.tv_sec = PAUSE_USEC_WHEN_NOT_READY / 1000000;
        pause.tv_usec = PAUSE_USEC_WHEN_NOT_READY % 1000000;
        rc = select(1, NULL, NULL, NULL, &pause);

        /* we wanted to sleep for PAUSE_USEC_WHEN_NOT_READY */
        remaining_usecs -= PAUSE_USEC_WHEN_NOT_READY;
        /* account for how much time select did NOT sleep */
        remaining_usecs += (pause.tv_sec * 1000000) + pause.tv_usec;

        /* fix timeout to account for the pause */
        timeout.tv_sec = remaining_usecs / 1000000;
        timeout.tv_usec = remaining_usecs % 1000000;
      }
    } else {
      rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    }

    if (rc == -1) {
      /* cleanup */
      for (auto fetchp : fetches) {
        multi.remove(&fetchp->easy);
        delete fetchp;
      }
      std::stringstream ss;
      ss << "select returned -1\n"
         << "id: " << id() << ",az: " << az() << ",iam_role: " << iam_role() << ",type: " << type() << "\n";
      throw std::runtime_error(ss.str());
    }

    while (!multi.perform(&still_running)) {
    }
    LOG::debug_in(
        CloudPlatform::aws,
        "select {} still_running {} timeout {}",
        rc,
        still_running,
        (timeout.tv_sec * 1000000 + timeout.tv_usec));

    if (still_running == 0)
      break; /* finished all, can stop looping */
  }

  /* we will need to map the Easy * to the Fetch *, create a map */
  std::map<const curlpp::Easy *, Fetch *> curl_to_fetch;
  for (auto fetch : fetches)
    curl_to_fetch[&fetch->easy] = fetch;

  /* parse info messages */
  for (auto info : multi.info()) {
    auto fetchp = curl_to_fetch[info.first];
    fetchp->retcode = info.second.code;
  }

  /* Create result map */
  std::map<std::string, AwsMetadataValue> ret;
  for (auto fetchp : fetches)
    ret.emplace(
        std::piecewise_construct, std::forward_as_tuple(fetchp->key), std::forward_as_tuple(fetchp->value, fetchp->retcode));

  /* cleanup */
  for (auto fetchp : fetches) {
    multi.remove(&fetchp->easy);
    delete fetchp;
  }

  return ret;
}

std::string AwsMetadata::fetch_imdsv2_token()
{
  try {
    curlpp::Easy easy;
    std::string token;

    easy.setOpt(curlpp::options::Url(IMDSV2_TOKEN_URL));
    /* IMDSv2 token requires a PUT to /latest/api/token with TTL header */
    easy.setOpt(curlpp::options::CustomRequest("PUT"));

    std::list<std::string> headers;
    headers.push_back(std::string("X-aws-ec2-metadata-token-ttl-seconds: ") + IMDSV2_TOKEN_TTL);
    easy.setOpt(curlpp::options::HttpHeader(headers));

    /* write the response into token string */
    curlpp::types::WriteFunctionFunctor functor =
        [&](char *buf, size_t size, size_t nmemb) -> size_t {
          size_t len = size * nmemb;
          if (len > 0) token.append(buf, len);
          return len;
        };
    easy.setOpt(curlpp::options::WriteFunction(functor));

    /* don't throw for small network glitches; we still want to fallback to v1 */
    easy.setOpt(curlpp::options::FailOnError(true));

    /* set a short timeout so token fetch doesn't stall whole metadata retrieval */
    easy.setOpt(curlpp::options::Timeout(2)); // 2 seconds; tune as you wish

    easy.perform();

    if (!token.empty()) {
      // strip trailing newline (if any)
      if (!token.empty() && token.back() == '\n') token.pop_back();
      return token;
    }
  } catch (const std::exception &e) {
    // token fetch failed; we will fall back to IMDSv1
    LOG::debug_in(CloudPlatform::aws, "IMDSv2 token fetch failed: {}", e.what());
  } catch (...) {
    LOG::debug_in(CloudPlatform::aws, "IMDSv2 token fetch unknown failure");
  }

  return std::string{}; // empty => fall back to IMDSv1
}

Expected<AwsMetadata, std::runtime_error> AwsMetadata::fetch(std::chrono::microseconds timeout)
{
  AwsMetadata metadata{timeout};

  try {
    if (!metadata.fetch_aws_instance_metadata()) {
      return {unexpected, "no metadata returned by AWS"};
    }
  } catch (std::exception const &e) {
    return {unexpected, e.what()};
  }

  return std::move(metadata);
}

bool AwsMetadata::fetch_aws_instance_metadata()
{
  // try IMDSv2 token first; if empty => we'll do IMDSv1 GETs (fallback)
  std::string token = fetch_imdsv2_token();
  bool using_imdsv2 = !token.empty();

  if (using_imdsv2) {
    LOG::debug_in(CloudPlatform::aws, "IMDSv2 token obtained, using IMDSv2 for metadata calls");
  } else {
    LOG::debug_in(CloudPlatform::aws, "No IMDSv2 token; falling back to IMDSv1 for metadata calls");
  }

  // Choose the appropriate metadata & dynamic prefixes based on IMDS version.
  // Use std::string so runtime concatenation works.
  std::string metadata_prefix = using_imdsv2 ? std::string(METADATA_PREFIX_V2) : std::string(METADATA_PREFIX_V1);
  std::string dynamic_prefix = using_imdsv2 ? std::string(DYNAMIC_METADATA_PREFIX_V2) : std::string(DYNAMIC_METADATA_PREFIX_V1);

  /* add all the Fetch object we want to get */
  std::map<std::string, std::string> fetches;
  fetches["az"] = metadata_prefix + "placement/availability-zone";
  fetches["iam-role"] = metadata_prefix + "iam/security-credentials/";
  fetches["id"] = metadata_prefix + "instance-id";
  fetches["type"] = metadata_prefix + "instance-type";
  fetches["interfaces"] = metadata_prefix + "network/interfaces/macs/";
  fetches["instance-identity"] = dynamic_prefix + "instance-identity/document";

  FetchResult ret = fetch(fetches, token);

  // set general instance metadata
  id_ = ret["id"];
  az_ = ret["az"];
  iam_role_ = ret["iam-role"];
  type_ = ret["type"];

  if (auto const &identity = ret["instance-identity"]; identity.valid()) {
    nlohmann::json const instance_identity(identity.value());

    if (auto account_id = try_get_string(instance_identity, "accountId")) {
      account_id_.set(*account_id);
      LOG::trace_in(CloudPlatform::aws, "retrieved AWS account id: {}", *account_id);
    }
  }

  if (!id_.valid() && !az_.valid() && !iam_role_.valid() && !type_.valid()) {
    return false;
  }

  // set network interfaces
  if (ret.at("interfaces").valid()) {
    std::istringstream stream(std::string(ret.at("interfaces").value()));
    std::string line;
    while (std::getline(stream, line)) {
      network_interfaces_.emplace_back(line);
    }
  }

  // get vpc_id, private/public ipv4, and ipv6 for each network interface
  fetches.clear();
  for (auto interface : network_interfaces_) {
    std::string vpc_url = metadata_prefix + "network/interfaces/macs/" + interface.id() + "/vpc-id";
    std::string local_ipv4_url = metadata_prefix + "network/interfaces/macs/" + interface.id() + "/local-ipv4s";
    std::string public_ipv4_url = metadata_prefix + "network/interfaces/macs/" + interface.id() + "/public-ipv4s";
    std::string ipv6_url = metadata_prefix + "network/interfaces/macs/" + interface.id() + "/ipv6s";
    fetches[interface.id() + "_vpc_id"] = vpc_url;
    fetches[interface.id() + "_private_ipv4"] = local_ipv4_url;
    fetches[interface.id() + "_public_ipv4"] = public_ipv4_url;
    fetches[interface.id() + "_ipv6"] = ipv6_url;
  }
  ret = fetch(fetches, token);

  for (auto &interface : network_interfaces_) {
    interface.set_info(ret);
  }

  // get public-private ipv4 mappings for each network interface
  fetches.clear();
  for (auto &interface : network_interfaces_) {
    for (auto public_ip : interface.public_ipv4s()) {
      std::string url = metadata_prefix + "network/interfaces/macs/" + interface.id() + "/ipv4-associations/" + public_ip;
      fetches[interface.id() + "_" + public_ip] = url;
    }
  }
  ret = fetch(fetches, token);

  for (auto &interface : network_interfaces_) {
    interface.set_mapped_ipv4s(ret);
  }

  return true;
}

void AwsMetadata::print_instance_metadata() const
{
  LOG::info("  AZ: {} (valid: {})", az_.value(), az_.valid());
  LOG::info("  IAM role: {} (valid: {})", iam_role_.value(), iam_role_.valid());
  LOG::info("  Instance ID: {} (valid: {})", id_.value(), id_.valid());
  LOG::info("  Instance type: {} (valid: {})", type_.value(), type_.valid());
  LOG::info("  Account ID: {} (valid: {})", account_id_.value(), account_id_.valid());
}

void AwsMetadata::print_interfaces() const
{
  for (auto interface : network_interfaces_) {
    interface.print_interface();
  }
}

Fetch::Fetch(const std::string &name, const char *url, const std::string &imdsv2_token)
    : key(name), retcode(-1)
{
  /* set the URL */
  easy.setOpt(curlpp::options::Url(url));

  /* set the write callback */
  using namespace std::placeholders;
  curlpp::types::WriteFunctionFunctor functor = std::bind(&Fetch::write, this, _1, _2, _3);
  easy.setOpt(curlpp::options::WriteFunction(functor));

  /* We want to get a failure on failed HTTP retcode (404, etc) */
  easy.setOpt(curlpp::options::FailOnError(true));

  /* If we have an IMDSv2 token, send it as header for this request */
  if (!imdsv2_token.empty()) {
    std::list<std::string> headers;
    headers.push_back(std::string("X-aws-ec2-metadata-token: ") + imdsv2_token);
    easy.setOpt(curlpp::options::HttpHeader(headers));
  }
}

size_t Fetch::write(char *buf, size_t size, size_t nmemb)
{
  size_t len = size * nmemb;

  /* is size sane? */
  if (len <= 0)
    return 0;

  /* if resulting value would be too big, only take the remaining size */
  if (len + value.size() > MAX_LEN) {
    len = MAX_LEN - value.size();
  }

  /* sanity check */
  if (len <= 0)
    return 0;

  /* ok, can append. will throw if there's a problem */
  value.append(buf, len);
  return len;
}

std::ostream &operator<<(std::ostream &os, const AwsMetadataValue &val)
{
  os << "AWS-Meta(valid=" << val.valid() << " value='" << val.value() << "')";
  return os;
}
