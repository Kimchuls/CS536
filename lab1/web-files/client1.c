// Client side C/C++ program to demonstrate Socket
// programming
// reference: https://blog.csdn.net/weixin_30410999/article/details/99426922
// reference: https://sites.google.com/site/yunluliussite/linux-shell/nonblocktimeout
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
char *message;
char modifiedSentence[1024] = {0};
void *send_pthread(void *arg)
{
	int sock = *(int *)arg;
	char sendBuf[1024] = {0};
	char recvBuf[2048] = {0};
	sprintf(sendBuf, "GET /picture.html HTTP/1.1\r\n");
	strcat(sendBuf, "Host:10.145.21.35:12000\r\n\r\n");
	if (send(sock, sendBuf, strlen(sendBuf), 0) < 0)
	{
		printf("send error\n");
	}
	printf("Modified sentence received from server:\n");
	while (1)
	{
		printf("1:\n");
		ssize_t result = recv(sock, modifiedSentence, sizeof(modifiedSentence), 0);
		if (result < 0)
		{
			printf("recv error\n");
			return NULL;
		}
		else if (result == 0)
		{
			// printf("recv disconnect\n");
			break;
		}
		printf("%s\n", modifiedSentence);
	}
}
void *recv_pthread(void *arg)
{
	int sock = *(int *)arg;
}

int main(int argc, char *argv[])
{
	char *ip_str = argv[1];
	int port = atoi(argv[2]);
	message = argv[3];
	int opt = 1;
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip_str);

	struct timeval tv_out;
	tv_out.tv_sec = 5;
	tv_out.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, ip_str, &serv_addr.sin_addr) <= 0)
	{
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
							 sizeof(serv_addr))) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	pthread_t thed;
	if (pthread_create(&thed, NULL, send_pthread, &sock) != 0)
	{
		printf("thread error:%s \n", strerror(errno));
		return -1;
	}
	pthread_join(thed, NULL);
	// closing the connected socket
	sleep(5);
	close(client_fd);
	return 0;
}
