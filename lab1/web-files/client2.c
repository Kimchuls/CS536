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
#include <ctype.h>
#include <stdlib.h>
char gethtml[100][1024] = {0};
char getsrc[100][1024] = {0};
int html_lines;
int src_cnt;
struct node
{
	int sockfd;
	char *message;
	char *ip;
	int port;
};
void substring(int left, int right, char *string_o, char *output) // get substring [left, right)
{
	// char substr[1024] = {0};
	for (int i = 0; left + i < right; i++)
	{
		memcpy(output + i, string_o + left + i, 1);
	}
}
void *send_pthread(void *arg)
{
	struct node n = *(struct node *)arg;
	// int sock = *(int *)arg;
	int sock = n.sockfd;
	char *message = n.message;
	char *ip = n.ip;
	int port = n.port;
	char sendBuf[1024] = {0};
	char recvBuf[2048] = {0};
	sprintf(sendBuf, "GET /%s HTTP/2.0\r\nHost:%s:%d\r\n\r\n", message, ip, port);
	printf("%s\n", sendBuf);

	int index_msg = 0;
	while (message[index_msg] != '.')
		index_msg++;
	index_msg++;
	char file_type[10] = {0};
	substring(index_msg, strlen(message), message, file_type);

	if (send(sock, sendBuf, strlen(sendBuf), 0) < 0)
	{
		printf("send error\n");
	}
	// printf("Modified sentence received from server:\n");
	if (0 != strcmp("html", file_type))
	{

		char modifiedSentence[1024 * 40] = {0};
		int number = 1;
		long total_length=0;
		while (1)
		{

			ssize_t result = recv(sock, modifiedSentence, sizeof(modifiedSentence), 0);

			if (result < -1)
			{
				printf("0-recv error\n");
				// return NULL;
				break;
			}
			else if (result == -1)
			{
				// printf("recv disconnect\n");
				break;
			}
			total_length+=result;
			if (number % 100 == 1)
			{
				printf("Object-Frame: %s Frame_%d\n", message, number);
			}
			// printf("%s\n", modifiedSentence);
			number++;
		}
		// if (send(sock, sendBuf, strlen(sendBuf), 0) < 0)
		// {
		// 	printf("1-send error\n");
		// }
		printf("total_length:%ld\n",total_length);
	}
	else
	{
		char output[1024] = {0};
		int index = 0;
		int number = 1;

		char modifiedSentence[1024] = {0};
		while (1)
		{
			// printf("1:\n");
			ssize_t result = recv(sock, modifiedSentence, sizeof(modifiedSentence), 0);
			printf("length: %ld\n", result);
			if (result < -1)
			{
				printf("recv error\n");
				return NULL;
			}
			else if (result == -1)
			{
				printf("recv disconnect\n");
				break;
			}
			if (number % 100 == 1)
			{
				printf("Object-Frame: %s Frame_%d\n", message, number);
			}
			// printf("%s\n", modifiedSentence);
			sprintf(output + index, "%s", modifiedSentence);
			// strcpy(gethtml[html_lines++],modifiedSentence);
			index += strlen(modifiedSentence);

			memset(modifiedSentence, 0, sizeof(modifiedSentence));
			number++;
		}
		// printf("%s\n", output);
		// // for (int i = 0; i < html_lines; i++)
		// // {
		// // 	printf("%s\n", gethtml[i]);
		// // }
		memset(gethtml, 0, sizeof(gethtml));
		html_lines = 0;
		int j = 0;
		for (int i = 0; i < index; i++, j++)
		{
			if (output[i] == '\n')
			{
				html_lines++;
				i++;
				j = 0;
				while (isspace(output[i]))
					i++;
			}
			memcpy(gethtml[html_lines] + j, output + i, 1);
		}
		for (int i = 0; i < html_lines; i++)
		{
			printf("%s\n", gethtml[i]);
		}
	}

	printf("end\n");
}
void check()
{
	int index;
	// line 0,1 is the response head
	memset(getsrc, 0, sizeof(getsrc));
	src_cnt = 0;
	for (index = 2; index < html_lines; index++)
	{
		// line_index = 0;
		// printf("line:%d\n%s\n", index, gethtml[index]);
		if ((0 == strcmp(gethtml[index], "<!DOCTYPE html>") || 0 == strcmp(gethtml[index], "<html>") || 0 == strcmp(gethtml[index], "<body>"))) // reduce useless line
			continue;
		if (gethtml[index][0] == '<')
		{
			char label[20] = {0};
			char return_opt[1024] = {0};
			int label_index = 0;
			while (gethtml[index][1 + label_index] != '>' && gethtml[index][1 + label_index] != ' ')
			{
				memcpy(label + label_index, &(gethtml[index][1 + label_index]), 1);
				label_index++;
			}
			if (gethtml[index][1 + label_index] == '>') // reduce lines w/o source
			{
				// printf("label: %s\n", label);
				continue;
			}
			memset(return_opt, 0, sizeof(return_opt));
			substring(0, 3, label, return_opt);
			// printf("%s\n",return_opt);
			if (0 == strcmp(return_opt, "!--"))
			{
				// printf("note: %s\n", label);
				continue;
			}
			// printf("feature: %s\n", label);
			label_index += 2;
			while (1)
			{
				int length = 0;
				while (gethtml[index][label_index + length] != '=')
					length++;
				memset(return_opt, 0, sizeof(return_opt));
				substring(label_index, label_index + length, gethtml[index], return_opt);
				// printf("%s\n", return_opt);
				// break;
				if (0 != strcmp("src", return_opt))
				{
					while (gethtml[index][label_index + length] != ' ')
						length++;
					label_index += length + 1; // point to the next position of ' '
				}
				else
				{
					label_index += length + 2; // point to the next position of "
					break;
				}
			}
			// line_index = label_index;
			int length = 0;
			while (gethtml[index][label_index + length] != '"')
				length++;
			// memset(return_opt, 0, sizeof(return_opt));
			substring(label_index, label_index + length, gethtml[index], getsrc[src_cnt]);
			printf("%s\n", getsrc[src_cnt++]);
		}
		else
		{
			continue;
		}

		// break;
	}
}
void connect_server(char *ip_str, int port, char *message)
{
	int opt = 1;
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		// return -1;
		exit(EXIT_FAILURE);
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
	tv_out.tv_sec = 1;
	tv_out.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, ip_str, &serv_addr.sin_addr) <= 0)
	{
		printf(
			"\nInvalid address/ Address not supported \n");
		exit(EXIT_FAILURE);
	}

	if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
							 sizeof(serv_addr))) < 0)
	{
		printf("\nConnection Failed \n");
		exit(EXIT_FAILURE);
	}
	pthread_t thed;
	struct node nn;
	nn.sockfd = sock;
	nn.message = message;
	nn.ip = ip_str;
	nn.port = port;
	if (pthread_create(&thed, NULL, send_pthread, &nn) != 0)
	{
		printf("thread error:%s \n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	pthread_join(thed, NULL);
}
void *send_pthread2(void *arg)
{
	struct node n = *(struct node *)arg;
	// int sock = *(int *)arg;
	int sock = n.sockfd;
	char *message = n.message;
	char *ip = n.ip;
	int port = n.port;
	char sendBuf[1024] = {0};
	char recvBuf[2048] = {0};
	sprintf(sendBuf, "GET /%s HTTP/2.0\r\nHost:%s:%d\r\n\r\n", message, ip, port);
	printf("%s\n", sendBuf);

	int index_msg = 0;
	while (message[index_msg] != '.')
		index_msg++;
	index_msg++;
	char file_type[10] = {0};
	substring(index_msg, strlen(message), message, file_type);

	if (send(sock, sendBuf, strlen(sendBuf), 0) < 0)
	{
		printf("send error\n");
	}
	// printf("Modified sentence received from server:\n");
	if (0 != strcmp("html", file_type))
	{

		char modifiedSentence[1024 * 40] = {0};
		int number = 1;
		long total_length=0;
		while (1)
		{

			ssize_t result = recv(sock, modifiedSentence, sizeof(modifiedSentence), 0);

			if (result < -1)
			{
				printf("0-recv error\n");
				// return NULL;
				break;
			}
			else if (result == -1)
			{
				// printf("recv disconnect\n");
				break;
			}
			total_length+=result;
			if (number % 100 == 1)
			{
				printf("Object-Frame: %s Frame_%d\n", message, number);
			}
			// printf("%s\n", modifiedSentence);
			number++;
		}
		// if (send(sock, sendBuf, strlen(sendBuf), 0) < 0)
		// {
		// 	printf("1-send error\n");
		// }
		printf("total_length:%ld\n",total_length);
	}
	else
	{
		char output[1024] = {0};
		int index = 0;
		int number = 1;

		char modifiedSentence[1024] = {0};
		while (1)
		{
			// printf("1:\n");
			ssize_t result = recv(sock, modifiedSentence, sizeof(modifiedSentence), 0);
			printf("length: %ld\n", result);
			if (result < -1)
			{
				printf("recv error\n");
				return NULL;
			}
			else if (result == -1)
			{
				printf("recv disconnect\n");
				break;
			}
			if (number % 100 == 1)
			{
				printf("Object-Frame: %s Frame_%d\n", message, number);
			}
			// printf("%s\n", modifiedSentence);
			sprintf(output + index, "%s", modifiedSentence);
			// strcpy(gethtml[html_lines++],modifiedSentence);
			index += strlen(modifiedSentence);

			memset(modifiedSentence, 0, sizeof(modifiedSentence));
			number++;
		}
		// printf("%s\n", output);
		// // for (int i = 0; i < html_lines; i++)
		// // {
		// // 	printf("%s\n", gethtml[i]);
		// // }
		memset(gethtml, 0, sizeof(gethtml));
		html_lines = 0;
		int j = 0;
		for (int i = 0; i < index; i++, j++)
		{
			if (output[i] == '\n')
			{
				html_lines++;
				i++;
				j = 0;
				while (isspace(output[i]))
					i++;
			}
			memcpy(gethtml[html_lines] + j, output + i, 1);
		}
		for (int i = 0; i < html_lines; i++)
		{
			printf("%s\n", gethtml[i]);
		}
	}

	printf("end\n");
}
int main(int argc, char *argv[])
{
	char *ip_str = argv[1];
	int port = atoi(argv[2]);
	char *message = argv[3];
	// connect_server(ip_str, port, message);

	int opt = 1;
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		// return -1;
		exit(EXIT_FAILURE);
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
	tv_out.tv_sec = 1;
	tv_out.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, ip_str, &serv_addr.sin_addr) <= 0)
	{
		printf(
			"\nInvalid address/ Address not supported \n");
		exit(EXIT_FAILURE);
	}

	if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
							 sizeof(serv_addr))) < 0)
	{
		printf("\nConnection Failed \n");
		exit(EXIT_FAILURE);
	}
	pthread_t thed;
	struct node nn;
	nn.sockfd = sock;
	nn.message = message;
	nn.ip = ip_str;
	nn.port = port;
	if (pthread_create(&thed, NULL, send_pthread, &nn) != 0)
	{
		printf("thread error:%s \n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	pthread_join(thed, NULL);

	// end

	while (!(0 == strcmp(gethtml[2], "<!DOCTYPE html>") || 0 == strcmp(gethtml[2], "<html>") || 0 == strcmp(gethtml[2], "<body>"))) // check to confirm getting the whole responce string, or restart
	{
		printf("1\n");
		// connect_server(ip_str, port, message);
		// pthread_t thed;
		// struct node nn;
		// nn.sockfd = sock;
		// nn.message = message;
		// nn.ip = ip_str;
		// nn.port = port;
		if (pthread_create(&thed, NULL, send_pthread, &nn) != 0)
		{
			printf("thread error:%s \n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		pthread_join(thed, NULL);
	}
	// closing the connected socket
	check();
	for (int i = 0; i < src_cnt; i++)
	{
		// char nmsg[2048] = {0};
		// sprintf(nmsg, "/%s", getsrc[i]);
		// connect_server(ip_str, port, getsrc[i]);
		pthread_t thed1;
		struct node nn2;
		nn2.sockfd = sock;
		nn2.message = getsrc[i];
		nn2.ip = ip_str;
		nn2.port = port;
		if (pthread_create(&thed1, NULL, send_pthread, &nn2) != 0)
		{
			printf("thread error:%s \n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		pthread_join(thed1, NULL);
	}
	// sleep(5);
	// close(client_fd);
	return 0;
}
