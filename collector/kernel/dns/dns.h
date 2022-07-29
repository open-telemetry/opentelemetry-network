/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/flowmill/ingest.wire_message.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DNS_NAME_MAX_LENGTH 256

#define MAX_ENCODED_DOMAIN_NAME 80
#define MAX_ENCODED_IP_ADDRS 16

#define MAX_ENCODED_DNS_MESSAGE                                                                                                \
  (/* timestamp */ sizeof(u64) + /* jb message */ jb_ingest__dns_response__data_size +                                         \
   /* ip addrs */ (sizeof(u32) * MAX_ENCODED_IP_ADDRS) + /* DNS name */ MAX_ENCODED_DOMAIN_NAME)

int dns_name_length(const unsigned char *encoded, const unsigned char *abuf, int alen);

void dns_expand_name(const unsigned char *encoded, const unsigned char *abuf, int alen, char *s, long *enclen);

int dns_expand_name_maxlen(
    const unsigned char *encoded, const unsigned char *abuf, int alen, char *s, long *enclen, int *expanded_len);

int dns_parse_query(
    const unsigned char *abuf,
    int alen,
    int *is_response,
    uint16_t *type_out,
    uint16_t *qid_out,
    char *question_out,
    int *question_len);

int dns_parse_a_aaaa_reply(
    const unsigned char *abuf,
    int alen,
    char *hostname_out,
    int *hostname_len,
    struct in_addr *in_addrs_out,
    int *num_in_addrs,
    struct in6_addr *in6_addrs_out,
    int *num_in6_addrs);

#ifdef __cplusplus
}
#endif /* __cplusplus */
