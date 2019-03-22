#pragma comment(lib,"ws2_32.lib") 
#include <Winsock2.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#define DATA_BUFFER 1024      //Ĭ�ϻ�������С 

int main(int argc, char * argv[]) {

	WSADATA wsaData;
	SOCKET sClient;
	int iPort = 8080;
	int iSend,iRecv;        //�ӷ������˷��ͺͽ��յ����ݳ���  
	char send_buf[DATA_BUFFER];     //���ͻ�����  
	char recv_buf[DATA_BUFFER];     //���ջ����� 
	struct sockaddr_in ser;      //�������˵�ַ    
	
	memset(send_buf, 0, sizeof(send_buf));     //��ʼ��������  
	memset(recv_buf, 0, sizeof(recv_buf));

	//����Э��ջ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed to load Winsock.\n");  //Winsock ��ʼ������   
		return -1;
	}

	//������������ַ
	ser.sin_family = AF_INET;      //��ʼ����������ַ��Ϣ  
	ser.sin_port = htons(iPort);      //�˿�ת��Ϊ�����ֽ���  
	ser.sin_addr.s_addr = inet_addr("127.0.0.1");   //IP ��ַת��Ϊ�����ֽ���    

	sClient = socket(AF_INET, SOCK_STREAM, 0);   //�����ͻ�����ʽ�׽��֣���������TCPЭ��
	if (sClient == INVALID_SOCKET) {
		printf("socket() Failed: %d\n", WSAGetLastError());
		return -1;
	} 
	
	//������������˽��� TCP ����  
	if (connect(sClient, (struct sockaddr *)&ser, sizeof(ser)) == INVALID_SOCKET) {
		printf("connect() Failed: %d\n", WSAGetLastError());
		return -1;
	}
	else {//���ӳɹ�
		//ͨ���׽��ַ�������
		printf("input data to send:\n");
		gets_s(send_buf);
		iSend = send(sClient, send_buf, sizeof(send_buf), 0);  //ͨ���׽��ַ������ݸ�������
		if (iSend == SOCKET_ERROR)   //���Ͳ��ɹ�
		{
			printf("send() Failed:%d\n", WSAGetLastError());
			return -1;
		}
		else {
			printf("send success\n");  //���ͳɹ�
		}
			
		printf("recv data from server: \n");
		while (1)
		{
			memset(recv_buf, 0, sizeof(recv_buf));  //������0
			iRecv = recv(sClient, recv_buf, sizeof(recv_buf), 0);
			if (strcmp(recv_buf, "") == 0)    // ���ջ���Ϊ�����ʾ������ϣ��˳�
				break;
			if (iRecv == SOCKET_ERROR)   //���մ���
			{
				printf("recv() Failed:%d\n", WSAGetLastError());
				return -1;
			}
			else
				printf("recv() data from server: %s\n", recv_buf);    // ��ӡ���ջ���

		}
	}
	closesocket(sClient);       //�ر� socket  
	WSACleanup();
	return 0;
}