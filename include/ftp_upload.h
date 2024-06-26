
#pragma once 

#include <cstdint>
#include <string>
#include <curl/curl.h>

class FtpUpload
{

public:
    typedef  int (*progressFunc)(void *p,curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow); 
public:
    FtpUpload(std::string url, std::string path, std::string objPath);
    std::string rmovePathRepeatSlash(const std::string &path); 
    ~FtpUpload();

private:
    static size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *userdata); 
    bool FtpFileUpload(std::string ServerUrl,  std::string FilePath, progressFunc func = progress_callback);
    static int progress_callback(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

private:
    std::string _Url;
    std::string _FilePath;
    std::string _objPath;
};
