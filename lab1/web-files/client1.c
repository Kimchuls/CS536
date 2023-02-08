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
#include <time.h>
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
	int sock = n.sockfd;
	char *message = n.message;
	char *ip = n.ip;
	int port = n.port;
	char sendBuf[1024] = {0};
	char recvBuf[2048] = {0};
	sprintf(sendBuf, "GET /%s HTTP/1.1\r\nHost:%s:%d\r\n\r\n", message, ip, port);
	// printf("%s\n", sendBuf);

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
	if (0 != strcmp("html", file_type))
	{

		// char modifiedSentence[1024 * 40] = {0};
		// int number = 1;
		// long total_length = 0;
		// while (1)
		// {

		// 	ssize_t result = recv(sock, modifiedSentence, sizeof(modifiedSentence), 0);

		// 	if (result < -1)
		// 	{
		// 		printf("0-recv error\n");
		// 		break;
		// 	}
		// 	else if (result == -1)
		// 	{
		// 		break;
		// 	}
		// 	total_length += result;
		// 	if (number % 100 == 1)
		// 	{
		// 		printf("Object-Frame: %s Frame_%d\n", message, number);
		// 	}
		// 	number++;
		// }
		// printf("total_length:%ld\n", total_length);
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
			// printf("length: %ld\n", result);
			if (result < -1)
			{
				printf("recv error\n");
				return NULL;
			}
			else if (result == -1)
			{
				// printf("recv disconnect\n");
				break;
			}
			// if (number % 100 == 1)
			// {
			// printf("Object-Frame: %s Frame_%d\n", message, number);
			// }
			sprintf(output + index, "%s", modifiedSentence);
			index += strlen(modifiedSentence);

			memset(modifiedSentence, 0, sizeof(modifiedSentence));
			number++;
		}
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
		// for (int i = 0; i < html_lines; i++)
		// {
		// 	printf("%s\n", gethtml[i]);
		// }
	}

	// printf("end\n");
}
void *send_pthread2(void *arg)
{
	struct node n = *(struct node *)arg;
	int sock = n.sockfd;
	// char *message = n.message;
	char *ip = n.ip;
	int port = n.port;
	char sendBuf[2048] = {0};
	char recvBuf[2048] = {0};
	for (int i = 0; i < src_cnt; i++)
	{
		sprintf(sendBuf, "GET /%s HTTP/1.1\r\nHost:%s:%d\r\n\r\n", getsrc[i], ip, port);
		// printf("%s\n", sendBuf);
		if (send(sock, sendBuf, strlen(sendBuf), 0) < 0)
		{
			printf("send error\n");
		}
		char modifiedSentence[1024 * 40] = {0};
		int number = 1;
		long total_length = 0;
		while (1)
		{

			ssize_t result = recv(sock, modifiedSentence, sizeof(modifiedSentence), 0);
			// printf("%s\n",modifiedSentence);
			// char substrings[128];
			// substring(0,17,modifiedSentence,substrings);
			// if()
			if (result < -1)
			{
				printf("recv error\n");
				break;
			}
			else if (result == -1)
			{
				break;
			}
			total_length += result;
			if (number % 100 == 1)
			{
				printf("Object-Frame: %s Frame_%d\n", getsrc[i], number);
			}
			number++;
		}
		// printf("total_length:%ld\n", total_length);

		// printf("end\n");
	}
}
void check()
{
	int index;
	// line 0,1 is the response head
	memset(getsrc, 0, sizeof(getsrc));
	src_cnt = 0;
	for (index = 2; index < html_lines; index++)
	{
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
				continue;
			}
			memset(return_opt, 0, sizeof(return_opt));
			substring(0, 3, label, return_opt);
			if (0 == strcmp(return_opt, "!--"))
			{
				continue;
			}
			label_index += 2;
			while (1)
			{
				int length = 0;
				while (gethtml[index][label_index + length] != '=')
					length++;
				memset(return_opt, 0, sizeof(return_opt));
				substring(label_index, label_index + length, gethtml[index], return_opt);
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
			int length = 0;
			while (gethtml[index][label_index + length] != '"')
				length++;
			substring(label_index, label_index + length, gethtml[index], getsrc[src_cnt]);
			// printf("%s\n", getsrc[src_cnt++]);
			src_cnt++;
		}
		else
		{
			continue;
		}
	}
}

int main(int argc, char *argv[])
{
	clock_t start, finish;
	start = clock();
	char *url = argv[1];
	int length = strlen(url);
	int index = 0;
	while (index < length && url[index] != '/')
		index++;
	if (index == length)
	{
		printf("\n URL error \n");
		exit(EXIT_FAILURE);
	}
	index += 2;
	char ip_str[20] = {0};
	int ip_index = 0;
	while (index < length && url[index] != ':')
	{
		ip_str[ip_index++] = url[index++];
	}
	ip_str[ip_index] = '\0';
	index++;
	int port = 0;
	while (index < length && url[index] != '/')
	{
		port = port * 10 + (int)(url[index++] - '0');
	}
	index++;
	char message[1024] = {0};
	int message_index = 0;
	while (index < length)
	{
		message[message_index++] = url[index++];
	}
	message[message_index] = '\0';

	int opt = 1;
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
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
		// printf("1\n");
		if (pthread_create(&thed, NULL, send_pthread, &nn) != 0)
		{
			printf("thread error:%s \n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		pthread_join(thed, NULL);
	}
	// closing the connected socket
	check();

	pthread_t thed1;
	if (pthread_create(&thed1, NULL, send_pthread2, &nn) != 0)
	{
		printf("thread error:%s \n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	pthread_join(thed1, NULL);

	// sleep(5);
	close(client_fd);
	// finish = clock_gettime(CLOCK_PROCESS_CPUTIME_ID);
	// printf("time=%f\n", (double)(finish - start) / CLOCKS_PER_SEC);
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	printf("Seconds = %ld \t Nanoseconds = %ld\n", now.tv_sec, now.tv_nsec);
	return 0;
}
