#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include "ftp_upload.h"


#include <errno.h>
#include <cerrno>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>  


extern char *optarg; 
extern int optind;   


typedef void  VOID ;
typedef  uint32_t UINT32;
typedef int  INT32;
typedef char  CHAR;
typedef unsigned char  UINT8;

typedef void * SYS_FILE_HANDLE_T;
#define SYS_FILE_HANDLE_INVALID NULL

#ifndef ERROR
#define ERROR               -1
#endif

#ifndef OK
#define OK                  0
#endif



typedef struct
{
    UINT32 uMagic;
    FILE *pFp;
    int iFd;
} SYS_FILE_HANDLE_PARAM;

typedef enum
{
    OPEN_MODE_READ   = 0x1,  /*只读方式打开*/
    OPEN_MODE_WRITE  = 0x2,  /*只写方式打开*/
    OPEN_MODE_RDWR   = 0x3,  /*读写方式打开，实际上是读和写的相或值*/
    OPEN_MODE_CREATE = 0x4,  /*创建文件，如果存在不会报错*/
    OPEN_MODE_CREATE_WITH_CHECK = 0x8,   /*创建的方式打开，如果存在则会报错*/
    OPEN_MODE_DIRECT = 0x10, /*开启direct io*/
}SYS_OPEN_MODE_T;

#define SYS_FILE_HANDLE_MAGIC 0x98765432

SYS_FILE_HANDLE_T sys_file_open(const CHAR * sFilePath, SYS_OPEN_MODE_T eMode)
{
    CHAR sOpt[16] = {0};
    SYS_FILE_HANDLE_PARAM * h = SYS_FILE_HANDLE_INVALID;

    FILE *pFile = NULL;
    int iFlag = 0;

    if(OPEN_MODE_CREATE & eMode)
    {
        if((OPEN_MODE_WRITE | OPEN_MODE_CREATE) == eMode)
        {
            snprintf(sOpt, sizeof(sOpt), "%s", "wb");
        }
        else
        {
            snprintf(sOpt, sizeof(sOpt), "%s", "rb+");
        }
    }
    else
    {
        if(OPEN_MODE_READ == eMode)
        {
            snprintf(sOpt, sizeof(sOpt), "%s", "rb");
        }
        else
        {
            snprintf(sOpt, sizeof(sOpt), "%s", "rb+");
        }
    }
    
    do
    {
        pFile = fopen(sFilePath, sOpt);
        if(NULL == pFile)
        {
            printf("open file %s failed\n", sFilePath);
            return SYS_FILE_HANDLE_INVALID;
        }

        h = (SYS_FILE_HANDLE_PARAM *)malloc(sizeof(SYS_FILE_HANDLE_PARAM));
        if(NULL == h)
        {
            break;
        }

        h->uMagic = SYS_FILE_HANDLE_MAGIC;
        h->pFp = pFile;
        return h;
    }while (0);

    fclose(pFile);

    return SYS_FILE_HANDLE_INVALID;
}

/**@brief 关闭文件
 * @param h 文件句柄
 * @return 成果返回OK，失败返回ERROR
 */
int sys_file_close(SYS_FILE_HANDLE_T h)
{
    int iRet = 0;

    SYS_FILE_HANDLE_PARAM *pHandle = (SYS_FILE_HANDLE_PARAM *)h;


    iRet = fclose(pHandle->pFp);

    pHandle->pFp = NULL;
    pHandle->uMagic = 0;

    free(h);

    if(iRet != 0)
    {
        return ERROR;
    }

    return OK;
}

/**@brief 读取文件
 * @param h 文件句柄
 * @param pBuf 数据缓冲区指针
 * @param iSize 缓冲区最大长度
 * @return 成果返回读取的字节数，失败返回ERROR
 */
UINT32 sys_file_read(SYS_FILE_HANDLE_T h, UINT8 * pBuf, int iSize)
{
    int iRet = 0;

    SYS_FILE_HANDLE_PARAM *pHandle = (SYS_FILE_HANDLE_PARAM *)h;

    iRet = fread(pBuf, 1, iSize, pHandle->pFp);
    if(iRet < 0)
    {
        printf("%p read %d bytes \n", pBuf, iSize);
        return ERROR;
    }

    return iRet;
}

/**@brief 写入文件
 * @param h 文件句柄
 * @param pBuf 数据缓冲区指针
 * @param iSize 缓冲区长度
 * @return 成果返回写入的字节数，失败返回ERROR
 */
UINT32 sys_file_write(SYS_FILE_HANDLE_T h, UINT8 * pBuf, int iSize)
{
    int iRet = 0;

    SYS_FILE_HANDLE_PARAM *pHandle = (SYS_FILE_HANDLE_PARAM *)h;


    iRet = fwrite(pBuf, 1, iSize, pHandle->pFp);

    if(iRet < 0)
    {
        printf("%p write %d bytes failed!\n", pBuf, iSize);
        return ERROR;
    }

    return iRet;
}



UINT32 sys_file_size(const char * sFilePath)
{
    INT32 iFd = -1;
    struct stat stBuf;

    if(NULL == sFilePath)
    {
        return ERROR;
    }

    memset(&stBuf, 0, sizeof(stBuf));

    iFd = open(sFilePath, O_RDONLY);
    if(iFd < 0)
    {
        printf("open %s failed!\n", sFilePath);
        return ERROR;
    }

    if (fstat(iFd, &stBuf) < 0)
    {
        printf("fstat %s failed!\n", sFilePath);
        close(iFd);
        return ERROR;
    }

    close(iFd);

    return stBuf.st_size;
}


typedef struct
{
	VOID *pData;
	UINT32 uSendSize;
	UINT32 uTotalSize;
}XB_PUT_DATA_CTRL_T;

