
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
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "ares.h"
#include "ares_dns.h"

#include <collector/kernel/dns/c_ares_nameser.h>
#include <collector/kernel/dns/dns.h>

// #define DEBUG_DNS_PARSE_QUERY 1

/**
 * returns ARES_SUCCESS if this is a valid request or response,
 * ARES_EBADQUERY otherwise, or on error
 *
 * Whether or not this is a request or reply will be returned in *is_response
 * The question's type will be returned in *type_out
 * The transaction ID will returned in *qid_out
 *
 * The question (hostname) of the request will be written to @question_out, and
 * its length to @question_len. @hostname_out must be at least
 * DNS_NAME_MAX_LENGTH characters long.
 */

int dns_parse_query(
    const unsigned char *abuf,
    int alen,
    int *is_response,
    uint16_t *type_out,
    uint16_t *qid_out,
    char *question_out,
    int *question_len)
{
  unsigned int qdcount, ancount, nscount; //, arcount;
  int qr, status, q_type, q_class;
  long len;
  uint16_t qid;
  const unsigned char *aptr;

  /* Give up if abuf doesn't have room for a header. */
  if (alen < HFIXEDSZ) {
#if DEBUG_DNS_PARSE_QUERY
    fprintf(stderr, "EBADQUERY: alen < HFIXEDSZ\n");
#endif
    return ARES_EBADQUERY;
  }

  /* Fetch the question and answer count from the header. */
  qr = DNS_HEADER_QR(abuf);
  qid = DNS_HEADER_QID(abuf);
  qdcount = DNS_HEADER_QDCOUNT(abuf);
  ancount = DNS_HEADER_ANCOUNT(abuf);
  nscount = DNS_HEADER_NSCOUNT(abuf);
  // arcount = DNS_HEADER_ARCOUNT(abuf);

#if DEBUG_DNS_PARSE_QUERY
  fprintf(
      stderr, "qr: %d qid: %d qdcount:%u ancount:%u nscount:%u arcount:%u\n", qr, (int)qid, qdcount, ancount, nscount, arcount);
#endif

  /* Determine if this is a request */
  if (qr == 0) {
    /* if it's a request, only accept requests with a single question and
     * nothing else */
    if (qdcount != 1 || ancount != 0 || nscount != 0) {
#if DEBUG_DNS_PARSE_QUERY
      fprintf(stderr, "EBADQUERY: request with more than one question or other records\n");
#endif
      return ARES_EBADQUERY;
    }
    if (is_response)
      *is_response = 0;
  } else {
    /* if it's a response, only accept responses to a single question */
    if (qdcount != 1) {
#if DEBUG_DNS_PARSE_QUERY
      fprintf(stderr, "EBADQUERY: response to not single question\n");
#endif
      return ARES_EBADQUERY;
    }
    if (is_response)
      *is_response = 1;
  }

  /* Return the transaction id */
  if (qid_out) {
    *qid_out = qid;
  }

  /* Expand the name from the question */
  aptr = abuf + HFIXEDSZ;
  status = dns_expand_name_maxlen(aptr, abuf, alen, question_out, &len, question_len);
  if (status != ARES_SUCCESS) {
#if DEBUG_DNS_PARSE_QUERY
    fprintf(stderr, "EBADQUERY: couldn't expand name\n");
#endif
    return status;
  }
  if (aptr + len + QFIXEDSZ > abuf + alen) {
#if DEBUG_DNS_PARSE_QUERY
    fprintf(stderr, "EBADQUERY: truncated packet\n");
#endif
    return ARES_EBADQUERY;
  }
  aptr += len;

  q_type = DNS_QUESTION_TYPE(aptr);
  q_class = DNS_QUESTION_CLASS(aptr);

  /* Reject anything that isn't an Internet query */
  if (q_class != ns_c_in) {
#if DEBUG_DNS_PARSE_QUERY
    fprintf(stderr, "EBADQUERY: non-internet query\n");
#endif
    return ARES_EBADQUERY;
  }

  if (type_out) {
    *type_out = q_type;
  }

  return ARES_SUCCESS;
}
