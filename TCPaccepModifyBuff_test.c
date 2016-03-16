/***********************************
*
*readme
*用于读取信息
*需要输入ip，端口，listen的连接数
*功能：输出连接的发来的数据
*
***********************************/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	
	if (argc < 5)
	{
		printf("usge:%s ip_address port_number backlog\n",
				basename(argv[0]));
		return -1;
	}
	const char *ip = argv[1];							// 读取绑定ip				
	int port = atoi(argv[2]);							// 将端口的ascii转成int
	int backlog = atoi(argv[3]);						// 将监听链接数量的ascii转成int
	
	
	int sock = socket (AF_INET, SOCK_STREAM, 0);		// 创建TCP的socket
	// 如果其值为假（即为0），那么它先向stderr打印一条出错信息，然后通过调用 abort 来终止程序运行。
	assert( sock >= 0);
	
	/*创建一个IPv4 socket地址*/
	struct sockaddr_in address;
	bzero(&address, sizeof(address));					// 结构体清零
	address.sin_family = AF_INET;						// TCP/IPv4协议族
	inet_pton(AF_INET, ip, &address.sin_addr);			// 将ip转成网络字节序存入sin_addr
	address.sin_port = htons(port);						// 将端口转成网络字节存入sin_port
	
	int recvbuf = atoi(argv[4]);						// 读取接收buffer的大小
	int len = sizeof(recvbuf);
	/*先设置TCP接收缓冲区的大小，然后读取输出*/
	getsockopt(	sock, SOL_SOCKET, SO_RCVBUF, &recvbuf,	// 读取TCP接收缓存区
				(socklen_t*)&len);	
	printf(	"the tcp receive buffer size after setting is:%d\n",
		recvbuf);		
	setsockopt(	sock, SOL_SOCKET, SO_RCVBUF, &recvbuf,	// 设置TCP接收缓存区为recvbuf
				sizeof(recvbuf));	 
	getsockopt(	sock, SOL_SOCKET, SO_RCVBUF, &recvbuf,	// 读取TCP接收缓存区
				(socklen_t*)&len);	
	printf(	"the tcp receive buffer size after setting is:%d\n",
			recvbuf);
			
	int ret = bind (sock, (struct sockaddr*)&address, 	// 绑定socket
					sizeof(address));
	assert (ret != -1);									// 绑定失败则退出
	
	ret = listen(sock, backlog);						// 最多监听backlog个连接
	assert (ret != -1);									// 监听失败则退出
	
	
	struct sockaddr_in client;							// client信息结构体
	socklen_t client_addrlength = sizeof(client);		// socket地址长度
	/*接收socket*/
	int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
	if(connfd < 0)
	{
		printf("errno is:%d\n",errno);
	}
	else
	{
		/*接收连接成功则打印客户端的IP地址和端口号*/	
		char buffer[BUF_SIZE];										// 定义缓存区
		
		memset(buffer, '\0', BUF_SIZE);								// 清空缓存区			
		while( recv( connfd, buffer, BUF_SIZE - 1, 0) > 0 ){};
		
		close(connfd);												// 关闭accep连接
	}

	close(sock);													// 关闭socket
	return 0;	
}






