
#pragma once 

#include <cstdint>
#include <string>

class FtpUpload
{

public:
  
public:
    FtpUpload();
   
    ~FtpUpload();

private:

    bool FtpFileUpload(std::string ServerUrl,  std::string FilePath);

private:
    std::string ServerUrl;
    std::string FilePath;
};
