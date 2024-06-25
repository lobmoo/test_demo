#include "socket.h"

struct svr_t {
	svr_process_thread func;
    int uWaitMsec;
	int sver_sock_fd;
    unsigned short listen_port;
};


static int pthreadGetPriorityScope(int *minPriority, int *maxPriority)
{
	/* get the allowable priority range for the scheduling policy */
	if (minPriority != NULL)
	{
		(*minPriority) = sched_get_priority_min(SCHED_RR);
		if (*minPriority == -1)
		{
			return -1;
		}
	}
	if (maxPriority != NULL)
	{
		(*maxPriority) = sched_get_priority_max(SCHED_RR);
		if (*maxPriority == -1)
		{
			return -1;
		}
	}
	//SOCKET_PRINTF("priority: min = %d, max = %d\n", *minPriority, *maxPriority);

	return 0;
}

int setPthreadAttr(pthread_attr_t *attr, int priority, size_t stacksize, int bRealTime)
{
	int rval;
	struct sched_param	params;
	int maxPriority, minPriority;

	rval = pthread_attr_init(attr);
	if (rval != 0)
	{
		return rval;
	}

	/* normally, need not to set */
	rval = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	rval = pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);
	if (rval != 0)
	{
		if (rval == ENOTSUP)
		{
			SOCKET_PRINTF("The system does not support the %s scope, using %s\n",
					"PTHREAD_SCOPE_SYSTEM", "PTHREAD_SCOPE_PROCESS");

			rval = pthread_attr_setscope(attr, PTHREAD_SCOPE_PROCESS);
		}

		if (rval)
		{
			pthread_attr_destroy(attr);
			return rval;
		}
	}

	/* use the round robin scheduling algorithm */
    if (0 != bRealTime)
    {
    	rval = pthread_attr_setschedpolicy(attr, SCHED_RR);
    }
    else
    {
    	rval = pthread_attr_setschedpolicy(attr, SCHED_OTHER);
    }
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	/* set the thread to be detached */
	rval = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	/* first get the scheduling parameter, then set the new priority */
	rval = pthread_attr_getschedparam(attr, &params);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	rval = pthreadGetPriorityScope(&minPriority, &maxPriority);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}
	if (priority < minPriority)
	{
		priority = minPriority;
	}
	else if (priority > maxPriority)
	{
		priority = maxPriority;
	}
	params.sched_priority = priority;
    if (0 == bRealTime)
    {
    	params.sched_priority = 0;
    }
	rval = pthread_attr_setschedparam(attr, &params);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}
#ifdef USE_NPTL
    /* nptl创建线程默认继承父线程的调度优先级，需要设置inheritsched为
       PTHREAD_EXPLICIT_SCHED，自己设置的调度策略和优先级才能生效 */
    rval = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
    if (rval != 0)
    {
        pthread_attr_destroy(attr);
        return rval;
    }
#endif

	/* when set stack size, we define a minmum value to avoid fail */
	if (stacksize < (16 * 1024))
	{
		stacksize = (16 * 1024);
	}
	
#ifdef USE_NPTL
    /*  hisi3531平台使用nptl线程库，部分线程会因为设置的堆栈大小不够大导致程序
        内存不足崩掉，暂时将所有线程堆栈大小设置成1M */
    stacksize = 1 * 1024 * 1024;
#endif
	rval = pthread_attr_setstacksize(attr, stacksize);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	return 0;
}
/*
* @Function: socket_create
* @Description:crate socket
* @Input:used udp?
* @Output:1:succe -1 fail
* @Return: 1:succe -1 fail
*/
int socket_create(int isUdp)
{
	int iSockFd = -1;

    iSockFd = socket(AF_INET, isUdp ? SOCK_DGRAM : SOCK_STREAM, false);
	if(iSockFd < 0)
    {
    	SOCKET_PRINTF("socket_create failed: errno %d\n", errno);
        return -1;
    }

	return iSockFd;
}


/*
* @Function: socket_close
* @Description:close socket
* @Input:fd
* @Output:NULL
* @Return: NULL
*/
void socket_close(int iSockFd)
{
    close(iSockFd);
}

