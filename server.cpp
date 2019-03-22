#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>
#include <string.h>
#define DEFAULT_PORT 8080
#define BUF_LENGTH 1024
#define MIN_BUF 128
#define SERVER "Server: csr_http1.1\r\n"

int file_not_found(SOCKET sAccept);
int file_ok(SOCKET sAccept, long flen);
int send_file(SOCKET sAccept, FILE *resource);
int send_not_found(SOCKET sAccept);

DWORD WINAPI SimpleHTTPServer(LPVOID lparam)
{
	SOCKET sAccept = (SOCKET)(LPVOID)lparam;
	char recv_buf[BUF_LENGTH];
	char method_req[MIN_BUF];
	char method_http[MIN_BUF];
	char url[MIN_BUF];
	char path[_MAX_PATH];

	// ������0
	memset(recv_buf, 0, sizeof(recv_buf));
	if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == SOCKET_ERROR)   //���մ���
	{
		printf("recv() Failed:%d\n", WSAGetLastError());
		return -1;
	}
	else
		printf("recv data from client:%s\n", recv_buf); //���ճɹ�����ӡ������

	

	//�����������
	int i = 0;
	int j = 0;
	// ȡ����һ������
	while (!(' ' == recv_buf[j]) && (i < sizeof(method_req) - 1))
	{
		method_req[i] = recv_buf[j];
		i++; j++;
	}
	method_req[i] = '\0';   // ������


	if (stricmp(method_req, "GET") && stricmp(method_req, "HEAD"))
	{
		closesocket(sAccept); //�ͷ������׽��֣�������ÿͻ���ͨ��
		printf("not get or head method.\nclose ok.\n");
		printf("please use HTTP/1.1\n");
		printf("***********************\n\n\n\n");
		return -1;
	}
	printf("method_req: %s\n", method_req);

	// ��ȡ���ڶ�������(url�ļ�·�����ո����)������'/'��Ϊwindows�µ�·���ָ���'\'
	// ����ֻ���Ǿ�̬����
	i = 0;
	while ((' ' == recv_buf[j]) && (j < sizeof(recv_buf)))
		j++;
	while (!(' ' == recv_buf[j]) && (i < sizeof(recv_buf) - 1) && (j < sizeof(recv_buf)))
	{
		if (recv_buf[j] == '/')
			url[i] = '\\';
		else if (recv_buf[j] == ' ')
			break;
		else
			url[i] = recv_buf[j];
		i++; j++;
	}
	url[i] = '\0';
	printf("url: %s\n", url);

	// �������url·��ת��Ϊ����·��
	_getcwd(path, _MAX_PATH);
	strcat(path, url);
	printf("path: %s\n", path);

	// �򿪱���·���µ��ļ������紫������r�ı���ʽ�򿪻����
	FILE *resource = fopen(path, "rb");

	// û�и��ļ�����һ���򵥵�404-file not found��htmlҳ�棬���Ͽ���������
	if (resource == NULL)
	{
		file_not_found(sAccept);
		// ���method��GET�������Զ����file not foundҳ��
		if (0 == stricmp(method_req, "GET"))
			send_not_found(sAccept);

		closesocket(sAccept); //�ͷ������׽��֣�������ÿͻ���ͨ��
		printf("file not found.\nclose ok.\n");
		printf("***********************\n\n\n\n");
		return -1;
	}

	// ����ļ����ȣ��ǵ������ļ�ָ�뵽�ļ�ͷ
	fseek(resource, 0, SEEK_SET);
	fseek(resource, 0, SEEK_END);
	long flen = ftell(resource);
	printf("file length: %ld\n", flen);
	fseek(resource, 0, SEEK_SET);

	// ����200 OK HEAD
	file_ok(sAccept, flen);

	// �����GET���������������Դ
	if (0 == stricmp(method_req, "GET"))
	{
		if (0 == send_file(sAccept, resource))
			printf("file send ok.\n");
		else
			printf("file send fail.\n");
	}
	fclose(resource);

	closesocket(sAccept); //�ͷ������׽��֣�������ÿͻ���ͨ��
	printf("close ok.\n");
	printf("***********************\n\n\n\n");

	return 0;

}

