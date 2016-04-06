/*	�������ܣ�����IPV4��socket������
	������1��������ip��ַ������ַΪNULLʱ���������С�
		  2�������Ķ˿�
		  3���������г�Ա��
	���أ�listen��fd			
*/

int Creat_IPV4_TCP_Socket( const char* ip, int port, int backlog )
{
	int ret;
	int sockfd = socket (AF_INET, SOCK_STREAM, 0);		// ����TCP��socket
	if( sockfd < 0){
		// �������ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	/*����һ��IPv4 socket��ַ*/
	struct sockaddr_in address;
	bzero( &address, sizeof(address) );					// �ṹ������
	address.sin_family = AF_INET;						// TCP/IPv4Э����
	address.sin_port = htons(port);						// ���˿�ת�������ֽڴ���sin_port
	if (NULL == ip){									// ��ַΪ�����������
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else{												// �����ض�IP
		inet_pton(AF_INET, ip, &address.sin_addr);		// ��ipת�������ֽ������sin_addr
	}
	
	ret = bind (sockfd, (struct sockaddr*)&address, 	// ��socket
					sizeof(address));
	if ( ret < 0){
		// �����ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	ret = listen(sockfd, backlog);						// ������backlog������
	if( ret < 0){
		// �������ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	return sockfd;										// ���ؼ���
}


/*	�������ܣ�����IPV4��socket���󶨷���
	������1��������ip��ַ������ַΪNULLʱ���������С�
		  2�������Ķ˿�
		  3���������г�Ա��
	���أ�listen��fd			
*/
int Creat_IPV4_UDP_Socket( const char* ip, int port)
{
	
	int sock = socket (AF_INET, SOCK_DGRAM, 0);			// ����UDP��socket
	// �����ֵΪ�٣���Ϊ0������ô������stderr��ӡһ��������Ϣ��Ȼ��ͨ������ abort ����ֹ�������С�
	if ( sock < 0){
		// �������ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	/*����һ��IPv4 socket��ַ*/
	struct sockaddr_in address;
	bzero(&address, sizeof(address));					// �ṹ������
	address.sin_family = AF_INET;						// TCP/IPv4Э����
	if (NULL == ip){									// ��ַΪ�����������
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else{												// �����ض�IP
		inet_pton(AF_INET, ip, &address.sin_addr);		// ��ipת�������ֽ������sin_addr
	}
	address.sin_port = htons(port);						// ���˿�ת�������ֽڴ���sin_port
	
	int ret = bind (sock, (struct sockaddr*)&address, 	// ��socket
				sizeof(address));
	if ( ret < 0){
		// ��ʧ�����˳�����ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	return sock;
	
}

/*���ļ����������óɷ�������*/
/*	�������ܣ����ļ����������óɷ������ģ��������ļ�������ԭʼ״̬
	������1���ļ�������
	���أ�ԭʼ�ļ�������			
*/
int SetNonblocking(int fd)
{
	int ret;
	int old_option = fcntl( fd, F_GETFL);					// ��ȡ��fd�ı�־
	int new_option = old_option | O_NONBLOCK;				// fd����Ϊ������ģʽ
	ret = fcntl( fd, F_SETFL, new_option);					// fd����Ϊ������ģʽ 
	if ( ret < 0){
		// �������ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	return old_option;
}





