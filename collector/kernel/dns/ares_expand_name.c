
/* Copyright 1998, 2011 by the Massachusetts Institute of Technology.
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

#include <arpa/nameser.h>
#include <netinet/in.h>

#include "ares.h"

#include <collector/kernel/dns/dns.h>

/* Maximum number of indirections allowed for a name */
#define MAX_INDIRS 50

/** from ares_nowarn.c **/
/*
** unsigned size_t to signed long
*/
#define CARES_MASK_SLONG 0x7FFFFFFFL
long aresx_uztosl(size_t uznum)
{
#ifdef __INTEL_COMPILER
#pragma warning(push)
#pragma warning(disable : 810) /* conversion may lose significant bits */
#endif

  return (long)(uznum & (size_t)CARES_MASK_SLONG);

#ifdef __INTEL_COMPILER
#pragma warning(pop)
#endif
}

/* Expand an RFC1035-encoded domain name given by encoded.  The
 * containing message is given by abuf and alen.  The result is written,
 * NUL-terminated, into s, whose size must be large enough to hold the
 * domain name, i.e., no smaller than dns_name_length()+1.
 * *enclen is set to the length of the encoded name (not the length of the
 * expanded name; the goal is to tell the caller how many bytes to
 * move forward to get past the encoded name).
 *
 * In the simple case, an encoded name is a series of labels, each
 * composed of a one-byte length (limited to values between 0 and 63
 * inclusive) followed by the label contents.  The name is terminated
 * by a zero-length label.
 *
 * In the more complicated case, a label may be terminated by an
 * indirection pointer, specified by two bytes with the high bits of
 * the first byte (corresponding to INDIR_MASK) set to 11.  With the
 * two high bits of the first byte stripped off, the indirection
 * pointer gives an offset from the beginning of the containing
 * message with more labels to decode.  Indirection can happen an
 * arbitrary number of times, so we have to detect loops.
 *
 * Since the expanded name uses '.' as a label separator, we use
 * backslashes to escape periods or backslashes in the expanded name.
 */
void dns_expand_name(const unsigned char *encoded, const unsigned char *abuf, int alen, char *expanded_name, long *enclen)
{
  int len, indir = 0;
  const unsigned char *p;
  char *q = expanded_name;

  /* No error-checking necessary; it was all done by name_length(). */
  p = encoded;
  while (*p) {
    if ((*p & INDIR_MASK) == INDIR_MASK) {
      if (!indir) {
        *enclen = aresx_uztosl(p + 2U - encoded);
        indir = 1;
      }
      p = abuf + ((*p & ~INDIR_MASK) << 8 | *(p + 1));
    } else {
      len = *p;
      p++;
      while (len--) {
        if (*p == '.' || *p == '\\')
          *q++ = '\\';
        *q++ = *p;
        p++;
      }
      *q++ = '.';
    }
  }
  if (!indir)
    *enclen = aresx_uztosl(p + 1U - encoded);

  /* Nuke the trailing period if we wrote one. */
  if (q > expanded_name)
    *(q - 1) = 0;
  else
    *q = 0; /* zero terminate the zero-length domain */
}

/* Return the length of the expansion of an encoded domain name, or
 * -1 if the encoding is invalid.
 */
int dns_name_length(const unsigned char *encoded, const unsigned char *abuf, int alen)
{
  int n = 0, offset, indir = 0, top;

  /* Allow the caller to pass us abuf + alen and have us check for it. */
  if (encoded >= abuf + alen)
    return -1;

  while (*encoded) {
    top = (*encoded & INDIR_MASK);
    if (top == INDIR_MASK) {
      /* Check the offset and go there. */
      if (encoded + 1 >= abuf + alen)
        return -1;
      offset = (*encoded & ~INDIR_MASK) << 8 | *(encoded + 1);
      if (offset >= alen)
        return -1;
      encoded = abuf + offset;

      /* If we've seen more indirects than the message length,
       * then there's a loop.
       */
      ++indir;
      if (indir > alen || indir > MAX_INDIRS)
        return -1;
    } else if (top == 0x00) {
      offset = *encoded;
      if (encoded + offset + 1 >= abuf + alen)
        return -1;
      encoded++;
      while (offset--) {
        n += (*encoded == '.' || *encoded == '\\') ? 2 : 1;
        encoded++;
      }
      n++;
    } else {
      /* RFC 1035 4.1.4 says other options (01, 10) for top 2
       * bits are reserved.
       */
      return -1;
    }
  }

  /* If there were any labels at all, then the number of dots is one
   * less than the number of labels, so subtract one.
   */
  return (n) ? n - 1 : n;
}

/**
 * This version of expand name assumes s is of length DNS_NAME_MAX_LENGTH and
 * fails if the expanded name exceeds that. If @expanded_len is not NULL, the
 *   expanded length is written there.
 */
int dns_expand_name_maxlen(
    const unsigned char *encoded, const unsigned char *abuf, int alen, char *expanded_name, long *enclen, int *expanded_len)
{
  int len = dns_name_length(encoded, abuf, alen);
  if (len < 0)
    return ARES_EBADNAME;

  if (len + 1 > DNS_NAME_MAX_LENGTH)
    return ARES_ENOMEM;

  dns_expand_name(encoded, abuf, alen, expanded_name, enclen);

  if (expanded_len)
    *expanded_len = len;

  return ARES_SUCCESS;
}
