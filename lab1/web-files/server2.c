// Server side C/C++ program to demonstrate Socket
// programming
// reference: https://blog.csdn.net/u011003120/article/details/87723011
// reference: https://blog.csdn.net/panco_/article/details/90081153
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
#define METHOD_LENGTH 32
#define URI_LENGTH 1024
#define REQUEST_LENGTH 102400
#define LINE_LENGTH 256
#define ll long long
#define MIN(a, b) (a) <= (b) ? (a) : (b)
#define MAX(a, b) (a) >= (b) ? (a) : (b)
char loadfile[100][1024] = {0};
int loadfile_length = 0;
int loadfile_flag[100] = {0};
struct node
{
    int new_socket;
    char ip[100];
    int port;
};
void badRequest400(int sockfd)
{
    char buff[] =
        "HTTP/2.0 400 Bad Request\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "400 Bad Request: URL String Syntax Error.\r\n";
    send(sockfd, buff, strlen(buff), 0);
    printf("HTTP/2.0 400 Bad Request\n");
}
void HTTPVersion505(int sockfd)
{
    char buff[] =
        "HTTP/2.0 505 HTTP Version Not Supported\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "505 HTTP Version Not Supported: Client sends GET with a different HTTP version.\r\n";
    send(sockfd, buff, strlen(buff), 0);
    printf("HTTP/2.0 505 HTTP Version Not Supported\n");
}
void NotFound404(int sockfd)
{
    char buff[] =
        "HTTP/2.0 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "404 Not Found: HTML file does not exist.\r\n";
    send(sockfd, buff, strlen(buff), 0);
    printf("HTTP/2.0 404 Not Found\n");
}

