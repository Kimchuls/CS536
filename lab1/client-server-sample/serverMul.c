// Server side C/C++ program to demonstrate Socket
// programming
// reference: https://www.cnblogs.com/charliecza/p/16938789.html
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
// #define PORT 12000
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct node
{
	int id;
	int new_socket;
	char ip[100];
	int port;
};
struct node n[100];
int flag[100] = {0};
int num = 0;
void *thread_recv(void *arg)
{
	char sentence[1024] = {0};
	struct node n = *(struct node *)arg;
	// int id = *(int *)arg;

	int new_socket = n.new_socket;
	// char *ip = n.ip;
	int port = n.port;
	// int valread = recv(new_socket, sentence, sizeof(sentence), 0);
	// printf("%s\n", sentence);
	// send(new_socket, sentence, strlen(sentence), 0);
	while (1)
	{
		int valread = recv(new_socket, sentence, sizeof(sentence), 0);
		if (valread <= 0)
		{
			printf("close-client: %s, %d\n", n.ip, port);
			return NULL;
		}
		else if (valread > 0)
		{
			printf("%s\n", sentence);
		}
		if (send(new_socket, sentence, strlen(sentence), 0) <= 0)
		{
			printf("close-client: %s, %d\n", n.ip, port);
			return NULL;
		}
	}
}
int main(int argc, char const *argv[])
{
	int port = atoi(argv[1]);
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	pthread_t recv_thread, send_thread;

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 12000
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	// Forcefully attaching socket to the port 12000
	if (bind(server_fd, (struct sockaddr *)&address,
			 sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("The server is ready to receive\n");

	while (1)
	{
		struct sockaddr_in skaddr;
		socklen_t skaddrlen = sizeof(skaddr);
		memset(&skaddr, 0, skaddrlen);
		if ((new_socket = accept(server_fd, (struct sockaddr *)&skaddr, (socklen_t *)&skaddrlen)) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		pthread_mutex_lock(&mutex);
		num++;
		{
			// int i;
			// for (i = 0; i < 100; i++)
			// {
			// 	if (flag[i] == 0)
			// 	{
			// 		flag[i] = 1;
			// 		break;
			// 	}
			// }
			// n[i].id = i;
			// n[i].new_socket = new_socket;
			// // n[i].ip = inet_ntoa(skaddr.sin_addr);
			// strcpy(n[i].ip, inet_ntoa(skaddr.sin_addr));
			// n[i].port = port;
			struct node n;
			n.new_socket = new_socket;
			strcpy(n.ip, inet_ntoa(skaddr.sin_addr));
			n.port = port;
			pthread_mutex_unlock(&mutex);
			printf("message-from-client: %s, %d \n", inet_ntoa(skaddr.sin_addr), port);
			if (pthread_create(&recv_thread, NULL, thread_recv, &n) < 0)
			{
				printf("create thread error:%s \n", strerror(errno));
				break;
			}
			pthread_detach(recv_thread);
		}
	}
	// close(new_socket);
	// shutdown(server_fd, SHUT_RDWR);
	// close(server_fd);
	return 0;
}
