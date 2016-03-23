/*************************************************
*
*readme
*用于测试非阻塞connect
*需要输入ip，端口
*功能：测试非阻塞connection,超过10ms后提示连接超时
*
**************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

/*将文件描述符设置成非阻塞的，返回原始状态*/
int setnonblocking(int fd)
{
	int old_option = fcntl( fd, F_GETFL);					// 获取旧fd的标志
	int new_option = old_option | O_NONBLOCK;				// fd设置为非阻塞模式
	fcntl( fd, F_SETFL, new_option);						// fd设置为非阻塞模式 
	return old_option;
}


/*超时连接函数，参数分别是服务器IP地址，端口号和超时间（毫秒）。函数成功
  时返回已经处于连接状态的socket，失败返回-1
*/

int unblock_connect(const char* ip, int port, int time)
{
	int ret = 0;
	/*创建一个IPv4 socket地址*/
	struct sockaddr_in address;
	bzero(&address, sizeof(address));						// 结构体清零
	address.sin_family = AF_INET;							// TCP/IPv4协议族
	inet_pton(AF_INET, ip, &address.sin_addr);				// 将ip转成网络字节序存入sin_addr
	address.sin_port = htons(port);							// 将端口转成网络字节存入sin_port
	
	int sockfd = socket (PF_INET, SOCK_STREAM, 0);			// 创建TCP的socket
	int fdopt = setnonblocking(sockfd);						// 将文件描述符设置成非阻塞，保存原始状态
	
	ret = connect(sockfd, (struct sockaddr*)&address,
						sizeof(address) );
	if(0 == ret)											// 连接成功
	{
		printf("connect with server immediately\n");
		fcntl( sockfd, F_SETFL, fdopt);						// fd设置为原始状态
		return sockfd;
	}
	else if( errno != EINPROGRESS)
	{
		/* 如果连接没有立即建立，那么只有当errno是EINPROGRESS时才表示连接
		   还在进行，否则返回出错*/
		printf("unbolck connect not support\n");
		return -1;   
	}
	
	fd_set readfds;
	fd_set writefds;
	FD_ZERO( &readfds);										// 清空可读文件描述符集合
	FD_SET( sockfd, &writefds );							// 将cnnfd位置1，将可写添加到文件描述符集合
	
	struct timeval timeout;
	timeout.tv_sec = time;
	timeout.tv_usec  = 0;
	
	ret = select( sockfd + 1, NULL, &writefds, NULL, &timeout);
	if( ret <= 0)
	{
		/*select 超时或者出错，立即返回*/
		printf("connection time out\n");
		close(sockfd);
		return -1;
	}
	

	if ( ! FD_ISSET( sockfd, &writefds ))					// writefds读取位置1
	{
		printf("no event on sockfd found\n");
		close(sockfd);
		return -1;
	}
	
		
	int error = 0;
	socklen_t length = sizeof(error);
	/*获取并清除sockfd上的错误*/
	if ( getsockopt( sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0)
	{
		printf("get socket option failed\n");
		close(sockfd);
		return -1;
	}
	
	/* 错误号不为0表示连接出错*/
	if( error != 0)
	{
		printf("connection failed after select with the error:%d\n", error);
		close(sockfd);
		return -1;
	}
	
	/*连接成功*/
	
	printf("connection ready after select with the socket:%d\n", sockfd);
	fcntl( sockfd, F_SETFL, fdopt);						// fd设置为原始状态
	return sockfd;
}


int main( int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("usge:%s ip_address port_number backlog\n",
				basename(argv[0]));
		return -1;
	}
	const char *ip = argv[1];								// 读取绑定ip				
	int port = atoi(argv[2]);								// 将端口的ascii转成int
	
	int sockfd = unblock_connect(ip, port, 10);				// 非阻塞连接socket
	if ( sockfd < 0)
	{
		return 1;
	}
    shutdown( sockfd, SHUT_WR );
    sleep( 200 );
    printf( "send data out\n" );
    send( sockfd, "abc", 3, 0 );
	return 0;
}


























