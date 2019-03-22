#pragma comment(lib,"ws2_32.lib") 
#include <Winsock2.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#define DATA_BUFFER 1024      //默认缓冲区大小 

int main(int argc, char * argv[]) {

	WSADATA wsaData;
	SOCKET sClient;
	int iPort = 8080;
	int iSend,iRecv;        //从服务器端发送和接收的数据长度  
	char send_buf[DATA_BUFFER];     //发送缓冲区  
	char recv_buf[DATA_BUFFER];     //接收缓冲区 
	struct sockaddr_in ser;      //服务器端地址    
	
	memset(send_buf, 0, sizeof(send_buf));     //初始化缓冲区  
	memset(recv_buf, 0, sizeof(recv_buf));

	//加载协议栈
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed to load Winsock.\n");  //Winsock 初始化错误   
		return -1;
	}

	//创建服务器地址
	ser.sin_family = AF_INET;      //初始化服务器地址信息  
	ser.sin_port = htons(iPort);      //端口转换为网络字节序  
	ser.sin_addr.s_addr = inet_addr("127.0.0.1");   //IP 地址转换为网络字节序    

	sClient = socket(AF_INET, SOCK_STREAM, 0);   //创建客户端流式套接字，运输层采用TCP协议
	if (sClient == INVALID_SOCKET) {
		printf("socket() Failed: %d\n", WSAGetLastError());
		return -1;
	} 
	
	//请求与服务器端建立 TCP 连接  
	if (connect(sClient, (struct sockaddr *)&ser, sizeof(ser)) == INVALID_SOCKET) {
		printf("connect() Failed: %d\n", WSAGetLastError());
		return -1;
	}
	else {//连接成功
		//通过套接字发送数据
		printf("input data to send:\n");
		gets_s(send_buf);
		iSend = send(sClient, send_buf, sizeof(send_buf), 0);  //通过套接字发送数据给服务器
		if (iSend == SOCKET_ERROR)   //发送不成功
		{
			printf("send() Failed:%d\n", WSAGetLastError());
			return -1;
		}
		else {
			printf("send success\n");  //发送成功
		}
			
		printf("recv data from server: \n");
		while (1)
		{
			memset(recv_buf, 0, sizeof(recv_buf));  //缓存清0
			iRecv = recv(sClient, recv_buf, sizeof(recv_buf), 0);
			if (strcmp(recv_buf, "") == 0)    // 接收缓存为空则表示接受完毕，退出
				break;
			if (iRecv == SOCKET_ERROR)   //接收错误
			{
				printf("recv() Failed:%d\n", WSAGetLastError());
				return -1;
			}
			else
				printf("recv() data from server: %s\n", recv_buf);    // 打印接收缓存

		}
	}
	closesocket(sClient);       //关闭 socket  
	WSACleanup();
	return 0;
}