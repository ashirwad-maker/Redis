#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <cassert>

const int max_msg_length = 4096;

static void die(const char* msg){
    int err = errno;
    fprintf(stderr,"[%d]%s\n",err,msg);
    abort();
}

static void msg(char *msg){
    fprintf(stderr,"%s\n",msg);
}

static int32_t read_full(int fd,char *buf,size_t n)
{
    while(n>0)
    {
        ssize_t rv = read(fd,buf,n);
        if(rv<=0){
            die("read()");
        }
        assert((size_t)rv<=n);
        n-=(size_t)rv;
        buf+=rv;
    }
    return 0;
}

static int write_all(int fd,char *buf,size_t n)
{
    while(n>0)
    {
        ssize_t rv = write(fd,buf,n);
        if(rv<=0){
            return -1;
        }
        assert((size_t)rv<=n);
        n-=(size_t)rv;
        buf+=rv;
    }
    return 0;
}

static int32_t one_request(int connfd)
{
    char read_buf[4+max_msg_length+1];
    errno=0;
   int32_t err = read_full(connfd,read_buf,4);
   if(err){
    if(errno == 0) 
        msg("EOF");
    else {
        msg("read() error");
     }
     return err;
   }
   uint32_t len = 0;
   memcpy(&len,read_buf,4);
   if(len>max_msg_length)
   {
    msg("too long");
    return -1;
   }
   // request by the client 
   err = read_full(connfd,&read_buf[4],len);
   if(err){
    msg("read error");
    return err;
   }

   // do something. 
   read_buf[4+len] = '\0';
   printf("Client Says: %s and the length of request is : %d\n",&read_buf[4],len);

    const char world[] = "world";
    char write_buf[4 + sizeof(world)];
    len = (uint32_t)strlen(world);
    memcpy(write_buf,&len,4);
    memcpy(&write_buf[4],world,len);
    return write_all(connfd,write_buf,4+len);

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


        // do_something(connfd);
        while (true) {
            // here the server only serves one client connection at once
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        close(connfd);
    } 
    return 0;
}