// ����404 file_not_found��ͷ
int file_not_found(SOCKET sAccept)
{
	char send_buf[MIN_BUF];
	//  time_t timep;   
	//  time(&timep);
	sprintf(send_buf, "HTTP/1.1 404 NOT FOUND\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	//  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
	//  send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Connection: keep-alive\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, SERVER);
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Content-Type: text/html\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	return 0;
}

// ����200 ok��ͷ
int file_ok(SOCKET sAccept, long flen)
{
	char send_buf[MIN_BUF];
	//  time_t timep;
	//  time(&timep);
	sprintf(send_buf, "HTTP/1.1 200 OK\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Connection: keep-alive\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	//  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
	//  send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, SERVER);
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Content-Length: %ld\r\n", flen);
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Content-Type: text/html\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	return 0;
}

// �����Զ����file_not_foundҳ��
int send_not_found(SOCKET sAccept)
{
	char send_buf[MIN_BUF];
	sprintf(send_buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "<BODY><h1 align='center'>404</h1><br/><h1 align='center'>file not found.</h1>\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "</BODY></HTML>\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	return 0;
}

// �����������Դ
int send_file(SOCKET sAccept, FILE *resource)
{
	char send_buf[BUF_LENGTH];
	while (1)
	{
		memset(send_buf, 0, sizeof(send_buf));       //������0
		fgets(send_buf, sizeof(send_buf), resource);
		//  printf("send_buf: %s\n",send_buf);
		if (SOCKET_ERROR == send(sAccept, send_buf, strlen(send_buf), 0))
		{
			printf("send() Failed:%d\n", WSAGetLastError());
			return -1;
		}
		if (feof(resource))
			return 0;
	}
}
int main(int argc, char* argv[]) {
	int  iPort = DEFAULT_PORT; //�˿ں�
	WSADATA wsaData;
	SOCKET sListen, sAccept;
	int  iLen;       //�ͻ�����ַ����
	struct sockaddr_in ser, cli;     //�������Ϳͻ��ĵ�ַ  

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed to load Winsock.\n");  //Winsock ��ʼ������   
		return -1;
	}
	else {
		printf("Server waiting\n");
	}

	sListen = socket(AF_INET, SOCK_STREAM, 0);  //�������������׽��֣������ͻ����� 
	if (sListen == INVALID_SOCKET) {
		printf("socket() Failed: %d\n", WSAGetLastError());
		return -1;
	}
	//���³�ʼ���������˵�ַ  
	ser.sin_family = AF_INET;      //ʹ�� IP ��ַ��  
	ser.sin_port = htons(iPort);      //������˿ں�ת��Ϊ�����ֽ���˿ں�  
	ser.sin_addr.s_addr = htonl(INADDR_ANY);  //������ IP ��ַת��Ϊ�����ֽ���������ַ 

											  //ʹ��ϵͳָ���� IP ��ַ INADDR_ANY  
	if (bind(sListen, (LPSOCKADDR)&ser, sizeof(ser)) == SOCKET_ERROR)  //�׽Ӷ����ַ�İ�  
	{
		printf("bind() Failed: %d\n", WSAGetLastError());
		return -1;
	}

	//ͨ���׽��ֽ��м���
	if (listen(sListen, 5) == SOCKET_ERROR)   //�������״̬  
	{
		printf("lisiten() Failed: %d\n", WSAGetLastError());
		return -1;
	}

	while (1)        //����ѭ���ȴ��ͻ�����������  
	{
		iLen = sizeof(cli);       //��ʼ���ͻ��˵�ַ���Ȳ���
		sAccept = accept(sListen, (struct sockaddr *)&cli, &iLen);
		if (sAccept == INVALID_SOCKET) {
			printf("accept() Failed: %d\n", WSAGetLastError());
			return -1;
		}

		printf("�������� IP:  %s  Port:  %d  \n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

		//���߲��������߳̽������������
		DWORD ThreadID;
		CreateThread(NULL, 0, SimpleHTTPServer, (LPVOID)sAccept, 0, &ThreadID);
	}
	closesocket(sListen);      //�ر� socket  
	WSACleanup();       //������ͳɹ��ֽ���  
	return 0;
}