#include "utils_base64.h"

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static const unsigned char base64_enc_map[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static const unsigned char base64_dec_map[128] = {
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 62,  127,
    127, 127, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  127, 127,
    127, 64,  127, 127, 127, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
    10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25,  127, 127, 127, 127, 127, 127, 26,  27,  28,  29,  30,  31,  32,  33,
    34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  127, 127, 127, 127, 127};

#define BASE64_SIZE_T_MAX ((unsigned int)-1) /* SIZE_T_MAX is not standard */

int utils_base64encode(unsigned char *dst, unsigned int dlen,
                       unsigned int *olen, const unsigned char *src,
                       unsigned int slen) {
  unsigned int i, n;
  unsigned char *p;

  if (slen == 0) {
    *olen = 0;
    return (0);
  }

  n = slen / 3 + (slen % 3 != 0);

  if (n > (BASE64_SIZE_T_MAX - 1) / 4) {
    *olen = BASE64_SIZE_T_MAX;
    return (-1);
  }

  n *= 4;

  if ((dlen < n + 1) || (NULL == dst)) {
    *olen = n + 1;
    return (-1);
  }

  n = (slen / 3) * 3;

  int C1, C2, C3;
  for (i = 0, p = dst; i < n; i += 3) {
    C1 = *src++;
    C2 = *src++;
    C3 = *src++;

    *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
    *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];
    *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
    *p++ = base64_enc_map[C3 & 0x3F];
  }

  if (i < slen) {
    C1 = *src++;
    C2 = ((i + 1) < slen) ? *src++ : 0;

    *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
    *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

    if ((i + 1) < slen) {
      *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
    } else {
      *p++ = '=';
    }

    *p++ = '=';
  }

  *olen = p - dst;
  *p = 0;

  return (0);
}

int utils_base64decode(unsigned char *dst, unsigned int dlen,
                       unsigned int *olen, const unsigned char *src,
                       unsigned int slen) {
  unsigned int i, n;
  unsigned int j, x;
  unsigned char *p;

  /* First pass: check for validity and get output length */
  for (i = n = j = 0; i < slen; ++i) {
    /* Skip spaces before checking for EOL */
    x = 0;
    while (i < slen && src[i] == ' ') {
      ++i;
      ++x;
    }

    /* Spaces at end of buffer are OK */
    if (i == slen) {
      break;
    }

    if ((slen - i) >= 2 && src[i] == '\r' && src[i + 1] == '\n') {
      continue;
    }

    if (src[i] == '\n') {
      continue;
    }

    /* Space inside a line is an error */
    if (x != 0) {
      return (-1);
    }

    if (src[i] == '=' && ++j > 2) {
      return (-1);
    }

    if (src[i] > 127 || base64_dec_map[src[i]] == 127) {
      return (-1);
    }

    if (base64_dec_map[src[i]] < 64 && j != 0) {
      return (-1);
    }

    n++;
  }

  if (n == 0) {
    *olen = 0;
    return (0);
  }

  n = ((n * 6) + 7) >> 3;
  n -= j;

  if (dst == NULL || dlen < n) {
    *olen = n;
    return (-1);
  }

  for (j = 3, n = x = 0, p = dst; i > 0; --i, ++src) {
    if (*src == '\r' || *src == '\n' || *src == ' ') {
      continue;
    }

    j -= (base64_dec_map[*src] == 64);
    x = (x << 6) | (base64_dec_map[*src] & 0x3F);

    if (++n == 4) {
      n = 0;
      if (j > 0) {
        *p++ = (unsigned char)(x >> 16);
      }
      if (j > 1) {
        *p++ = (unsigned char)(x >> 8);
      }
      if (j > 2) {
        *p++ = (unsigned char)(x);
      }
    }
  }

  *olen = p - dst;

  return (0);
}

#ifdef __cplusplus
}
#endif
