#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#define BUF_LENGTH 1024
#define MIN_BUF 128

unsigned WINAPI RequestHandler(void *arg);
char* ContentType(char* file);
void send_data(SOCKET sAccept, char* ct, char* fileName);
void send_not_found(SOCKET sAccept);

unsigned WINAPI RequestHandler(void *arg) {
	SOCKET sAccept = (SOCKET)arg;
	char buf[BUF_LENGTH];
	char method[MIN_BUF];
	char ct[MIN_BUF];
	char filename[MIN_BUF];

	recv(sAccept, buf, BUF_LENGTH, 0);
	//�Ƿ�ΪHTTP����
	if (strstr(buf, "HTTP/") == NULL) {
		send_not_found(sAccept);
		closesocket(sAccept);
		return 1;
	}

	strcpy(method, strtok(buf, " /"));
	if (strcmp(method, "GET")) {
		send_not_found(sAccept);
	}

	strcpy(filename, strtok(NULL, " /"));
	strcpy(ct, ContentType(filename));
	send_data(sAccept, ct, filename);
	return 0;
}

char* ContentType(char* file) {
	char extension[MIN_BUF];
	char filename[MIN_BUF];
	strcpy(filename, file);
	strtok(filename, ".");
	strcpy(extension, strtok(NULL, "."));
	if (!strcmp(extension, "html") || !strcmp(extension, "htm"))
		return "text/html";
	else
		return "text/plain";
}


void send_not_found(SOCKET sAccept)
{
	char proto[] = "HTTP/1.0 400 Bad Request\r\n";
	char servname[] = "Server:simple web server\r\n";
	char clen[] = "Content-length:2048\r\n";
	char ctype[] = "Content-type:text/html\r\n\r\n";
	char content[] = "<HTML><HEAD><TITLE>NETWORK</TITLE></HEAD><BODY>ERROR</BODY></HTML>";

	send(sAccept, proto, strlen(proto), 0);
	send(sAccept, servname, strlen(servname), 0);
	send(sAccept, clen, strlen(clen), 0);
	send(sAccept, ctype, strlen(ctype), 0);
	send(sAccept, content, strlen(content), 0);

	closesocket(sAccept);
}

// �����������Դ
void send_data(SOCKET sAccept,char* ct,char* filename)
{
	char proto[] = "HTTP/1.0 200 OK\r\n";
	char servname[] = "Server:simple web server\r\n";
	char clen[] = "Content-length:2048\r\n";
	char ctype[MIN_BUF];
	char buf[BUF_LENGTH];
	//printf(filename);
	FILE* sendfile = fopen(filename, "rb");

	sprintf(ctype, "Content-type:%s\r\n\r\n", ct);
	if (sendfile== NULL) {
		send_not_found(sAccept);
		return;
	}

	send(sAccept, proto, strlen(proto), 0);
	send(sAccept, servname, strlen(servname), 0);
	send(sAccept, clen, strlen(clen), 0);
	send(sAccept, ctype, strlen(ctype), 0);


	//while (fgets(buf, BUF_LENGTH, sendfile) != NULL) {
	//	send(sAccept, buf, strlen(buf), 0);
	//}

	while (1) {
		memset(buf, 0, sizeof(buf));       //������0
		fgets(buf, sizeof(buf), sendfile);

		if (SOCKET_ERROR == send(sAccept, buf, strlen(buf), 0))
		{
			printf("send() Failed:%d\n", WSAGetLastError());
			return -1;
		}
		if (feof(sendfile))
			return 0;
	}

	closesocket(sAccept);
}
int main(int argc, char* argv[]) {

	WSADATA wsaData;
	SOCKET sListen, sAccept;
	int  iLen;       //�ͻ�����ַ����
	SOCKADDR_IN ser, cli;     //�������Ϳͻ��ĵ�ַ  

	HANDLE hThread;
	DWORD dwThread;

	if (argc != 2) {
		printf("use: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed to load Winsock.\n");  //Winsock ��ʼ������   
		return -1;
	}
	else {
		printf("Server waiting\n");
	}

	sListen = socket(PF_INET, SOCK_STREAM, 0);  //�������������׽��֣������ͻ����� 
	if (sListen == INVALID_SOCKET) {
		printf("socket() Failed: %d\n", WSAGetLastError());
		return -1;
	}

	memset(&ser, 0, sizeof(ser));
	//���³�ʼ���������˵�ַ  
	ser.sin_family = AF_INET;      //ʹ�� IP ��ַ��  
	ser.sin_port = htons(atoi(argv[1]));      //������˿ں�ת��Ϊ�����ֽ���˿ں�  
	ser.sin_addr.s_addr = htonl(INADDR_ANY);  //������ IP ��ַת��Ϊ�����ֽ���������ַ 

											  //ʹ��ϵͳָ���� IP ��ַ INADDR_ANY  
	if (bind(sListen, (SOCKADDR*)&ser, sizeof(ser)) == SOCKET_ERROR)  //�׽Ӷ����ַ�İ�  
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
		sAccept = accept(sListen, (SOCKADDR *)&cli, &iLen);
		if (sAccept == INVALID_SOCKET) {
			printf("accept() Failed: %d\n", WSAGetLastError());
			return -1;
		}

		printf("�������� IP:  %s  Port:  %d  \n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

		//���߲��������߳̽������������
		hThread = (HANDLE)_beginthreadex(NULL, 0, RequestHandler, (void*)sAccept, 0, (unsigned*)&dwThread);
	}
	closesocket(sListen);      //�ر� socket  
	WSACleanup();       //������ͳɹ��ֽ���  
	return 0;
}