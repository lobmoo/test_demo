/******************************************************************************
 * Copyright 2023 The Move-X Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include "aes_decrypt.h"
#include <cstring>
#include <iostream>
#include <vector>
#include "utils_aes.h"
#include "utils_base64.h"

#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 16
#endif

bool AesEncrypt(const std::string& plain_text, const std::string& key,
                std::string* const result) {
  std::string cipher_text;
  int key_length = key.length();
  int plain_text_length = plain_text.length();
  utils_aes_context aescontext = {0};

  utils_aes_init(&aescontext);
  if (0 != utils_aes_setkey_enc(&aescontext, (const unsigned char*)key.c_str(),
                                8 * key_length)) {
    return false;
  }
  int block_size = AES_BLOCK_SIZE;
  int num_blocks = (plain_text_length + block_size) / block_size;
  char* padded_plain_text = new char[num_blocks * block_size];
  memset(padded_plain_text, 0,
           num_blocks * block_size);
  memcpy(padded_plain_text,  plain_text.c_str(),
           plain_text_length);

  int padding_length = block_size - (plain_text_length % block_size);
  memset(padded_plain_text + plain_text_length,
          padding_length,
           padding_length);

  for (int i = 0; i < num_blocks; ++i) {
    unsigned char input_block[AES_BLOCK_SIZE];
    memset(input_block, 0, block_size);
    memcpy(input_block, padded_plain_text + (i * block_size),
             block_size);
    unsigned char output_block[AES_BLOCK_SIZE];
    if (0 != utils_aes_crypt_ecb(&aescontext, UTILS_AES_ENCRYPT, input_block,
                                 output_block)) {
      return false;
    }
    cipher_text.append(reinterpret_cast<const char*>(output_block), block_size);
  }
  uint32_t out_base64_size = 2 * cipher_text.length();
  std::string out_base64(out_base64_size, '\0');
  unsigned int olen = 0;
  if (utils_base64encode((unsigned char*)out_base64.data(), out_base64_size,
                         &olen, (unsigned char*)cipher_text.data(),
                         cipher_text.length())) {
    return false;
  }
  out_base64.resize(olen);
  delete[] padded_plain_text;
  *result = out_base64;
  return true;
}

bool AesDecrypt(const std::string& cipher_text, const std::string& key,
                std::string* const result) {
  int key_length = key.length();
  int cipher_text_length = 0;
  int temp_buf_len = cipher_text.length() + 1;
  std::vector<unsigned char> temp_buf(temp_buf_len);
  utils_aes_context aescontext = {0};

  utils_aes_init(&aescontext);
  utils_aes_setkey_dec(&aescontext, (const unsigned char*)key.c_str(),
                       8 * key_length);
  unsigned int olen = 0;
  if (0 != utils_base64decode(temp_buf.data(), temp_buf_len, &olen,
                              (unsigned char*)cipher_text.data(),
                              cipher_text.length())) {
    return false;
  }
  cipher_text_length = olen;
  int block_size = AES_BLOCK_SIZE;
  int num_blocks = cipher_text_length / block_size;
  char* padded_plain_text = new char[cipher_text_length];
  memset(padded_plain_text, 0, cipher_text_length);
  for (int i = 0; i < num_blocks; ++i) {
    unsigned char input_block[AES_BLOCK_SIZE];
    memset(input_block, 0, block_size);
    memcpy(input_block, temp_buf.data() + (i * block_size),
             block_size);
    unsigned char output_block[AES_BLOCK_SIZE];
    if (0 != utils_aes_crypt_ecb(&aescontext, UTILS_AES_DECRYPT, input_block,
                                 output_block)) {
      return false;
    }
    memcpy(padded_plain_text + (i * block_size),
             output_block, block_size);
  }

  int padding_length = padded_plain_text[cipher_text_length - 1];
  result->assign(padded_plain_text, cipher_text_length - padding_length);
  delete[] padded_plain_text;
  return true;
}


