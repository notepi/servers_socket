/*	函数功能：创建IPV4的socket并返回
	参数：1：监听的ip地址
		  2：监听的端口
		  3：监听队列成员数
	返回：listen的fd			
*/
int Creat_IPV4_TCP_Socket( const char* ip, int port, int backlog )
{
	int ret;
	int sockfd = socket (AF_INET, SOCK_STREAM, 0);		// 创建TCP的socket
	if( sockfd < 0)
	{
		// 如果创建失败，打印错误原因并返回
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	/*创建一个IPv4 socket地址*/
	struct sockaddr_in address;
	bzero( &address, sizeof(address) );					// 结构体清零
	address.sin_family = AF_INET;						// TCP/IPv4协议族
	inet_pton(AF_INET, ip, &address.sin_addr);			// 将ip转成网络字节序存入sin_addr
	address.sin_port = htons(port);						// 将端口转成网络字节存入sin_port
	
	ret = bind (sockfd, (struct sockaddr*)&address, 	// 绑定socket
					sizeof(address));
	if ( ret < 0)
	{
		// 如果绑定失败，打印错误原因并返回
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	ret = listen(sockfd, backlog);						// 最多监听backlog个连接
	if( ret < 0)
	{
		// 如果创建失败，打印错误原因并返回
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	return sockfd;										// 返回监听
}