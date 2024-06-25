#include "ftp_upload.h"

#include <stdio.h>
#include <fstream>
#include "curl/curl.h"


#define FTPUPLOAD_INFO
#ifdef FTPUPLOAD_INFO
#define INFO(fmt, args...) printf("I[%s:%4d] " fmt, __FILE__, __LINE__, ##args)
#else
#define INFO(fmt, args...)
#endif
    
#define FTPUPLOAD_WARN
#ifdef FTPUPLOAD_WARN
#define WARN(fmt, args...) printf("W[%s:%4d] " fmt, __FILE__, __LINE__, ##args)
#else
#define WARN(fmt, args...)
#endif
    
#define FTPUPLOAD_ERR
#ifdef FTPUPLOAD_ERR
#define ERR(fmt, args...) printf("\033[31mE[%s:%4d]\033[0m " fmt, __FILE__, __LINE__, ##args)
#else
#define ERR(fmt, args...)
#endif


FtpUpload::FtpUpload()
{
 
}

FtpUpload::~FtpUpload()
{

}


bool FtpUpload::FtpFileUpload(std::string ServerUrl,  std::string FilePath)
{
    bool iRet = true;
    if(FilePath.empty() || ServerUrl.empty())
    {
        FTPUPLOAD_ERR("FilePath  or  ServerUrl is null \n");
        return false;
    }
    std::ifstream inputFile(FilePath, std::ios::binary);
    if (!inputFile.is_open())
    {
        FTPUPLOAD_ERR("FilePath: %s  open  failed!\n", FilePath);
        return false;
    }

    inputFile.seekg(0, std::ios::end);
    std::streamsize size = inputFile.tellg(); 
    if(size <= 0)
    {
        FTPUPLOAD_ERR("File size is zero or negative.");
    }
    FTPUPLOAD_ERR("File size:  %d bytes\n", size);

    CURL *pCurl = curl_easy_init();
    if(pCurl)
    {
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L); 												
		curl_easy_setopt(pCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);			
		curl_easy_setopt(pCurl, CURLOPT_URL, ServerUrl);
		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 600);			
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 30);
		curl_easy_setopt(pCurl, CURLOPT_READDATA, &inputFile);
        curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, [](void *ptr, size_t size, size_t nmemb, void *userdata) {
            auto *stream = static_cast<std::ifstream *>(userdata);
            return stream->readsome(static_cast<char *>(ptr), size * nmemb);});
		curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(size));	
		curl_easy_setopt(pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15);

        CURLcode res = curl_easy_perform(pCurl);
        if(res != CURLE_OK)
        { 
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            iRet = false;
        }

        curl_easy_cleanup(pCurl);
    }
    inputFile.close();
    return iRet;
}