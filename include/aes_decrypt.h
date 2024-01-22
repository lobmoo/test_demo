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

/**
 * @file
 */

#pragma once

#include <string>



/**
 * @brief must 16 byte
 */

#define AES_KEY "AlbzB9DrZLSq3X7y"

/**
 * @namespace movex::common::util
 * @brief movex::common::util
 */

/**
 * @brief  AesEncrypt
 * @param  plain_text       need encrypt data
 * @param  key              key
 * @param  result           cipher data
 * @return true  / false
 */
bool AesEncrypt(const std::string& plain_text, const std::string& key,
                std::string* const result);

/**
 * @brief  AesDecrypt
 * @param  plain_text       cipher data
 * @param  key              key
 * @param  result           source data
 * @return true  / false
 */
bool AesDecrypt(const std::string& cipher_text, const std::string& key,
                std::string* const result);

