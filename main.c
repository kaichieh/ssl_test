#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ssl.h"

int main(int argc, char **argv){
    if(argc <=2)
        printf("usage:%s [host] [port]\n", argv[0]);
    int status,bytes_sent, bytes_receive,error_code;
    char *request = "GET / HTTP/1.1\r\n\r\n";
    char buffer[1024]={0};
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    memset(&hints, 0 , sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if(status = getaddrinfo(argv[1],argv[2],&hints, &serverInfo) !=0){
        printf("fail to get address of %s", argv[1]);
        exit(-1);
    }
    int sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if(sockfd < 0){
        printf("fail to create socket");
        exit(-1);
    }

    if(status = connect(sockfd, serverInfo->ai_addr, serverInfo->ai_addrlen) != 0){
        printf("fail to connect to %s", argv[1]);
        exit(-1);
    }

    const SSL_METHOD * method = SSLv23_client_method();
    if(NULL == method){
        printf("fail to create method");
        exit(-1);
    }
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();
    SSL_CTX * sslCtx = SSL_CTX_new(method);
    if(NULL == sslCtx){
        printf("fail to create sslCtx");
        exit(-1);
    }
    SSL* ssl = SSL_new(sslCtx);
    if(NULL == ssl){
        printf("fail to create ssl");
        exit(-1);
    }
    if(0==SSL_set_fd(ssl, sockfd)){
        printf("fail to set fd to openssl");
        exit(-1);
    }
    if(error_code = SSL_connect(ssl)!=1){
        printf("fail to handshake%d", SSL_get_error(ssl,error_code));
        exit(-1);
    }

    printf("send data\n");
    bytes_sent = SSL_write(ssl,request, strlen(request));
//    bytes_sent = send(sockfd, request, strlen(request), 0);
    if(bytes_sent<0 || bytes_sent!=strlen(request)){
        printf("send to server fail");
        exit(-1);
    }



    printf("receive data\n");
    bytes_receive = SSL_read(ssl, buffer, sizeof(buffer));
//    bytes_receive =  recv(sockfd, buffer, sizeof(buffer),0);
    if(bytes_receive<=0){
        printf("receive from server fail");
        exit(-1);
    }
    printf("%s", buffer);

    close(sockfd);
    return 0;
}