/*
* @Function: socket_connect_wait
* @Description:connect 
* @Input:sockfd: 描述fd: address ipv4  msecond:connect超时时间(ms) -1为阻塞式connect
* @Output:NULL
* @Return: NULL
*/
static int socket_connect_wait(int sockfd, struct sockaddr_in *address, int msecond)
{
    int err = 0; 
    int len = sizeof(int);
    int block_or_not = 0; // 将socket设置成阻塞或非阻塞
    int ret_val = -1;     // 接收函数返回
    fd_set set;
    struct timeval mytm;

    if ((NULL == address) || (sockfd < 0))
    {
        SOCKET_PRINTF("connect_with_timeout para error\n");
        return -1;
    }

    memset(&mytm, 0, sizeof(struct timeval));

    if (-1 == msecond)  /*阻塞connet*/
    {
        ret_val = connect(sockfd, (struct sockaddr *)address, sizeof(struct sockaddr_in));
        SOCKET_PRINTF(" %d\n", ret_val);
        return ret_val;
    }

    block_or_not = 1; // 设置非阻塞
    if (0 != ioctl(sockfd, FIONBIO, &block_or_not))
    {
        SOCKET_PRINTF("ioctl socket failed\n");
    }
    
    ret_val = connect(sockfd, (struct sockaddr *)address, (socklen_t)sizeof(struct sockaddr_in));

    if (-1 == ret_val)
    {
        if (EINPROGRESS == errno)
        {
            FD_ZERO(&set);
            FD_SET(sockfd, &set);
            mytm.tv_sec = msecond / 1000;
            mytm.tv_usec = (msecond % 1000) * 1000;

            if (select(sockfd + 1, NULL, &set, NULL, &mytm) > 0)
            {
                // 清除错误
                (void)getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len);
                if (0 == err)
                {
                    ret_val = 0;
                }
                else
                {
                    ret_val = -1;
                }
            }
            else
            {
                ret_val = -1;
            }
        }
    }

    block_or_not = 0; // 设置阻塞
    if (0 != ioctl(sockfd, FIONBIO, &block_or_not))
    {
        SOCKET_PRINTF("ioctl socket failed\n");
    }

    return ret_val;
}

/*
* @Function: socket_connect_wait
* @Description:close socket
* @Input:sHostName  ip  port 端口  uWaitMsec  超时时间  isBlock 是否阻塞 SOCKET_BLOCK 阻塞 SOCKET_NOBLOCK 非阻塞
* @Output:NULL
* @Return: NULL
*/
int socket_client_tcp_create_ipv4(const char *sHostName, int uPort, int uWaitMsec, int isBlock)
{
    int iSock = -1;
    struct sockaddr_in address;

    iSock = socket_create(false); /*建立TCP连接*/
    if (iSock < 0)
    {
        SOCKET_PRINTF("sys_socket_create：  failed!\n");
        return -1;
    }
    memset(&address,0,sizeof(address));

    address.sin_family = AF_INET;  /*ipv4协议簇*/
    address.sin_addr.s_addr = inet_addr(sHostName);
    address.sin_port = htons(uPort);

    if (0 != socket_connect_wait(iSock, &address, uWaitMsec))
    {
        socket_close(iSock);
        SOCKET_PRINTF("connect to host %s:%d in %dms failed!\n", sHostName, uPort, uWaitMsec);
        return -1;
    }
    if(1 == isBlock)  /*设置非阻塞*/
    {
        if (0 != ioctl(iSock, FIONBIO, &isBlock))
        {
            SOCKET_PRINTF("ioctl  Block socket failed\n");
            return -1;
        }
    }
    return iSock;
}


/*
* @Function: socket_accept
* @Description:socket_accept
* @Input:iSockFd  socket描述符  address  协议簇地址  uWaitMsec 超时时间  
* @Output:NULL
* @Return: 客户端socketfd
*/
static int socket_accept(int iSockFd, struct sockaddr_in *address, int uWaitMsec)
{
    int iConnFd = -1;
	int uAddrSize = 0;
	struct timeval 	stTimeout;
	fd_set rset;
    if(uWaitMsec != WAIT_FOREVER)
    {
    	stTimeout.tv_sec = uWaitMsec/1000;
    	stTimeout.tv_usec = uWaitMsec%1000;
        FD_ZERO(&rset);
        FD_SET(iSockFd, &rset);
        if(select(iSockFd + 1, &rset, NULL, NULL, &stTimeout) <= 0)
        {
            SOCKET_PRINTF("wait accept client connect failed, err:%s\n", strerror(errno));
            return -1;
    	}
    }
        uAddrSize = sizeof(struct sockaddr_in);
        if((iConnFd = accept(iSockFd, (struct sockaddr *)(address),(socklen_t *)&uAddrSize)) < 0)
        {
            SOCKET_PRINTF("socket_accept failed: errno %s\n", strerror(errno));
        }

    return iConnFd;
}