void sendText(int sockfd, char *name)
{
    // printf("%s,%d,%d,%d\n", name, 0 == strcmp(name, "text"), 0 == strcmp(name, "picture"), 0 == strcmp(name, "bigpicture"));
    if (!(0 == strcmp(name, "text") || 0 == strcmp(name, "picture") || 0 == strcmp(name, "bigpicture") || 0 == strcmp(name, "video")))
    {
        NotFound404(sockfd);
        return;
    }
    // sleep(2);
    char path[100] = {0};
    strcat(path, "./www/");
    strcat(path, name);
    strcat(path, ".html");
    FILE *f = fopen(path, "r");
    char header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n";
    send(sockfd, header, sizeof(header), 0);
    // printf("HTTP/1.1 200 OK\n");
    sleep(0.005);
    char line[LINE_LENGTH] = {0};
    while (fgets(line, sizeof(line), f) != NULL)
    {
        send(sockfd, line, strlen(line), 0);
        // printf("%s\n",line);
        memset(line, 0, sizeof(line));
    }
    // char end[] = "\r\n";
    // send(sockfd, end, sizeof(end), 0);
    printf("HTTP/1.1 200 OK\n");
    // sleep(5);
    fclose(f);
}
void substring(int left, int right, char *string_o, char *output) // get substring [left, right)
{
    // char substr[1024] = {0};
    for (int i = 0; left + i < right; i++)
    {
        memcpy(output + i, string_o + left + i, 1);
    }
}
void *thread_recv(void *arg)
{
    struct node n = *(struct node *)arg;
    int sockfd = n.new_socket;
    // char *ip = n.ip;
    int port = n.port;
    printf("message-from-client: %s, %d \n", n.ip, port);

    /* parse the request information*/
    char method[METHOD_LENGTH] = {0};
    char uri[URI_LENGTH] = {0};
    char http_version[URI_LENGTH] = {0};
    char request[REQUEST_LENGTH] = {0};
    memset(loadfile, 0, sizeof(loadfile));
    loadfile_length = 0;
    memset(loadfile_flag, 0, sizeof(loadfile_flag));

    while (1)
    {
        ssize_t length = recv(sockfd, request, sizeof(request), 0);
        if (length < 0)
        {
            printf("close-client: %s, %d\n", n.ip, port);
            return NULL;
        }
        else if (length == 0)
        {

            printf("close-client: %s, %d\n", n.ip, port);
            break;
        }
        if (send(sockfd, "a", 1, 0) < 0)
        {
            printf("close-client: %s, %d\n", n.ip, port);
            return NULL;
        }

        // printf("REQUEST\n%s\n%ld\n", request,length);
        int request_front = 0;
        int method_front = 0;
        while (request_front < length && !isspace(request[request_front]))
            method[method_front++] = request[request_front++];
        method[method_front] = '\0';
        while (request_front < length && isspace(request[request_front]))
            request_front++;

        int uri_front = 0;
        while (request_front < length && !isspace(request[request_front]))
            uri[uri_front++] = request[request_front++];
        uri[uri_front] = '\0';
        while (request_front < length && isspace(request[request_front]))
            request_front++;

        int http_front = -1;
        while (request_front < length && !(request[request_front] == '\n' || request[request_front] == 0x0d))
            memcpy(&http_version[++http_front], &request[request_front++], 1);
        http_version[++http_front] = '\0';

        // printf("URI: %s\n", uri);
        if (0 == strcmp("/end", uri))
        {
            char loadfile_name[100][100];
            for (int i = 0; i < loadfile_length; i++)
            {
                printf("%s\n", loadfile[i]);

                sprintf(loadfile_name[i], "./www/%s", loadfile[i]);
            }
            FILE *fs[100];
            ll lengths[100];

            for (int xx = 0; xx < loadfile_length; xx++)
            {
                fs[xx] = fopen(loadfile_name[xx], "rb");
                if (fs[xx] == NULL)
                    return NULL;
                fseek(fs[xx], 0, SEEK_END);
                lengths[xx] = ftell(fs[xx]);
                fseek(fs[xx], 0, SEEK_SET);
                if (lengths[xx] <= 0)
                {
                    fclose(fs[xx]);
                    return NULL;
                }
            }
            int X = 40 * 1024;
            int indexes[100] = {0};
            int ii = 1;
            while (1)
            {
                int flag = 0;
                for (int xx = 0; xx < loadfile_length; xx++)
                {
                    char picBuff[40 * 1024] = {0};
                    int size = MIN(X, lengths[xx] - indexes[xx]);
                    if (size == 0)
                        continue;
                    fread(picBuff, 1, size, fs[xx]);
                    send(sockfd, loadfile[xx], strlen(loadfile[xx]), 0);
                    // send(sockfd, "\n", strlen("\n"), 0);
                    flag = 1;
                    indexes[xx] += size;
                    if (ii % 100 == 1)
                        printf("%s %d\n", loadfile[xx], ii);
                }
                ii++;
                if (flag == 0)
                    break;
            }
            for (int xx = 0; xx < loadfile_length; xx++)
            {
                fclose(fs[xx]);
                // fs[xx] = fopen(loadfile_name[xx], "rb");
            }
            break;
        }

        /*check GET request HTTP 505 HTTP Version Not Supported*/
        if (strcmp(http_version, "HTTP/2.0") != 0)
        {
            printf("0-message-to-client: %s, %d \n", n.ip, port);
            HTTPVersion505(sockfd);
            // goto end;
            return NULL;
        }

        request_front = 0;
        char firstline[REQUEST_LENGTH] = {0};
        int firstline_front = 0;
        while (request[request_front] != '\n')
            firstline[firstline_front++] = request[request_front++];
        firstline[firstline_front] = '\0';

        /*check GET request URI 400 Bad Request*/
        char name[REQUEST_LENGTH] = {0};
        char *name_front = name;
        int uri_pt = 0;
        int file_type;
        if (strlen(uri) == 0 || uri[uri_pt] != '/')
        {
            printf("1-message-to-client: %s, %d \n", n.ip, port);
            badRequest400(sockfd);
            return NULL;
            // goto end;
        }
        else
        {
            uri_pt++;
            if (strlen(uri) > 5 && uri[uri_pt] == 'w' && uri[uri_pt + 1] == 'w' &&
                uri[uri_pt + 2] == 'w' && uri[uri_pt + 3] == '/')
            {
                uri_pt += 4;
            }
            while (uri_pt < strlen(uri) && uri[uri_pt] != '.')
            {
                *name_front++ = uri[uri_pt++];
            }
            *name_front = '\0';
            if (uri_pt == strlen(uri))
            {
                printf("2-message-to-client: %s, %d \n", n.ip, port);
                badRequest400(sockfd);
                return NULL;
                // goto end;
            }
            uri_pt++;
            if (strlen(uri) == 3 + uri_pt && uri[uri_pt] == 'm' && uri[uri_pt + 1] == 'p' &&
                uri[uri_pt + 2] == '4')
            {
                file_type = 2;
            }
            else if (strlen(uri) == 4 + uri_pt && uri[uri_pt] == 'h' && uri[uri_pt + 1] == 't' &&
                     uri[uri_pt + 2] == 'm' && uri[uri_pt + 3] == 'l')
            {
                file_type = 0;
            }
            else if (strlen(uri) == 4 + uri_pt && uri[uri_pt] == 'j' && uri[uri_pt + 1] == 'p' &&
                     uri[uri_pt + 2] == 'e' && uri[uri_pt + 3] == 'g')
            {
                file_type = 1;
            }
            else
            {
                printf("3-message-to-client: %s, %d \n", n.ip, port);
                badRequest400(sockfd);
                return NULL;
                // goto end;
            }
        }
        /*check GET request HTML file 200 OK/ 404 Not Found*/
        // printf("name=%s, file_type=%d\n", name, file_type);
        printf("4-message-to-client: %s, %d \n", n.ip, port);
        if (file_type == 0)
        {
            sendText(sockfd, name);
        }
        else if (file_type == 1 && 0 == strcmp("purdue", name))
        {
            printf("SEND PURDUE\n");
            // sendPicture(sockfd, name);
            for (int xx = 0; xx < 100; xx++)
            {
                if (loadfile_flag[xx] == 0)
                {
                    strcpy((loadfile[xx]), "purdue.jpeg");
                    loadfile_length++;
                    loadfile_flag[xx] = 1;
                    break;
                }
            }
        }
        else if (file_type == 1 && 0 == strcmp(name, "bigpicture"))
        {
            printf("SEND BIGPICTURE\n");
            // sendBigPicture(sockfd, name);
            for (int xx = 0; xx < 100; xx++)
            {
                if (loadfile_flag[xx] == 0)
                {
                    strcpy((loadfile[xx]), "bigpicture.jpeg");
                    loadfile_length++;
                    loadfile_flag[xx] = 1;
                    break;
                }
            }
        }
        else if (file_type == 2 && 0 == strcmp(name, "video"))
        {
            printf("SEND VIDEO\n");
            // sendVideo(sockfd, name);
            for (int xx = 0; xx < 100; xx++)
            {
                if (loadfile_flag[xx] == 0)
                {
                    strcpy(loadfile[xx], "video.mp4");
                    loadfile_length++;
                    loadfile_flag[xx] = 1;
                    break;
                }
            }
        }
        else
        {
            NotFound404(sockfd);
        }
    }

    // end:
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
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
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
        struct node n;
        n.new_socket = new_socket;
        // n.ip = inet_ntoa(skaddr.sin_addr);

        strcpy(n.ip, inet_ntoa(skaddr.sin_addr));
        n.port = port;
        if (pthread_create(&recv_thread, NULL, thread_recv, &n) != 0)
        {
            perror("thread fail");
            exit(EXIT_FAILURE);
        }
    }
    close(new_socket);
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}
