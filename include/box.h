/**
 * @file box.h
 * @brief 
 * @author wwk (1162431386@qq.com)
 * @version 1.0
 * @date 2023-09-22
 * 
 * @copyright Copyright (c) 2023  by  wwk
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2023-09-22     <td>1.0     <td>wwk   <td>修改?
 * </table>
 */
#pragma once 

#include <cstdint>

class box
{
      
public:
    box() = delete;
    box(int32_t length, int32_t width);
    ~box();

private:
    uint32_t length;
    uint32_t width;  

};


// CBC模式加密
int aes_cbc_encrypt(char* in, char* key, char* out);
// CBC模式解密
int aes_cbc_decrypt(char* in, char* key, char* out);
// 将加密与解密整合在一起
void AES(unsigned char* InBuff, unsigned char* OutBuff, unsigned char* key, char* Type);