/*	�������ܣ�����IPV4��socket������
	������1��������ip��ַ
		  2�������Ķ˿�
		  3���������г�Ա��
	���أ�listen��fd			
*/
int Creat_IPV4_TCP_Socket( const char* ip, int port, int backlog )
{
	int ret;
	int sockfd = socket (AF_INET, SOCK_STREAM, 0);		// ����TCP��socket
	if( sockfd < 0)
	{
		// �������ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	/*����һ��IPv4 socket��ַ*/
	struct sockaddr_in address;
	bzero( &address, sizeof(address) );					// �ṹ������
	address.sin_family = AF_INET;						// TCP/IPv4Э����
	inet_pton(AF_INET, ip, &address.sin_addr);			// ��ipת�������ֽ������sin_addr
	address.sin_port = htons(port);						// ���˿�ת�������ֽڴ���sin_port
	
	ret = bind (sockfd, (struct sockaddr*)&address, 	// ��socket
					sizeof(address));
	if ( ret < 0)
	{
		// �����ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	ret = listen(sockfd, backlog);						// ������backlog������
	if( ret < 0)
	{
		// �������ʧ�ܣ���ӡ����ԭ�򲢷���
		printf("errno value: %d, it means: %s",errno, strerror(errno));
		return -1;
	}
	
	return sockfd;										// ���ؼ���
}