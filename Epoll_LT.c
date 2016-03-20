/***********************************
*
*readme
*���ڲ���ETģʽ
*��Ҫ����ip���˿�
*���ܣ�����Epoll_LT
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
/*���ļ����������óɷ�������*/
int setnonblocking(int fd)
{
	int old_option = fcntl( fd, F_GETFL);						// ��ȡ��fd�ı�־
	int new_option = old_option | O_NONBLOCK;					// fd����Ϊ������ģʽ
	fcntl( fd, F_SETFL, new_option);							// fd����Ϊ������ģʽ 
	return old_option;
}

/*���ļ�������fd��EPOLLENע�ᵽepollfdָʾ��epoll�ں��¼��У�
  ����enable_etָ���Ƿ�fd����ETģʽ*/
  void addfd(int epollfd, int fd, int enable_et)
  {
	  struct epoll_event event;
	  event.data.fd = fd;
	  event.events = EPOLLIN;									// �������ݿɶ�
	  if(1 == enable_et)
	  {
		  event.events |= EPOLLET;								// ����ΪETģʽ��Ĭ����LTģʽ
	  }
	  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);			// ��epoll������¼�	
	  setnonblocking(fd);										// ���¼����óɷ�����ģʽ
	  
  }
  
/*LTģʽ�Ĺ�������*/
void lt(struct epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	int i= 0;
	for( i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;								// ��ȡ�¼����ļ���ʶ��
		if (sockfd == listenfd)										// �ж��Ǳ��ؼ����˿�,���ǿͻ����������Ķ˿�
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof (client_address);
			int connfd = accept(listenfd, (struct sockaddr *)&client_address, 
								&client_addrlength);				// ����socket
			addfd(epollfd, connfd, 0);								// ��connfd����ETģʽ
		}
		else if(events[i].events &EPOLLIN)							// �����ݿɶ�
		{
			/*ֻҪsocket�������л���δ��ȡ�����ݣ���δ���ͱ�����*/
			printf("event trigger once\n");
			memset(buf, '\0', BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
			if(ret <= 0)											// δ��ȡ������
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

/*LTģʽ�Ĺ�������*/
void et(struct epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	int i;
	for( i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;								// ��ȡ�¼����ļ���ʶ��
		if (sockfd == listenfd)										// �ж��Ǳ��ؼ����˿�,���ǿͻ����������Ķ˿�
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof (client_address);
			int connfd = accept(listenfd, (struct sockaddr *)&client_address, 
								&client_addrlength);				// ����socket
			addfd(epollfd, connfd, 1);								// ��connfd����LTģʽ
		}
		else if(events[i].events &EPOLLIN)							// �����ݿɶ�
		{
			/*��δ��벻�ᱻ�ظ���������������ѭ����ȡ���ݣ�
			  ��ȷ����socket���������������ݶ���*/
			printf("event trigger once\n");
			while(1)
			{
				memset(buf, '\0', BUFFER_SIZE);
				int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);	// ��ȡ����
				if (ret < 0)
				{
					/*���ڷ�����IO�����������������ʾ�����Ѿ�ȫ����ȡ��ϡ�
					  �˺�,epoll�����ٴδ���sockfd�ϵ�EPOLLIN�¼�����������
					  һ�β���*/
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
	const char *ip = argv[1];								// ��ȡ��ip				
	int port = atoi(argv[2]);								// ���˿ڵ�asciiת��int
	
	int listenfd = socket (AF_INET, SOCK_STREAM, 0);		// ����TCP��socket
	// �����ֵΪ�٣���Ϊ0������ô������stderr��ӡһ��������Ϣ��Ȼ��ͨ������ abort ����ֹ�������С�
	assert( listenfd >= 0);
	
	/*����һ��IPv4 socket��ַ*/
	struct sockaddr_in address;
	bzero(&address, sizeof(address));						// �ṹ������
	address.sin_family = AF_INET;							// TCP/IPv4Э����
	inet_pton(AF_INET, ip, &address.sin_addr);				// ��ipת�������ֽ������sin_addr
	address.sin_port = htons(port);							// ���˿�ת�������ֽڴ���sin_port
			
	int ret = bind (listenfd, (struct sockaddr*)&address, 	// ��socket
					sizeof(address));
	assert (ret != -1);										// ��ʧ�����˳�
	
	ret = listen(listenfd, 5);								// ������5������
	assert (ret != -1);										// ����ʧ�����˳�
	
	printf("in listen\n");
	struct epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert( epollfd != -1);
//	addfd(epollfd, listenfd, 1);							// ����LTģʽ������ETģʽ
	addfd(epollfd, listenfd, 0);							// ����ETģʽ������LTģʽ
	while(1)
	{
		printf("in while\n");
		int ret = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0)
		{
			printf("epoll failure\n");
			break;
		}
		lt(events, ret, epollfd, listenfd);					// ʹ��LTģʽ
		//et(events, ret, epollfd, listenfd);					// ʹ��ETģʽ
	}
	close(listenfd);
	return 0;
}











