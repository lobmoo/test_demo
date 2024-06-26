#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>

#include "ftp_upload.h"
 

extern char *optarg; 
extern int optind;   




void FileupHelp(void)
{
std::cout   << "Options:\n"
            << "  -u, --url        Specify the URL   demo: user:passwd@ip:port \n"
            << "  -p, --path       Specify the file path (absolute path)\n"
            << "  -o, --objective  Specify the objective path(absolute path)\n"
            << "  -h, --help       Display this help message and exit\n";
}


int main(int argc, char *argv[]) 
{
    std::string url;
    std::string path; 
    std::string optargPath;
    int opt = -1;
    optind = 1;
    while ( -1 != (opt = getopt(argc, argv, "u:p:o:h"))) {
        switch (opt) {
            case 'u':
                url = optarg; 
                std::cout << "++++++++++++url:  " << url << std::endl;
                break;
            case 'p':
                path = optarg;
                std::cout << "++++++file path:  " << path << std::endl;
                break;
            case 'o': 
                optargPath = optarg;
                std::cout << "+objective path:  " << optargPath << std::endl;
                break;
            case 'h':
                FileupHelp();
                break;
            default:
                FileupHelp();
                break;
        }
    }
    if(url.empty() || path.empty() || optargPath.empty())
    {
        FileupHelp();
    }
    FtpUpload filup(url, path, optargPath);
   
    return 0;
}