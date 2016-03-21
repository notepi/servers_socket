/***********************************
*
*readme
*用于测试ET模式
*需要输入ip，端口
*功能：测试Epoll_LT
*
***********************************/

#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>


#define BUFFER_SIZE 		10
#define MAX_EVENT_NUMBER 	1024
/*将文件描述符设置成非阻塞的*/
int setnonblocking(int fd)
{
	int old_option = fcntl( fd, F_GETFL);						// 获取旧fd的标志
	int new_option = old_option | O_NONBLOCK;					// fd设置为非阻塞模式
	fcntl( fd, F_SETFL, new_option);							// fd设置为非阻塞模式 
	return old_option;
}

/*将文件描述符fd的EPOLLEN注册到epollfd指示的epoll内核事件中，
  参数enable_et指定是否fd启动ET模式*/
  void addfd(int epollfd, int fd, int enable_et)
  {
	  struct epoll_event event;
	  event.data.fd = fd;
	  event.events = EPOLLIN;									// 设置数据可读
	  if(1 == enable_et)
	  {
		  event.events |= EPOLLET;								// 配置为ET模式，默认是LT模式
	  }
	  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);			// 在epoll中添加事件	
	  setnonblocking(fd);										// 将事件配置成非阻塞模式
	  
  }
  
/*LT模式的工作流程*/
void lt(struct epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	int i= 0;
	for( i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;								// 读取事件的文件标识符
		if (sockfd == listenfd)										// 判断是本地监听端口,还是客户端连过来的端口
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof (client_address);
			int connfd = accept(listenfd, (struct sockaddr *)&client_address, 
								&client_addrlength);				// 连接socket
			addfd(epollfd, connfd, 0);								// 对connfd禁用ET模式
		}
		else if(events[i].events &EPOLLIN)							// 有数据可读
		{
			/*只要socket读缓存中还有未读取的数据，这段代码就被触发*/
			printf("event trigger once\n");
			memset(buf, '\0', BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
			if(ret <= 0)											// 未读取到数据
			{
				close( sockfd);
				continue;
			}
			printf("get %d bytes of content:%s\n", ret, buf);
		}
		else
		{
			printf("someting else happened\n");
		}
	}
}

/*LT模式的工作流程*/
void et(struct epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	int i;
	for( i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;								// 读取事件的文件标识符
		if (sockfd == listenfd)										// 判断是本地监听端口,还是客户端连过来的端口
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof (client_address);
			int connfd = accept(listenfd, (struct sockaddr *)&client_address, 
								&client_addrlength);				// 连接socket
			addfd(epollfd, connfd, 1);								// 对connfd禁用LT模式
		}
		else if(events[i].events &EPOLLIN)							// 有数据可读
		{
			/*这段代码不会被重复触发，所以我们循环读取数据，
			  以确保把socket读缓存中所有数据读出*/
			printf("event trigger once\n");
			while(1)
			{
				memset(buf, '\0', BUFFER_SIZE);
				int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);	// 读取数据
				if (ret < 0)
				{
					/*对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕。
					  此后,epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下
					  一次操作*/
					  if((errno == EAGAIN) || (errno == EWOULDBLOCK))
					  {
						  printf("read later\n");
						  break;
					  }
					close(sockfd);
					break;
				}
				else if (ret == 0)
				{
					close(sockfd);
				}
				else
				{
					printf("get %d bytes of content:%s\n", ret, buf);
				}
			}
		}
		else
		{
			printf("someting else happen\n");
		}
	}
}
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("usge:%s ip_address port_number backlog\n",
				basename(argv[0]));
		return -1;
	}
	const char *ip = argv[1];								// 读取绑定ip				
	int port = atoi(argv[2]);								// 将端口的ascii转成int
	
	int listenfd = socket (AF_INET, SOCK_STREAM, 0);		// 创建TCP的socket
	// 如果其值为假（即为0），那么它先向stderr打印一条出错信息，然后通过调用 abort 来终止程序运行。
	assert( listenfd >= 0);
	
	/*创建一个IPv4 socket地址*/
	struct sockaddr_in address;
	bzero(&address, sizeof(address));						// 结构体清零
	address.sin_family = AF_INET;							// TCP/IPv4协议族
	inet_pton(AF_INET, ip, &address.sin_addr);				// 将ip转成网络字节序存入sin_addr
	address.sin_port = htons(port);							// 将端口转成网络字节存入sin_port
			
	int ret = bind (listenfd, (struct sockaddr*)&address, 	// 绑定socket
					sizeof(address));
	assert (ret != -1);										// 绑定失败则退出
	
	ret = listen(listenfd, 5);								// 最多监听5个连接
	assert (ret != -1);										// 监听失败则退出
	
	printf("in listen\n");
	struct epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert( epollfd != -1);
//	addfd(epollfd, listenfd, 1);							// 禁用LT模式，启用ET模式
	addfd(epollfd, listenfd, 0);							// 禁用ET模式，启用LT模式
	while(1)
	{
		printf("in while\n");
		int ret = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0)
		{
			printf("epoll failure\n");
			break;
		}
		lt(events, ret, epollfd, listenfd);					// 使用LT模式
		//et(events, ret, epollfd, listenfd);					// 使用ET模式
	}
	close(listenfd);
	return 0;
}











