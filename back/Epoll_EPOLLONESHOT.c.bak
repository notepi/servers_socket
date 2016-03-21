/***********************************
*
*readme
*用于测试ET模式
*需要输入ip，端口
*功能：测试Epoll_ET_EPOLLONESHOT
*
***********************************/

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


struct fds
{
   int epollfd;
   int sockfd;
};

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

/*将文件描述符fd的EPOLLEN注册到epollfd指示的epoll内核事件中(ET模式)，
  参数enable_et指定是否注册fd上的EPOLLONESHOT事件*/
void addfd(int epollfd, int fd, int enable_et)
{
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET;								// 设置数据可读，ET模式
  if(1 == enable_et)
  {
	  event.events |= EPOLLONESHOT;								// 配置为EPOLLONESHOT模式
  }
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);				// 在epoll中添加事件	
  setnonblocking(fd);											// 将事件配置成非阻塞模式
}
  
  
/*	重置fd上的事件。这样操作之后，尽管fd上的EPOLLONESHOT事件被注册，
	但是操作系统仍然会触发fd上的EPOLLIN事件，且只触发一次 
*/
void reset_oneshot(int epollfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;			// 设置数据可读，ET模式，EPOLLONESHOT
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);				// 在epoll中修改事件	
}
 
 
 
/*工作线程*/
void *worker(void *arg)
{
	int sockfd = ((struct fds*)arg)->sockfd;
	int epollfd = ((struct fds*)arg)->epollfd;
	printf("strart new thread to receive data on fd:%d\n", sockfd);
	char buf[BUFFER_SIZE];
	memset(buf, '\0', BUFFER_SIZE);
	/*循环读取sockfd上的数据，直到遇到EAGIN错误*/
	while(1)
	{
		int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);	// 读取数据
		if (ret < 0)
		{
			if(errno == EAGAIN)
			{
				reset_oneshot( epollfd, sockfd);
				printf("read later\n");
				break;
			}
		}
		else if (ret == 0)
		{
			close(sockfd);
			printf("read later\n");
			break;
		}
		else
		{
			printf("get %d bytes of content:%s\n", ret, buf);
			/*休眠5s，模拟数据处理过程*/
			sleep(5);
		}
	}
	printf("end thread receiving data on fd:%d\n", sockfd);
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

	/*注意，监听socket listenfd上是不能注册EPOLLONESHOT事件的，
	否则应用程序只能处理一个客户连接！因为后续的客户请求将不再
	触发listenfd上的EPOLLIN*/
	addfd(epollfd, listenfd, 0);	
	
	while(1)
	{
		printf("in while\n");
		int ret = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1);
		printf("epoll_wait\n");
		if (ret < 0)
		{
			printf("epoll failure\n");
			break;
		}
		int i;
		for( i = 0; i < ret; i++)
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
				pthread_t thread;
				struct fds fds_for_new_worker;
				fds_for_new_worker.epollfd = epollfd;
				fds_for_new_worker.sockfd = sockfd;
				/*新启动一个线程为sockfd服务*/
				pthread_create(&thread, NULL, worker, (void *)&fds_for_new_worker);
			}
			else
			{
				printf("someting else happened\n");
			}
		}
	}
	close(listenfd);
	return 0;
}