/*
* @Function: socket_server_crate
* @Description:创建一个服务端socketfd
* @Input:uPort  端口 
* @Output:NULL
* @Return: 服务端socketfd
*/
static int socket_server_crate(unsigned short uPort)
{
    int sock = -1;
    int optval = 1;
    struct sockaddr_in address;

    memset(&address,0,sizeof(struct sockaddr_in));
    sock = socket_create(false);
    if(-1 == sock)
    {
        SOCKET_PRINTF("socket crate faild!\n");
        return -1;
    }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(uPort);
    
    /*SO_REUSEADDR是让端口释放后立即就可以被再次使用*/
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if(0 != bind(sock,(struct sockaddr *)&address,sizeof(struct sockaddr_in)))
    {
        SOCKET_PRINTF("socket bind faild!\n");
        goto exit;
    }
    
    if(0 != listen(sock,SOCK_LISTEN_NUM))
    {
        SOCKET_PRINTF("socket listen faild!\n");
        goto exit;
    }

    return sock;

exit:
   socket_close(sock);
   return  -1;
}


/*
* @Function: socket_server_pthread_crate
* @Description:创建一个服务器，并对每个新链接起一个线程处理
* @Input:arg  自定义参数 （svr_t）
* @Output:NULL
* @Return:NULL
*/
void  *socket_server_pthread_crate(void *arg)
{
    struct svr_process_t *svr_process = NULL;
    int cli_sock_fd = -1;
    struct sockaddr_in cli_addr;
    pthread_attr_t attr;
    struct svr_t *svr = (struct svr_t *)arg;
    socklen_t addr_len = 0;
    int ret = 0;
    int cli_num = 0;
    pthread_t tid = (pthread_t)-1;
    if(NULL == svr->func)
    {
        SOCKET_PRINTF("svr process thread func is NULL,please define the func!\n");
        return NULL;
    }
        while(1)
        {
        memset((char *)&cli_addr, 0, sizeof (cli_addr));
        addr_len = sizeof (cli_addr);
        do {
                cli_sock_fd = socket_accept(svr->sver_sock_fd, &cli_addr, svr->uWaitMsec);
            } while ((-1 == cli_sock_fd) && (EINTR == errno));
            if (-1 == cli_sock_fd) {
                SOCKET_PRINTF("FAIL to accept, svr->sock_fd = %d, %s, TRY AGAIN\n", svr->sver_sock_fd, strerror(errno));
                sleep(1);	
                continue;
            }
            svr_process = (struct svr_process_t *)malloc(sizeof (*svr_process));
            if (NULL == svr_process) {
                SOCKET_PRINTF("OUT of memory, sizeof (*svr_process) = %d\n", (int)sizeof (*svr_process));
                SAFE_CLOSE(cli_sock_fd);
                continue;
            }
            memset((char *)svr_process, 0, sizeof (*svr_process));
            svr_process->cli_sock_fd = cli_sock_fd;
            memcpy((char *)&(svr_process->cliaddr), (char *)&cli_addr, sizeof (struct sockaddr_in));
            
            setPthreadAttr(&attr, 50, 1024 * 1024, 1);
            ret = pthread_create(&tid, NULL, svr->func, svr_process);
            pthread_attr_destroy(&attr);
            
            SOCKET_PRINTF("#######Cli_%d_Connected!\n", cli_num++);
            if (0 != ret) {
                SOCKET_PRINTF("FAIL to create svr_process_thread, %s\n", strerror(ret));
                SAFE_CLOSE(svr_process->cli_sock_fd);
                SAFE_FREE(svr_process);
                continue;
            }
        }
        socket_close(svr->sver_sock_fd);
        return NULL;
}

/*
* @Function: svr_main
* @Description:创建一个服务器，并一个线程处理
* @Input:arg  svr_t
* @Output:NULL
* @Return:成功 0  失败 -1
*/
static int svr_main(struct svr_t *svr)
{
    int ret = 0;
    pthread_attr_t attr;
    pthread_t tid = (pthread_t)-1;

    setPthreadAttr(&attr, 50, 64 * 1024, 1);
    ret = pthread_create(&tid, NULL, socket_server_pthread_crate, svr);
    pthread_attr_destroy(&attr);
    if (0 != ret) {
        SOCKET_PRINTF("FAIL to create svr_main_thread, listen_port = %u, %s\n", (unsigned int)svr->listen_port, strerror(ret));
        return -1;
    }
    return 0;
}

