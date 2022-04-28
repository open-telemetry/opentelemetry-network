#include <channel/buffered_writer.h>
#include <channel/test_channel.h>
#include <collector/kernel/cgroup_handler.h>
#include <common/intake_encoder.h>
#include <generated/flowmill/ingest/otlp_log_encoder.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <util/curl_engine.h>
#include <util/json.h>
#include <util/log.h>
#include <util/logger.h>

#include <string>
#include <unordered_map>

#include <uv.h>

const char *dummy_json_response_data = R"delim(
{
  "Id": "ad87fc49c1389b0939d637ae700aa4eb4f51cf7da569551857e80789305a7d90",
  "Created": "2021-11-04T16:54:03.099687792Z",
  "Path": "/srv/entrypoint.sh",
  "Args": [
    "--port=8000",
    "--internal-prom=0.0.0.0:7000",
    "--prom=0.0.0.0:7001",
    "--prom-queue-count=1",
    "--ingest_shard_count=1",
    "--matching_shard_count=1",
    "--aggregation_shard_count=1",
    "--enable-aws-enrichment",
    "--log-console",
    "--debug"
  ],
  "State": {
    "Status": "running",
    "Running": true,
    "Paused": false,
    "Restarting": false,
    "OOMKilled": false,
    "Dead": false,
    "Pid": 4251,
    "ExitCode": 0,
    "Error": "",
    "StartedAt": "2021-11-04T16:54:05.666083454Z",
    "FinishedAt": "0001-01-01T00:00:00Z"
  },
  "Image": "sha256:5c9ce00b94a01bf23a44eaf995f068d1993baefccf45ee37e5448f03f8499ab0",
  "ResolvConfPath": "/var/lib/docker/containers/ad87fc49c1389b0939d637ae700aa4eb4f51cf7da569551857e80789305a7d90/resolv.conf",
  "HostnamePath": "/var/lib/docker/containers/ad87fc49c1389b0939d637ae700aa4eb4f51cf7da569551857e80789305a7d90/hostname",
  "HostsPath": "/var/lib/docker/containers/ad87fc49c1389b0939d637ae700aa4eb4f51cf7da569551857e80789305a7d90/hosts",
  "LogPath": "/var/lib/docker/containers/ad87fc49c1389b0939d637ae700aa4eb4f51cf7da569551857e80789305a7d90/ad87fc49c1389b0939d637ae700aa4eb4f51cf7da569551857e80789305a7d90-json.log",
  "Name": "/amazing_chatterjee",
  "RestartCount": 0,
  "Driver": "overlay2",
  "Platform": "linux",
  "MountLabel": "",
  "ProcessLabel": "",
  "AppArmorProfile": "unconfined",
  "ExecIDs": null,
  "HostConfig": {
    "Binds": [
      "/home/vagrant/src/:/root/src",
      "/home/vagrant/out/:/root/out"
    ],
    "ContainerIDFile": "",
    "LogConfig": {
      "Type": "json-file",
      "Config": {}
    },
    "NetworkMode": "default",
    "PortBindings": {
      "7000/tcp": [
        {
          "HostIp": "",
          "HostPort": "7000"
        }
      ],
      "7001/tcp": [
        {
          "HostIp": "",
          "HostPort": "7001"
        },
        {
          "HostIp": "",
          "HostPort": "7002"
        },
        {
          "HostIp": "",
          "HostPort": "7003"
        }
      ],
      "8000/tcp": [
        {
          "HostIp": "",
          "HostPort": "8000"
        }
      ]
    },
    "RestartPolicy": {
      "Name": "no",
      "MaximumRetryCount": 0
    },
    "AutoRemove": true,
    "VolumeDriver": "",
    "VolumesFrom": null,
    "CapAdd": null,
    "CapDrop": null,
    "CgroupnsMode": "host",
    "Dns": [],
    "DnsOptions": [],
    "DnsSearch": [],
    "ExtraHosts": null,
    "GroupAdd": null,
    "IpcMode": "private",
    "Cgroup": "",
    "Links": null,
    "OomScoreAdj": 0,
    "PidMode": "",
    "Privileged": true,
    "PublishAllPorts": false,
    "ReadonlyRootfs": false,
    "SecurityOpt": [
      "label=disable"
    ],
    "UTSMode": "",
    "UsernsMode": "",
    "ShmSize": 67108864,
    "Runtime": "runc",
    "ConsoleSize": [
      0,
      0
    ],
    "Isolation": "",
    "CpuShares": 0,
    "Memory": 0,
    "NanoCpus": 0,
    "CgroupParent": "",
    "BlkioWeight": 0,
    "BlkioWeightDevice": [],
    "BlkioDeviceReadBps": null,
    "BlkioDeviceWriteBps": null,
    "BlkioDeviceReadIOps": null,
    "BlkioDeviceWriteIOps": null,
    "CpuPeriod": 0,
    "CpuQuota": 0,
    "CpuRealtimePeriod": 0,
    "CpuRealtimeRuntime": 0,
    "CpusetCpus": "",
    "CpusetMems": "",
    "Devices": [],
    "DeviceCgroupRules": null,
    "DeviceRequests": null,
    "KernelMemory": 0,
    "KernelMemoryTCP": 0,
    "MemoryReservation": 0,
    "MemorySwap": 0,
    "MemorySwappiness": null,
    "OomKillDisable": false,
    "PidsLimit": null,
    "Ulimits": null,
    "CpuCount": 0,
    "CpuPercent": 0,
    "IOMaximumIOps": 0,
    "IOMaximumBandwidth": 0,
    "MaskedPaths": null,
    "ReadonlyPaths": null
  },
  "GraphDriver": {
    "Data": {
      "LowerDir": "/var/lib/docker/overlay2/925ab32b43b5e0b27aae80af7cf4edfe129b79e56aca5c41337094eb95b83fea-init/diff:/var/lib/docker/overlay2/835a05eea84b538ad3fa865e50895ce03c59517ea7db69ce7313a3b421321c2d/diff:/var/lib/docker/overlay2/3b05911a571c14d6965e32a2cbe998bf75ae287c5969cef9e9350958ed805b95/diff:/var/lib/docker/overlay2/15982a35f2ac7e3e362106b03e8ece88a0b959407147e483e09dc5674fa1566d/diff:/var/lib/docker/overlay2/1c7fb4b897d78db2bb5da88423bfafc92ea0df95fdce8b8c91316a985fe59738/diff:/var/lib/docker/overlay2/5a32678605742638e9c3f784e9f11eddf36f9c8a1436711feadc685d7f0800ec/diff:/var/lib/docker/overlay2/94cba6304ed8b8f0f3c71b192e100eb51d9fe663f6cbd909adbf71f2a23166a0/diff:/var/lib/docker/overlay2/531befd27476430f7b6df70dde3120cb16eedc9aef0ba86042b097655d3e90f4/diff:/var/lib/docker/overlay2/0c1ef5b3e5063760f7f87674f5957937ab46f5162b5bffaad5ead3df0b6b3657/diff:/var/lib/docker/overlay2/85ac7b1e505a2705a28d2a24d16e905035a851c60e09f8949d498dbe5590dcd0/diff",
      "MergedDir": "/var/lib/docker/overlay2/925ab32b43b5e0b27aae80af7cf4edfe129b79e56aca5c41337094eb95b83fea/merged",
      "UpperDir": "/var/lib/docker/overlay2/925ab32b43b5e0b27aae80af7cf4edfe129b79e56aca5c41337094eb95b83fea/diff",
      "WorkDir": "/var/lib/docker/overlay2/925ab32b43b5e0b27aae80af7cf4edfe129b79e56aca5c41337094eb95b83fea/work"
    },
    "Name": "overlay2"
  },
  "Mounts": [
    {
      "Type": "bind",
      "Source": "/home/vagrant/src",
      "Destination": "/root/src",
      "Mode": "",
      "RW": true,
      "Propagation": "rprivate"
    },
    {
      "Type": "bind",
      "Source": "/home/vagrant/out",
      "Destination": "/root/out",
      "Mode": "",
      "RW": true,
      "Propagation": "rprivate"
    }
  ],
  "Config": {
    "Hostname": "ad87fc49c138",
    "Domainname": "",
    "User": "",
    "AttachStdin": false,
    "AttachStdout": true,
    "AttachStderr": true,
    "ExposedPorts": {
      "7000/tcp": {},
      "7001/tcp": {},
      "7010/tcp": {},
      "8000/tcp": {}
    },
    "Tty": true,
    "OpenStdin": false,
    "StdinOnce": false,
    "Env": [
      "FLOWMILL_TENANT_ID=100001",
      "FLOWMILL_RUN_UNDER_GDB=",
      "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
      "OPENSSL_CONF=/etc/openssl/openssl.cnf",
      "GEOIP_PATH=/usr/share/GeoIP"
    ],
    "Cmd": [
      "--port=8000",
      "--internal-prom=0.0.0.0:7000",
      "--prom=0.0.0.0:7001",
      "--prom-queue-count=1",
      "--ingest_shard_count=1",
      "--matching_shard_count=1",
      "--aggregation_shard_count=1",
      "--enable-aws-enrichment",
      "--log-console",
      "--debug"
    ],
    "Image": "localhost:5000/pipeline-server",
    "Volumes": null,
    "WorkingDir": "",
    "Entrypoint": [
      "/srv/entrypoint.sh"
    ],
    "OnBuild": null,
    "Labels": {
      "org.label-schema.description": "Flowtune telemetry server",
      "org.label-schema.name": "flowtune/flowtune-server",
      "org.label-schema.schema-version": "1.0",
      "org.label-schema.usage": "./README.md",
      "org.label-schema.vcs-url": "https://review.prod.flowtune.io/deploy/flowtune-server-docker/",
      "org.label-schema.vendor": "Flowmill, Inc"
    }
  },
  "NetworkSettings": {
    "Bridge": "",
    "SandboxID": "da32f06c760e26fb0c9a476711bc83b76c5d12513ab45d38076c7f2359782d38",
    "HairpinMode": false,
    "LinkLocalIPv6Address": "",
    "LinkLocalIPv6PrefixLen": 0,
    "Ports": {
      "7000/tcp": [
        {
          "HostIp": "0.0.0.0",
          "HostPort": "7000"
        },
        {
          "HostIp": "::",
          "HostPort": "7000"
        }
      ],
      "7001/tcp": [
        {
          "HostIp": "0.0.0.0",
          "HostPort": "7003"
        },
        {
          "HostIp": "::",
          "HostPort": "7003"
        },
        {
          "HostIp": "0.0.0.0",
          "HostPort": "7002"
        },
        {
          "HostIp": "::",
          "HostPort": "7002"
        },
        {
          "HostIp": "0.0.0.0",
          "HostPort": "7001"
        },
        {
          "HostIp": "::",
          "HostPort": "7001"
        }
      ],
      "7010/tcp": null,
      "8000/tcp": [
        {
          "HostIp": "0.0.0.0",
          "HostPort": "8000"
        },
        {
          "HostIp": "::",
          "HostPort": "8000"
        }
      ]
    },
    "SandboxKey": "/var/run/docker/netns/da32f06c760e",
    "SecondaryIPAddresses": null,
    "SecondaryIPv6Addresses": null,
    "EndpointID": "29e7a05fc7beb59d78b2262e1290751ab4fe2b21e4a9711a0793e558ab57a43c",
    "Gateway": "172.17.0.1",
    "GlobalIPv6Address": "",
    "GlobalIPv6PrefixLen": 0,
    "IPAddress": "172.17.0.2",
    "IPPrefixLen": 16,
    "IPv6Gateway": "",
    "MacAddress": "02:42:ac:11:00:02",
    "Networks": {
      "bridge": {
        "IPAMConfig": null,
        "Links": null,
        "Aliases": null,
        "NetworkID": "2fa7167f0c612eda844cb3e9465e92108b5d10a4304033191007c00916eb661e",
        "EndpointID": "29e7a05fc7beb59d78b2262e1290751ab4fe2b21e4a9711a0793e558ab57a43c",
        "Gateway": "172.17.0.1",
        "IPAddress": "172.17.0.2",
        "IPPrefixLen": 16,
        "IPv6Gateway": "",
        "GlobalIPv6Address": "",
        "GlobalIPv6PrefixLen": 0,
        "MacAddress": "02:42:ac:11:00:02",
        "DriverOpts": null
      }
    }
  }
}
)delim";

