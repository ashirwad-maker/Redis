//Implementation of server. 

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

int main(int argc, char const* argv[])
{
    int server_fd,new_socket;
    ssize_t valread; // ssize_t is a signed inter type and is used in functions like read(),write(),recv()
    struct sockaddr_in address; /* sockaddr_in is a data structure in C in context of socket programming 
                                    The members are :- 
                                        short sin_family; Address Famiily(AF_INET)  
                                        unsigned short sin_port; Port Number
                                        struct in_addr sin_addr; IPV 4 address
                                        char  sin_zero[8]; // Padding to match size of sockaddr*/
    
    
    
    int opt=1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello From server";

    // Creating a file descriptor
    if((server_fd = socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Socket Failure");
        exit(EXIT_FAILURE);
    }

    //Forcefully attaching socket to the port 8080
    if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR , &opt,sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //Forcefully attaching socket to the port 8080
    if(bind(server_fd,(struct sockaddr*) &address,sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd,3)<0){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    if((new_socket = accept(server_fd,(struct sockaddr*)&address,&addrlen))<0){
        perror("ACCEPT");
        exit(EXIT_FAILURE);
    }
    valread = read(new_socket,buffer,1024-1);
    printf("%s\n",buffer);
    send(new_socket,hello,strlen(hello),0);
    printf("Hello Message Sent\n");
    
    // Closing the connected socket 
    close(new_socket);
    //closing the listening socket
    close(server_fd);
    return 0;

}