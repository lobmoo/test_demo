#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>

#include "ftp_upload.h"
 

extern char *optarg; 
extern int optind;   

int main(int argc, char *argv[]) 
{
    optind = 1;
    std::string url;
    std::string path; 
    int opt = getopt(argc, argv, "u:p:o:h");
    while (opt != -1) {
        switch (opt) {
            case 'u':
                url = optarg; 
                std::cout << "url:" << optarg << std::endl;
                break;
            case 'p':
                std::cout << "选项 -a 带的参数是: " << optarg << std::endl;
                break;
            case 'o': 
                std::cerr << "未知选项" << std::endl;
                break;
            default:
                std::cerr << "未知选项" << std::endl;
                return -1;
        }
        opt = getopt(argc, argv, "ha:");
    }
    return 0;
}