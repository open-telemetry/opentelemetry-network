
/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include <limits.h>
#include <string.h>
#include <strings.h>

#include "ares.h"
#include "ares_dns.h"

#include <collector/kernel/dns/c_ares_nameser.h>
#include <collector/kernel/dns/dns.h>

/**
 * The hostname of the reply will be written to @hostname_out, and its length
 *   to @hostname_len. @hostname_out must be at least DNS_NAME_MAX_LENGTH
 *   characters long.
 */

int dns_parse_a_aaaa_reply(
    const unsigned char *abuf,
    int alen,
    char *hostname_out,
    int *hostname_len,
    struct in_addr *in_addrs_out,
    int *num_in_addrs,
    struct in6_addr *in6_addrs_out,
    int *num_in6_addrs)
{
  unsigned int qdcount, ancount;
  int status, i, rr_type, rr_class, rr_len, rr_ttl, naddrs, n6addrs;
  int cname_ttl = INT_MAX; /* the TTL imposed by the CNAME chain */
  int naliases;
  long len;
  const unsigned char *aptr;
  char *hostname = hostname_out;
  char rr_name[DNS_NAME_MAX_LENGTH], rr_data[DNS_NAME_MAX_LENGTH];
  const int max_in_addrs = (in_addrs_out && num_in_addrs) ? *num_in_addrs : 0;
  const int max_in6_addrs = (in6_addrs_out && num_in6_addrs) ? *num_in6_addrs : 0;

  /* Set number of addresses returned to NULL for all failure cases. */
  if (num_in_addrs) {
    *num_in_addrs = 0;
  }
  if (num_in6_addrs) {
    *num_in6_addrs = 0;
  }

  /* Give up if abuf doesn't have room for a header. */
  if (alen < HFIXEDSZ) {
    return ARES_EBADRESP;
  }

  /* Ensure this is a response */
  if (DNS_HEADER_QR(abuf) != 1) {
    return ARES_EBADRESP;
  }

  /* Fetch the question and answer count from the header. */
  qdcount = DNS_HEADER_QDCOUNT(abuf);
  ancount = DNS_HEADER_ANCOUNT(abuf);
  if (qdcount != 1) {
    return ARES_EBADRESP;
  }

  /* Expand the name from the question, and skip past the question. */
  aptr = abuf + HFIXEDSZ;
  status = dns_expand_name_maxlen(aptr, abuf, alen, hostname, &len, hostname_len);
  if (status != ARES_SUCCESS) {
    return status;
  }
  if (aptr + len + QFIXEDSZ > abuf + alen) {
    return ARES_EBADRESP;
  }
  aptr += len + QFIXEDSZ;

  naddrs = 0;
  n6addrs = 0;
  naliases = 0;

  /* Examine each answer resource record (RR) in turn. */
  for (i = 0; i < (int)ancount; i++) {
    /* Decode the RR up to the data field. */
    status = dns_expand_name_maxlen(aptr, abuf, alen, rr_name, &len, NULL);
    if (status != ARES_SUCCESS) {
      break;
    }
    aptr += len;
    if (aptr + RRFIXEDSZ > abuf + alen) {
      status = ARES_EBADRESP;
      break;
    }
    rr_type = DNS_RR_TYPE(aptr);
    rr_class = DNS_RR_CLASS(aptr);
    rr_len = DNS_RR_LEN(aptr);
    rr_ttl = DNS_RR_TTL(aptr);
    aptr += RRFIXEDSZ;
    if (aptr + rr_len > abuf + alen) {
      status = ARES_EBADRESP;
      break;
    }

    if (rr_class == C_IN && rr_type == T_A && rr_len == sizeof(struct in_addr) && strcasecmp(rr_name, hostname) == 0) {

      if (naddrs < max_in_addrs) {
        struct in_addr *at = &in_addrs_out[naddrs];
        if (aptr + sizeof(struct in_addr) > abuf + alen) {
          /* LCOV_EXCL_START: already checked above */
          status = ARES_EBADRESP;
          break;
        } /* LCOV_EXCL_STOP */
        memcpy(at, aptr, sizeof(struct in_addr));
      }
      naddrs++;
      status = ARES_SUCCESS;

    } else if (
        rr_class == C_IN && rr_type == T_AAAA && rr_len == sizeof(struct ares_in6_addr) && strcasecmp(rr_name, hostname) == 0) {

      if (n6addrs < max_in6_addrs) {
        struct in6_addr *const at = &in6_addrs_out[n6addrs];
        if (aptr + sizeof(struct in6_addr) > abuf + alen) {
          /* LCOV_EXCL_START: already checked above */
          status = ARES_EBADRESP;
          break;
        } /* LCOV_EXCL_STOP */
        memcpy(at, aptr, sizeof(struct in6_addr));
      }
      n6addrs++;
      status = ARES_SUCCESS;
    }

    if (rr_class == C_IN && rr_type == T_CNAME) {
      /* Record the RR name as an alias. */
      naliases++;

      /* Decode the RR data and replace the hostname with it. */
      status = dns_expand_name_maxlen(aptr, abuf, alen, rr_data, &len, NULL);
      if (status != ARES_SUCCESS) {
        break;
      }
      hostname = rr_data;

      /* Take the min of the TTLs we see in the CNAME chain. */
      if (cname_ttl > rr_ttl) {
        cname_ttl = rr_ttl;
      }
    }

    aptr += rr_len;
    if (aptr > abuf + alen) {
      /* LCOV_EXCL_START: already checked above */
      status = ARES_EBADRESP;
      break;
    } /* LCOV_EXCL_STOP */
  }

  /* here we are a bit aggressive and write a response even on failure so
   * caller can choose to use what we got so far */
  if (num_in_addrs) {
    *num_in_addrs = naddrs < max_in_addrs ? naddrs : max_in_addrs;
  }
  if (num_in6_addrs) {
    *num_in6_addrs = n6addrs < max_in6_addrs ? n6addrs : max_in6_addrs;
  }

  if (status == ARES_SUCCESS && naddrs == 0 && n6addrs == 0 && naliases == 0) {
    /* the check for naliases to be zero is to make sure CNAME responses
       don't get caught here */
    status = ARES_ENODATA;
  }

  return status;
}