/*
* @Function: svr_init
* @Description:服务端初始化函数
* @Input:arg  port 端口  func 数据处理自定义回调函数，可在该函数实现收发  uWaitMsec  等待链接超时  isBlock 是否阻塞 SOCKET_BLOCK 阻塞 SOCKET_NOBLOCK 非阻塞
* @Output:NULL
* @Return:成功 0  失败 -1
*/
int svr_init(unsigned short int port, svr_process_thread func, int uWaitMsec)
{
    struct svr_t *svr = NULL;
    svr = (struct svr_t *)malloc(sizeof(struct svr_t));
    if(NULL == svr)
    {
        SOCKET_PRINTF("svr malloc faild!\n");
        return -1;
    }
    memset(svr,0,sizeof(sizeof(struct svr_t)));
    svr->sver_sock_fd = socket_server_crate(port);
    if(-1 == svr->sver_sock_fd)
    {
        SOCKET_PRINTF("socket_server_crate error!\n");
        goto EXIT;
    }
    svr->func = func;
    svr->uWaitMsec = uWaitMsec;
    svr->listen_port = port;
    if(-1 == svr_main(svr))
    {
        SOCKET_PRINTF("svr main enter error!\n");
        goto EXIT;  
    }
    SOCKET_PRINTF("svr crate sucess! PORT :  %d\n",port);
    return 0;

EXIT:
    free(svr);
    svr = NULL;
    return -1;
}

/*
* @Function: sys_socket_readn_wait
* @Description:从对端接受固定字节的数据
* @Input: sockfd：描述符  pbuf：接受缓冲区  buflen：接受字节长  uWaitMsec：超等等待时间
* @Output:NULL
* @Return:实际接收到数据的长度
*/
int sys_socket_readn_wait(int sockfd, void* pbuf, int buflen, unsigned int uWaitMsec)
{
	int	nleft;
	int	nread;
	char *ptr;
	struct timeval 	stTimeout;
	fd_set rset;

	ptr = (char *)pbuf;
	nleft = buflen;

	while (nleft > 0)
    {
    	stTimeout.tv_sec = uWaitMsec;
    	stTimeout.tv_usec = 0;
    	FD_ZERO(&rset);
    	FD_SET(sockfd, &rset);
    	if(select(sockfd+1, &rset, NULL, NULL, &stTimeout) <= 0)
        {
            /* 0--timeout */
        	return -1;
        }

    	if((nread = recv(sockfd, ptr, nleft, 0)) < 0)
        {
            
        	if(errno == EINTR)
            {
            	nread = 0;
            }
        	else
            {
            	return -1;
            }
        }
    	else if (nread == 0)
        {
        	break;
        }

    	nleft -= nread;
    	ptr   += nread;
    }
    
	return(buflen - nleft);
}


/*
* @Function: sys_socket_read_wait
* @Description:尝试从对端接受数据
* @Input: sockfd：描述符  pbuf：接受缓冲区  buflen：缓冲区大小  uWaitMsec：超等等待时间
* @Output:NULL
* @Return:实际接收到数据的长度
*/
int sys_socket_read_wait(int sockfd, void* pbuf, int bufsize, unsigned int uWaitMsec)
{
	int	nleft;
	int	nread;
	char *ptr;
	struct timeval 	stTimeout;
	fd_set rset;

	ptr = (char *)pbuf;
	nleft = bufsize;

	do
    {
    	stTimeout.tv_sec = uWaitMsec/1000;
    	stTimeout.tv_usec = (uWaitMsec%1000)*1000;

    	FD_ZERO(&rset);
    	FD_SET(sockfd, &rset);
    	if(select(sockfd+1, &rset, NULL, NULL, &stTimeout) <= 0)
        {
            /* 0--timeout */
        	return -1;
        }

    	if((nread = recv(sockfd, ptr, nleft, 0)) < 0)
        {
        	if(errno == EINTR)
            {
            	nread = 0;
            }
        	else
            {
            	return -1;
            }
        }
    	else if (nread == 0)
        {
        	break;
        }

    	nleft -= nread;
    	ptr   += nread;

    }while(0);

	return (bufsize - nleft);
}

/*
* @Function: sys_socket_writen
* @Description:向对端发送数据
* @Input: sockfd：描述符  pbuf：接受缓冲区  buflen：发送的字节数
* @Output:NULL
* @Return:实际发送的数据的长度
*/
int sys_socket_writen(int sockfd, void* pbuf, int buflen)
{
	int nleft, nwritten;
 	char *ptr;

	ptr = (char *)pbuf;
	nleft = buflen;

	while(nleft>0)
    {
    	if((nwritten = send(sockfd, ptr, nleft, MSG_NOSIGNAL)) == -1)
        {
        	if(errno == EINTR)
            {
            	SOCKET_PRINTF("EINTR\n");
            	nwritten = 0;
            }
        	else
            {
            	SOCKET_PRINTF("Send() error, %s\n", strerror(errno));
            	return -1;
            }
        }

    	nleft -= nwritten;
    	ptr   += nwritten;
    }

	return(buflen);
}