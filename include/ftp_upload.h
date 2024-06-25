
#pragma once 

#include <cstdint>
#include <string>

class FtpUpload
{

public:
  
public:
    FtpUpload(std::string url, std::string path, std::string objPath);
    std::string rmovePathRepeatSlash(const std::string &path); 
    ~FtpUpload();

private:

    bool FtpFileUpload(std::string ServerUrl,  std::string FilePath);

private:
    std::string _Url;
    std::string _FilePath;
    std::string _objPath;
};
