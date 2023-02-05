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
struct node
{
    int new_socket;
    char *ip;
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
// int compare(char *a, char *b)
// {
//     // int length = MAX(strlen(a), strlen(b));
//     printf("length: %ld %ld\n", strlen(a), strlen(b));
//     if (strlen(a) != strlen(b))
//         return -1;
//     int length = strlen(a);
//     int i;
//     for (i = 0; i < length; i++)
//     {
//         if (a[i] != b[i])
//             return -1;
//     }
//     return 0;
// }
void sendText(int sockfd, char *name)
{
    // printf("%s,%d,%d,%d\n", name, 0 == strcmp(name, "text"), 0 == strcmp(name, "picture"), 0 == strcmp(name, "bigpicture"));
    if (!(0 == strcmp(name, "text") || 0 == strcmp(name, "picture") || 0 == strcmp(name, "bigpicture") || 0 == strcmp(name, "video")))
    {
        NotFound404(sockfd);
        return;
    }
    name = strcat(name, ".html");
    FILE *f = fopen(name, "r");
    char header[] =
        "HTTP/2.0 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n";
    send(sockfd, header, sizeof(header), 0);
    printf("HTTP/2.0 200 OK\n");
    char line[LINE_LENGTH] = {0};
    while (fgets(line, sizeof(line), f) != NULL)
    {
        send(sockfd, line, strlen(line), 0);
        memset(line, 0, sizeof(line));
    }
    char end[] = "\r\n";
    send(sockfd, end, sizeof(end), 0);
    fclose(f);
}
void sendPicture(int sockfd, char *name)
{
    // name = sprintf("./",name,".jpeg");
    FILE *f = fopen("./purdue.jpeg", "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    if (length <= 0)
    {
        fclose(f);
        return;
    }
    fseek(f, 0, SEEK_SET);
    char picBuff[38 * 1024] = {0};
    length = fread(picBuff, 1, length, f);
    char *ret_buff = (char *)malloc(length + 1024);
    int tlen = sprintf(ret_buff, "HTTP/2.0 200 OK\r\n"
                                 "Content-Type: image/jpeg\r\n"
                                 "Content-Length: %d\r\n"
                                 "Connection: close\r\n"
                                 "Accept-Ranges:bytes\r\n"
                                 "\r\n",
                       length);

    memcpy(ret_buff + tlen, picBuff, length);
    tlen += length;
    send(sockfd, ret_buff, tlen, 0);
    printf("HTTP/2.0 200 OK\n");
    // 2647*3529

    send(sockfd, picBuff, sizeof(picBuff), 0);
    fclose(f);
}
void sendBigPicture(int sockfd, char *name)
{
    FILE *f = fopen("./bigpicture.jpeg", "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    if (length <= 0)
    {
        fclose(f);
        return;
    }
    fseek(f, 0, SEEK_SET);
    // char picBuff[5119 * 1024] = {0};
    // length = fread(picBuff, 1, length, f);


    char *ret_buff = (char *)malloc(length + 1024);
    int tlen = sprintf(ret_buff, "HTTP/2.0 200 OK\r\n"
                                 "Content-Type: image/jpeg\r\n"
                                 //  "Content-Length: %d\r\n"
                                 // "Transfer-Encoding: Chunked"
                                 //  "Connection: keep-alive\r\n"
                                 //  "Keep-Alive: timeout=10\r\n"
                                 "Accept-Ranges:bytes\r\n"
                                 "\r\n");
    send(sockfd, ret_buff, tlen, 0);

    int i = 0;
    int X = 10240;
    char picBuff[10240]={0};
    for (i = 0; i < length; i += X)
    {
        int size = MIN(X, length - i);
        size=fread(picBuff,1,size,f);
        send(sockfd, picBuff , size, 0);
    }
    fclose(f);
}

void sendVideo(int sockfd, char *name)
{
    FILE *f = fopen("./video.mp4", "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    ll length = ftell(f);
    fseek(f, 0, SEEK_SET);
    // printf("%lld\n", length);
    // return;

    if (length <= 0)
    {
        fclose(f);
        return;
    }
    // char picBuff[5119 * 1024] = {0};
    // length = fread(picBuff, 1, length, f);
    char *ret_buff = (char *)malloc(length + 1024);
    int tlen = sprintf(ret_buff, "HTTP/2.0 200 OK\r\n"
                                 "Content-Type: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8\r\n"
                                 //  "Content-Length: %d\r\n"
                                 // "Transfer-Encoding: Chunked"
                                 //  "Connection: keep-alive\r\n"
                                 //  "Keep-Alive: timeout=10\r\n"
                                 "Accept-Ranges:bytes\r\n"
                                 "\r\n");
    send(sockfd, ret_buff, tlen, 0);

    // memcpy(ret_buff + tlen, picBuff, length);
    // tlen += length;
    // send(sockfd, ret_buff, tlen, 0);
    // printf("HTTP/2.0 200 OK\n");
    // send(sockfd, picBuff, 10240, 0);
    // send(sockfd, picBuff+10240, sizeof(picBuff)-10240, 0);
    int i = 0;
    int X = 10240;
    char picBuff[10240]={0};
    for (i = 0; i < length; i += X)
    {
        int size = MIN(X, length - i);
        size=fread(picBuff,1,size,f);
        send(sockfd, picBuff , size, 0);
    }
    fclose(f);
}
void *thread_recv(void *arg)
{
    struct node n = *(struct node *)arg;
    int sockfd = n.new_socket;
    char *ip = n.ip;
    int port = n.port;
    printf("message-from-client: %s, %d \n", ip, port);

    /* parse the request information*/
    char method[METHOD_LENGTH] = {0};
    char uri[URI_LENGTH] = {0};
    char http_version[URI_LENGTH] = {0};
    char request[REQUEST_LENGTH] = {0};
    ssize_t length = recv(sockfd, request, sizeof(request), 0);
    printf("REQUEST\n%s\n", request);

    if (strlen(request) == 0)
    {
        printf("-1-message-to-client: %s, %d \n", ip, port);
        badRequest400(sockfd);
        return NULL;
    }
    int request_front = 0;
    int method_front = 0;
    while (!isspace(request[request_front]))
        method[method_front++] = request[request_front++];
    method[method_front] = '\0';
    // printf("method %s\n", method);

    while (isspace(request[request_front]))
        request_front++;

    int uri_front = 0;
    while (!isspace(request[request_front]))
        uri[uri_front++] = request[request_front++];
    uri[uri_front] = '\0';
    // printf("uri %s\n", uri);

    while (isspace(request[request_front]))
        request_front++;

    int http_front = -1;
    while (!(request[request_front] == '\n' || request[request_front] == 0x0d))
        // http_version[http_front++] = request[request_front++];
        memcpy(&http_version[++http_front], &request[request_front++], 1);
    http_version[++http_front] = '\0';
    // char ver[9] = "HTTP/2.0";
    // printf("%ld, %ld, %d, '%s'\n",strlen(ver), strlen(http_version),http_front, http_version);
    // for (int x = 0; x <= strlen(http_version); x++)
    //     printf("%d ", http_version[x]);

    // printf(".\n");
    /*check GET request HTTP 505 HTTP Version Not Supported*/
    // printf("%d\n", strcmp(http_version, "HTTP/2.0"));
    if (strcmp(http_version, "HTTP/2.0") != 0)
    {
        printf("0-message-to-client: %s, %d \n", ip, port);
        HTTPVersion505(sockfd);
        // return NULL;
        goto end;
    }

    request_front = 0;
    char firstline[REQUEST_LENGTH] = {0};
    int firstline_front = 0;
    while (request[request_front] != '\n')
        firstline[firstline_front++] = request[request_front++];
    firstline[firstline_front] = '\0';
    // printf("%s\n", firstline);

    /*check GET request URI 400 Bad Request*/
    char name[REQUEST_LENGTH] = {0};
    char *name_front = name;
    int uri_pt = 0;
    int file_type;
    if (strlen(uri) == 0 || uri[uri_pt] != '/')
    {
        printf("1-message-to-client: %s, %d \n", ip, port);
        badRequest400(sockfd);
        // return NULL;
        goto end;
    }
    else
    {
        // printf("uri %s\n", uri);
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
            printf("2-message-to-client: %s, %d \n", ip, port);
            badRequest400(sockfd);
            // return NULL;
            goto end;
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
            printf("3-message-to-client: %s, %d \n", ip, port);
            badRequest400(sockfd);
            // return NULL;
            goto end;
        }
    }
    /*check GET request HTML file 200 OK/ 404 Not Found*/
    // printf("name=%s, file_type=%d\n", name, (file_type == 0));
    printf("4-message-to-client: %s, %d \n", ip, port);
    if (file_type == 0)
    {
        sendText(sockfd, name);
    }
    else if (file_type == 1 && 0 == strcmp("purdue", name))
    {
        sendPicture(sockfd, name);
    }
    else if (file_type == 1 && 0 == strcmp(name, "bigpicture"))
    {
        sendBigPicture(sockfd, name);
    }
    else if (file_type == 2 && 0 == strcmp(name, "video"))
    {
        // printf("SEND VIDEO\n");
        sendVideo(sockfd, name);
    }
    else
    {
        NotFound404(sockfd);
    }
    // printf("END ONE PART\n");
    printf("\n\n");
end:
    close(sockfd);
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
        n.ip = inet_ntoa(skaddr.sin_addr);
        n.port = port;
        // printf("message-from-client: %s, %d \n", inet_ntoa(skaddr.sin_addr), port);
        // if (pthread_create(&recv_thread, NULL, thread_recv, &n) < 0)
        // {
        //     printf("create thread error:%s \n", strerror(errno));
        //     break;
        // }
        // pthread_detach(recv_thread);
        if (pthread_create(&recv_thread, NULL, thread_recv, &n) == 0)
        {
        }
        else
        {
            perror("thread fail");
            exit(EXIT_FAILURE);
        }
    }
    close(new_socket);
    shutdown(server_fd, SHUT_RDWR);
    // close(server_fd);
    return 0;
}
