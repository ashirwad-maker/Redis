#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void die(const char* msg){
    int err = errno;
    fprintf(stderr,"[%d]%s\n",err,msg);
    abort();
}

static void msg(char *msg){
    fprintf(stderr,"%s\n",msg);
}

static void do_something(int connfd){
    char rbuf[64] = {};
    ssize_t n = read(connfd,rbuf,sizeof(rbuf)-1); // -1, because of \0 at the end
    if(n<0){
        die("read error");
        return;
    }
    printf("Client says : %s\n",rbuf);
    char wbuf[] = "world";
    write(connfd,wbuf,strlen(wbuf));
}

int main()
{
    int fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234); // ntohs/l -> network to host short/long
    addr.sin_addr.s_addr = ntohl(0);

    //bind
    int rv = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
    if(rv<0){
        die("bind()");
    }

    //listen
    rv = listen(fd,SOMAXCONN); //listen(server_fd,backlog)
    if(rv<0){
        die("listen()");
    }

    //accept
    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd,(struct sockaddr*) &client_addr,&socklen);
        if(connfd<0){
            continue; //error
        }
        do_something(connfd);
        close(connfd);
    } 
}

