#include "ftp_upload.h"

#include <stdio.h>
#include <fstream>
#include "curl/curl.h"
#include <iostream>

#define  DEBUG


#ifdef  DEBUG
#define FTPUPLOAD_INFO(fmt, args...) do{printf("I[%s:%4d] " fmt, __FILE__, __LINE__, ##args);}while(0);
#else
#define FTPUPLOAD_INFO(...) printf(__VA_ARGS__) 
#endif
    
#ifdef  DEBUG
#define FTPUPLOAD_ERR(fmt, args...) do{ printf("\033[31mE[%s:%4d]\033[0m " fmt, __FILE__, __LINE__, ##args);}while(0);
#else
#define FTPUPLOAD_ERR(...) printf(__VA_ARGS__) 
#endif
  
#ifdef  DEBUG
#define FTPUPLOAD_WARN(fmt, args...) do{printf("I[%s:%4d] " fmt, __FILE__, __LINE__, ##args);}while(0);
#else
#define FTPUPLOAD_WARN(...) printf(__VA_ARGS__) 
#endif
  



FtpUpload::FtpUpload(std::string url, std::string path, std::string objPath) : _Url(url), _FilePath(path), _objPath(objPath)
{
    std::string serverUrl;
   
    serverUrl = rmovePathRepeatSlash(url + '/'  + objPath);
    serverUrl  = "ftp://" + serverUrl;
    FTPUPLOAD_INFO("upload url : %s,  objPath : %s \n", serverUrl.c_str(), objPath.c_str());
    if(!FtpFileUpload(serverUrl, path))
    {
       FTPUPLOAD_ERR("FtpFileUpload failed!\n"); 
    }
}


FtpUpload::~FtpUpload()
{

}



std::string FtpUpload ::rmovePathRepeatSlash(const std::string &path) {
    std::string result;
    size_t length = path.length();

    if (length == 0) {
        return result;
    }

    result += path[0];

    for (size_t i = 1; i < length; i++) {
        if (path[i] == '/' && path[i - 1] == '/') {
            continue;
        }
        result += path[i];
    }

    return result;
}



size_t FtpUpload::ReadCallback(void *ptr, size_t size, size_t nmemb, void *userdata) {
        std::ifstream* stream = static_cast<std::ifstream*>(userdata);
        return stream->readsome(static_cast<char*>(ptr), size * nmemb);
}

int FtpUpload::progress_callback(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) 
{
    
    static curl_off_t last_ulnow = 0;  
    static double last_call_time = 0.0; 

    
    double current_time = time(NULL);

    
    if (last_call_time == 0.0) {
        last_call_time = current_time;
    }

   
    double seconds = current_time - last_call_time;
    if (seconds > 0) { 
        curl_off_t bytes_uploaded = ulnow - last_ulnow;
        
        double speed = bytes_uploaded / seconds;

        last_ulnow = ulnow;
        last_call_time = current_time;

   
        char speed_buffer[50];
        snprintf(speed_buffer, sizeof(speed_buffer), "Speed: %6.2f MB/sec", speed / (1024*1024));

        
        double upload_percentage = (double)ulnow / ultotal * 100.0;

        
        printf("\rUploading: [");
        for (int i = 0; i < 50; ++i) {
            if (i < (upload_percentage * 50 / 100)) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("] %3.0f%% %s", upload_percentage, speed_buffer);
        fflush(stdout);
    }
        return 0;
}


bool FtpUpload::FtpFileUpload(std::string ServerUrl,  std::string FilePath, progressFunc func)
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
        FTPUPLOAD_ERR("FilePath: %s  open  failed!\n", FilePath.c_str());
        return false;
    }

    inputFile.seekg(0, std::ios::end);
    std::streamsize size = inputFile.tellg(); 
    if(size <= 0)
    {
        FTPUPLOAD_ERR("File size is zero or negative.");
        return false;
    }
    FTPUPLOAD_INFO("File size:  %ld bytes\n", size);
    inputFile.seekg(0, std::ios::beg);

    CURL *pCurl = curl_easy_init();
    if(pCurl)
    {
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L); 												
		curl_easy_setopt(pCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);			
		curl_easy_setopt(pCurl, CURLOPT_URL, ServerUrl.c_str());
		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 600);			
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 30);
		curl_easy_setopt(pCurl, CURLOPT_READDATA,&inputFile);

        curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L); // 进度回调启用
        curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, func);
        curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, ReadCallback);
		curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(size));	
		curl_easy_setopt(pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15L);

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