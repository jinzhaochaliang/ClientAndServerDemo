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

	// 缓存清0
	memset(recv_buf, 0, sizeof(recv_buf));
	if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == SOCKET_ERROR)   //接收错误
	{
		printf("recv() Failed:%d\n", WSAGetLastError());
		return -1;
	}
	else
		printf("recv data from client:%s\n", recv_buf); //接收成功，打印请求报文

	

	//处理接收数据
	int i = 0;
	int j = 0;
	// 取出第一个单词
	while (!(' ' == recv_buf[j]) && (i < sizeof(method_req) - 1))
	{
		method_req[i] = recv_buf[j];
		i++; j++;
	}
	method_req[i] = '\0';   // 结束符


	if (stricmp(method_req, "GET") && stricmp(method_req, "HEAD"))
	{
		closesocket(sAccept); //释放连接套接字，结束与该客户的通信
		printf("not get or head method.\nclose ok.\n");
		printf("please use HTTP/1.1\n");
		printf("***********************\n\n\n\n");
		return -1;
	}
	printf("method_req: %s\n", method_req);

	// 提取出第二个单词(url文件路径，空格结束)，并把'/'改为windows下的路径分隔符'\'
	// 这里只考虑静态请求
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

	// 将请求的url路径转换为本地路径
	_getcwd(path, _MAX_PATH);
	strcat(path, url);
	printf("path: %s\n", path);

	// 打开本地路径下的文件，网络传输中用r文本方式打开会出错
	FILE *resource = fopen(path, "rb");

	// 没有该文件则发送一个简单的404-file not found的html页面，并断开本次连接
	if (resource == NULL)
	{
		file_not_found(sAccept);
		// 如果method是GET，则发送自定义的file not found页面
		if (0 == stricmp(method_req, "GET"))
			send_not_found(sAccept);

		closesocket(sAccept); //释放连接套接字，结束与该客户的通信
		printf("file not found.\nclose ok.\n");
		printf("***********************\n\n\n\n");
		return -1;
	}

	// 求出文件长度，记得重置文件指针到文件头
	fseek(resource, 0, SEEK_SET);
	fseek(resource, 0, SEEK_END);
	long flen = ftell(resource);
	printf("file length: %ld\n", flen);
	fseek(resource, 0, SEEK_SET);

	// 发送200 OK HEAD
	file_ok(sAccept, flen);

	// 如果是GET方法则发送请求的资源
	if (0 == stricmp(method_req, "GET"))
	{
		if (0 == send_file(sAccept, resource))
			printf("file send ok.\n");
		else
			printf("file send fail.\n");
	}
	fclose(resource);

	closesocket(sAccept); //释放连接套接字，结束与该客户的通信
	printf("close ok.\n");
	printf("***********************\n\n\n\n");

	return 0;

}

// 发送404 file_not_found报头
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

// 发送200 ok报头
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

// 发送自定义的file_not_found页面
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

// 发送请求的资源
int send_file(SOCKET sAccept, FILE *resource)
{
	char send_buf[BUF_LENGTH];
	while (1)
	{
		memset(send_buf, 0, sizeof(send_buf));       //缓存清0
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
	int  iPort = DEFAULT_PORT; //端口号
	WSADATA wsaData;
	SOCKET sListen, sAccept;
	int  iLen;       //客户机地址长度
	struct sockaddr_in ser, cli;     //服务器和客户的地址  

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed to load Winsock.\n");  //Winsock 初始化错误   
		return -1;
	}
	else {
		printf("Server waiting\n");
	}

	sListen = socket(AF_INET, SOCK_STREAM, 0);  //创建服务器端套接字，监听客户请求 
	if (sListen == INVALID_SOCKET) {
		printf("socket() Failed: %d\n", WSAGetLastError());
		return -1;
	}
	//以下初始化服务器端地址  
	ser.sin_family = AF_INET;      //使用 IP 地址族  
	ser.sin_port = htons(iPort);      //主机序端口号转换为网络字节序端口号  
	ser.sin_addr.s_addr = htonl(INADDR_ANY);  //主机序 IP 地址转换为网络字节序主机地址 

											  //使用系统指定的 IP 地址 INADDR_ANY  
	if (bind(sListen, (LPSOCKADDR)&ser, sizeof(ser)) == SOCKET_ERROR)  //套接定与地址的绑定  
	{
		printf("bind() Failed: %d\n", WSAGetLastError());
		return -1;
	}

	//通过套接字进行监听
	if (listen(sListen, 5) == SOCKET_ERROR)   //进入监听状态  
	{
		printf("lisiten() Failed: %d\n", WSAGetLastError());
		return -1;
	}

	while (1)        //进入循环等待客户的连接请求  
	{
		iLen = sizeof(cli);       //初始化客户端地址长度参数
		sAccept = accept(sListen, (struct sockaddr *)&cli, &iLen);
		if (sAccept == INVALID_SOCKET) {
			printf("accept() Failed: %d\n", WSAGetLastError());
			return -1;
		}

		printf("连接来自 IP:  %s  Port:  %d  \n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

		//第七步，创建线程接受浏览器请求
		DWORD ThreadID;
		CreateThread(NULL, 0, SimpleHTTPServer, (LPVOID)sAccept, 0, &ThreadID);
	}
	closesocket(sListen);      //关闭 socket  
	WSACleanup();       //输出发送成功字节数  
	return 0;
}