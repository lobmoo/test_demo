#ifndef __UTILS_BASE64_H__
#define __UTILS_BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
* Base64 encoding
* @param dst Encoded data after processing
* @param dlen Size of encoded data after processing
* @param olen Actual encoded data size
* @param src Encoded data before processing
* @param slen Size of encoded data before processing
* @return 0: Success  -1: Failure
*/
int utils_base64encode(unsigned char *dst, unsigned int dlen, unsigned int *olen, const unsigned char *src, unsigned int slen);


/**
* Base64 decoding
* @param dst Decoded data after processing
* @param dlen Size of decoded data after processing
* @param olen Actual decoded data size
* @param src Encoded data before processing
* @param slen Size of encoded data before processing
* @return 0: Success  -1: Failure
*/
int utils_base64decode(unsigned char *dst, unsigned int dlen, unsigned int *olen, const unsigned char *src, unsigned int slen);

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_BASE64_H__ */