/* Sample data received by the writer from the above dummy response_data:
{
  "resourceLogs": [
    {
      "instrumentationLibraryLogs": [
        {
          "log_records": [
            {
              "name": "container_annotation",
              "timeUnixNano": "800022144477563",
              "body": {
                "kvlist_value": {
                  "values": [
                    {
                      "key": "cgroup",
                      "value": {
                        "stringValue": "1"
                      }
                    },
                    {
                      "key": "key",
                      "value": {
                        "stringValue": "org.label-schema.description"
                      }
                    },
                    {
                      "key": "value",
                      "value": {
                        "stringValue": "Flowtune telemetry server"
                      }
                    }
                  ]
                }
              }
            }
          ]
        }
      ]
    }
  ]
}
*/

class CgroupHandlerTest : public ::testing::Test {
protected:
  void SetUp() override { ASSERT_EQ(0, uv_loop_init(&loop_)); }

  void TearDown() override
  {
    // Clean up loop_ to avoid valgrind and asan complaints about memory leaks.
    close_uv_loop_cleanly(&loop_);
  }

  uv_loop_t loop_;
};

TEST_F(CgroupHandlerTest, handle_docker_response)
{
  channel::TestChannel test_channel(std::nullopt, IntakeEncoder::otlp_log);
  channel::BufferedWriter buffered_writer(test_channel, 1024);
  flowmill::ingest::OtlpLogEncoder encoder("host-not-used", "port-not-used");
  flowmill::ingest::Writer writer(buffered_writer, monotonic, 0, &encoder);

  std::unique_ptr<CurlEngine> curl_engine = CurlEngine::create(&loop_);

  CgroupHandler::CgroupSettings cgroup_settings;

  logging::Logger logger(writer);

  CgroupHandler cgroup_handler(writer, *curl_engine.get(), cgroup_settings, logger);

  nlohmann::json const dummy_json_response_object = nlohmann::json::parse(dummy_json_response_data);

  // Populate a map with expected key/value pairs to compare against results from cgroup_handler.handle_docker_response().
  std::unordered_map<std::string, std::string> key_value_map;
  for (auto item : dummy_json_response_object["Config"]["Labels"].items()) {
    key_value_map[item.key()] = item.value().get<std::string>();
  }
  ASSERT_NE(0UL, key_value_map.size());

  // Pass the dummy response_data to CgroupHandler::handle_docker_response().
  cgroup_handler.handle_docker_response(1, dummy_json_response_object.dump());

  LOG::debug("data in channel: {}", log_waive(test_channel.get_ss().str()));

  // Validate that the writer gets the expected values.
  std::string line;
  std::stringstream &ss = test_channel.get_ss();
  while (std::getline(ss, line)) {
    if (line.size() && line[0] == '{') {
      nlohmann::json const object = nlohmann::json::parse(line);
      for (auto const &rl : object["resourceLogs"]) {
        for (auto const &ill : rl["instrumentationLibraryLogs"]) {
          for (auto const &log : ill["log_records"]) {
            std::string key;
            std::string value;
            for (auto const &kv : log["body"]["kvlist_value"]["values"]) {
              auto keyit = kv.find("key");
              if (keyit != kv.end()) {
                if (kv["key"] == "key") {
                  key = kv["value"]["stringValue"];
                  continue;
                } else if (kv["key"] == "value") {
                  value = kv["value"]["stringValue"];
                  continue;
                }
              }
            }
            // if both key and value were found, confirm they are equal to those originally provided, tracked in key_value_map
            if (key.size() && value.size()) {
              LOG::debug("key {}, value: {}", key, value);
              EXPECT_TRUE(key_value_map[key] == value) << "key_value_map[key]=" << key_value_map[key] << " value=" << value;
              key_value_map.erase(key);
            }
          }
        }
      }
    }
  }
  EXPECT_EQ(0UL, key_value_map.size());
}
