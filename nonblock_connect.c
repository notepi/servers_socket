/*************************************************
*
*readme
*���ڲ��Է�����connect
*��Ҫ����ip���˿�
*���ܣ����Է�����connection,����10ms����ʾ���ӳ�ʱ
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

/*���ļ����������óɷ������ģ�����ԭʼ״̬*/
int setnonblocking(int fd)
{
	int old_option = fcntl( fd, F_GETFL);					// ��ȡ��fd�ı�־
	int new_option = old_option | O_NONBLOCK;				// fd����Ϊ������ģʽ
	fcntl( fd, F_SETFL, new_option);						// fd����Ϊ������ģʽ 
	return old_option;
}


/*��ʱ���Ӻ����������ֱ��Ƿ�����IP��ַ���˿ںźͳ�ʱ�䣨���룩�������ɹ�
  ʱ�����Ѿ���������״̬��socket��ʧ�ܷ���-1
*/

int unblock_connect(const char* ip, int port, int time)
{
	int ret = 0;
	/*����һ��IPv4 socket��ַ*/
	struct sockaddr_in address;
	bzero(&address, sizeof(address));						// �ṹ������
	address.sin_family = AF_INET;							// TCP/IPv4Э����
	inet_pton(AF_INET, ip, &address.sin_addr);				// ��ipת�������ֽ������sin_addr
	address.sin_port = htons(port);							// ���˿�ת�������ֽڴ���sin_port
	
	int sockfd = socket (PF_INET, SOCK_STREAM, 0);			// ����TCP��socket
	int fdopt = setnonblocking(sockfd);						// ���ļ����������óɷ�����������ԭʼ״̬
	
	ret = connect(sockfd, (struct sockaddr*)&address,
						sizeof(address) );
	if(0 == ret)											// ���ӳɹ�
	{
		printf("connect with server immediately\n");
		fcntl( sockfd, F_SETFL, fdopt);						// fd����Ϊԭʼ״̬
		return sockfd;
	}
	else if( errno != EINPROGRESS)
	{
		/* �������û��������������ôֻ�е�errno��EINPROGRESSʱ�ű�ʾ����
		   ���ڽ��У����򷵻س���*/
		printf("unbolck connect not support\n");
		return -1;   
	}
	
	fd_set readfds;
	fd_set writefds;
	FD_ZERO( &readfds);										// ��տɶ��ļ�����������
	FD_SET( sockfd, &writefds );							// ��cnnfdλ��1������д��ӵ��ļ�����������
	
	struct timeval timeout;
	timeout.tv_sec = time;
	timeout.tv_usec  = 0;
	
	ret = select( sockfd + 1, NULL, &writefds, NULL, &timeout);
	if( ret <= 0)
	{
		/*select ��ʱ���߳�����������*/
		printf("connection time out\n");
		close(sockfd);
		return -1;
	}
	

	if ( ! FD_ISSET( sockfd, &writefds ))					// writefds��ȡλ��1
	{
		printf("no event on sockfd found\n");
		close(sockfd);
		return -1;
	}
	
		
	int error = 0;
	socklen_t length = sizeof(error);
	/*��ȡ�����sockfd�ϵĴ���*/
	if ( getsockopt( sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0)
	{
		printf("get socket option failed\n");
		close(sockfd);
		return -1;
	}
	
	/* ����Ų�Ϊ0��ʾ���ӳ���*/
	if( error != 0)
	{
		printf("connection failed after select with the error:%d\n", error);
		close(sockfd);
		return -1;
	}
	
	/*���ӳɹ�*/
	
	printf("connection ready after select with the socket:%d\n", sockfd);
	fcntl( sockfd, F_SETFL, fdopt);						// fd����Ϊԭʼ״̬
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
	const char *ip = argv[1];								// ��ȡ��ip				
	int port = atoi(argv[2]);								// ���˿ڵ�asciiת��int
	
	int sockfd = unblock_connect(ip, port, 10);				// ����������socket
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


























