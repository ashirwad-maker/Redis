// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdio.h>
// #include <errno.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <netinet/ip.h>
// #include <cassert>
// const int max_msg_length = 4096;

// static void die(const char* msg){
//     int err = errno;
//     fprintf(stderr,"[%d]%s\n",err,msg);
//     abort();
// }

// static void msg(char *msg){
//     fprintf(stderr,"%s\n",msg);
// }


// static int32_t read_full(int fd,char *buf,size_t n)
// {
//     printf("Length** : %d",&n);
//     while(n>0)
//     {
//         ssize_t rv = read(fd,buf,n);
//         if(rv<=0){
//             msg("read() in read_full()");
//             return rv;
//         }
//         assert((size_t)rv<=n);
//         n-=(size_t)rv;
//         buf+=rv;
//     }
//     return 0;
// }

// static int write_full(int fd,char *buf,size_t n)
// {
//     while(n>0)
//     {
//         ssize_t rv = write(fd,buf,n);
//         if(rv<=0)
//             die("write()");
//         assert((size_t)rv<=n);
//         n-=(size_t)rv;
//         buf+=rv;
//     }
//     return 0;
// }
// void gg()
// {
//     uint32_t a= 1;
//     char s[4096];
//     memcpy(s,(char *)&a,sizeof(a));
//     printf("gg() -> %s , %d\n",s,sizeof(a));
// }

// static int32_t query(int fd,char *text){
//     char write_buf[4+max_msg_length+1];
//     uint32_t len = (uint32_t)strlen(text);
//     if(len > max_msg_length){
//         return -1;
//     }
//     memcpy(write_buf,&len,(size_t)4);
//     memcpy(&write_buf[4],text,(size_t)len);
//     printf("Command is %s and length is %d and text is %s\n",write_buf,len,text);

//     gg();

//     uint32_t len1 = (uint32_t)strlen(write_buf);
//     printf("%d\n",len1);
//     for(int i=0;write_buf[i]!='\0';i++){
//         printf("i : %c",write_buf[i]);
//     }
//     return 0;

//     int32_t err = write_full(fd,write_buf,4+len);
//     if(err) return err;

//     char read_buf[4+max_msg_length+1];
//     errno = 0;
//     int rv = read_full(fd,read_buf,4);
//     if(rv<0) die("  read1");
//     memcpy(&len,read_buf,4);
//     if(len>max_msg_length){
//         msg("too long message");
//         return -1;
//     }
//     rv = read_full(fd,read_buf+4,len);
//     if(rv){
//         msg("read() in query()");
//         return rv;
//     }
//     read_buf[4+len] = '\0';
//     printf("server says: %s\n",read_buf+4);
//     return 0;
// }

// int main()
// {
//     int fd = socket(AF_INET,SOCK_STREAM,0);
//     if(fd<0){
//         die("socket");
//     }
//     struct sockaddr_in addr = {};
//     addr.sin_family = AF_INET;
//     addr.sin_port = ntohs(1234);
//     addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

//     //connect
//     int rv = connect(fd,(struct sockaddr*)&addr,sizeof(addr));
//     if(rv){
//         die("connect");
//     }
//     char msg[] = "hello";
//     write(fd,msg,strlen(msg));
//     char rbuf[64] = {};
//     ssize_t n = read(fd,rbuf,sizeof(rbuf)-1);
//     if(n<0){
//         die("read");
//     }
//     printf("server says: %s\n",rbuf);

//     int32_t err = query(fd,"hello1");
//     if(err) goto L_DONE;
//     // err = query(fd,"hello2");
//     // if(err) goto L_DONE;
//     // err = query(fd,"hello3");
//     // if(err) goto L_DONE;

//     L_DONE:

//     close(fd);
//     return 0;
// }



#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>


static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error, or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

const size_t k_max_msg = 4096;

static int32_t query(int fd, const char *text) {
    uint32_t len = (uint32_t)strlen(text);
    if (len > k_max_msg) {
        return -1;
    }

    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);  // assume little endian
    memcpy(&wbuf[4], text, len);
    if (int32_t err = write_all(fd, wbuf, 4 + len)) {
        return err;
    }

    // 4 bytes header
    char rbuf[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    memcpy(&len, rbuf, 4);  // assume little endian
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    printf("server says: %s\n", &rbuf[4]);
    return 0;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    // multiple requests
    int32_t err = query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }
    err = query(fd, "hello3");
    if (err) {
        goto L_DONE;
    }
    err = query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(fd);
    return 0;
}