XB_PUT_DATA_CTRL_T g_stXbCtrl = {0};

static size_t xb_put_upload_callback(VOID* pBuffer, size_t size, size_t nmemb, VOID *pStream)  
{
	INT32 uSendLen = 0;
	INT32 iRet = 0;
	if(g_stXbCtrl.pData == NULL)
	{
		printf("data == NULL!\n");
		return 0;
	}
	if(g_stXbCtrl.uSendSize >= g_stXbCtrl.uTotalSize)
	{
		printf("stPutCtrl.uSendSize = %d, stPutCtrl.uTotalSize = %d!\n", g_stXbCtrl.uSendSize, g_stXbCtrl.uTotalSize);
		return 0;
	}

	uSendLen = (g_stXbCtrl.uTotalSize - g_stXbCtrl.uSendSize) < 16 * 1024 ? (g_stXbCtrl.uTotalSize - g_stXbCtrl.uSendSize) : 16 * 1024;
		
	iRet = sys_file_read((SYS_FILE_HANDLE_T)g_stXbCtrl.pData, (UINT8 *)pBuffer, uSendLen);
	if(iRet != uSendLen)
	{
		printf("siRet = %d, uSendLen = %d, error!\n",iRet, uSendLen);
		return ERROR;
	}
	
	g_stXbCtrl.uSendSize += uSendLen;

	return uSendLen;
}


void update_progress_bar(CURL *handle, double dltotal, double dlnow, double ultotal, double ulnow) {
    // 计算上传进度百分比
    static curl_off_t last_ulnow = 0;  // 上一次回调的上传字节数
    static double last_call_time = 0.0; // 上一次回调的时间（秒）

    // 获取当前时间（秒）
    double current_time = time(NULL);

    // 如果是首次回调，不计算速度
    if (last_call_time == 0.0) {
        last_call_time = current_time;
    }

    // 计算自上次回调以来的秒数
    double seconds = current_time - last_call_time;
    if (seconds > 0) { // 防止除以零
        // 计算自上次回调以来上传的字节数
        curl_off_t bytes_uploaded = ulnow - last_ulnow;
        // 计算速度（字节/秒）
        double speed = bytes_uploaded / seconds;

        // 更新上次回调的信息
        last_ulnow = ulnow;
        last_call_time = current_time;

        // 格式化速度字符串
        char speed_buffer[50];
        snprintf(speed_buffer, sizeof(speed_buffer), "Speed: %6.2f Kbytes/sec", speed / 1024);

        // 计算上传进度百分比
        double upload_percentage = (double)ulnow / ultotal * 100.0;

        // 打印或更新进度条和速度
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
}

static int progress_callback(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    // 调用更新进度条的函数
    update_progress_bar(NULL, 0, 0, ultotal, ulnow);

    // 返回 0 表示继续传输
    return 0;
}

// 速度回调函数
static int xferinfo_callback(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    // 打印速度信息，例如每秒钟的字节数
   // printf("Speed: %llu bytes/sec\n", ulnow);
    //printf("Progress: %llu / %llu bytes\n", ulnow, ultotal);
    // 返回 0 表示继续传输
    return 0;
}
INT32 xb_upload_data(const CHAR *serverUrl, const CHAR *sPwd)
{
    CURL *pCurl;
    CURLcode res;
    int iRet = 0;
    SYS_FILE_HANDLE_T pHandle = NULL;
	INT32 iSize = 0;
	
    if(NULL == serverUrl)
    {
        printf("serverUrl err.\n");
        return ERROR;
    }
	
    if(NULL == sPwd)
    {
        printf("sPwd err.\n");
        return ERROR;
    }

	memset(&g_stXbCtrl, 0, sizeof(XB_PUT_DATA_CTRL_T));

	pHandle = sys_file_open(sPwd, OPEN_MODE_READ);
	if(NULL == pHandle)
	{
        printf("%s open error!\n\n", sPwd);
		return ERROR;
	}
	
	iSize = sys_file_size(sPwd);
	if(iSize <= 0)
	{
        printf("%s size error!\n\n", sPwd);
		sys_file_close(pHandle);
		return ERROR;
	}
	
	g_stXbCtrl.pData = pHandle;
	g_stXbCtrl.uSendSize = 0;
	g_stXbCtrl.uTotalSize = iSize;

    pCurl = curl_easy_init();
	if (pCurl != NULL)
	{
		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L); 												
		curl_easy_setopt(pCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);			
		curl_easy_setopt(pCurl, CURLOPT_URL, serverUrl);
		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 600);		
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 30);
		curl_easy_setopt(pCurl, CURLOPT_READDATA, g_stXbCtrl.pData);
        curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, xb_put_upload_callback); 

        curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L); // 确保进度回调启用
        curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        // curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, NULL); // 自定义数据，如果有的话	

		curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)iSize);	
		curl_easy_setopt(pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15);			

		/*DO*/
		res = curl_easy_perform(pCurl);
		if(res != CURLE_OK)
		{ 
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			iRet = ERROR;
		}
		else
		{
			printf("curl_easy_perform:http upload success,size == [%d]\n", iSize);
			iRet = OK;
		}
		printf("\n");
		curl_easy_cleanup(pCurl);
	}

	
	sys_file_close(pHandle);
    return iRet;
}




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
   // xb_upload_data("ftp:/lobmo:123456@10.2.14.227:21/123454.tar", "/home/wwk/workspeace/movex.tar");
    //xb_upload_data("ftp://oms:A21d32a297S5@47.114.190.241:9995/A3HL/HeiBiao/123454.tar", "/home/wwk/workspeace/1.tar");
   
    return 0;
}