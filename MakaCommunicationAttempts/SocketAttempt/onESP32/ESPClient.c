#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 12345 
#define SERVERIP        "192.168.10.1"
int main(int argc, char const *argv[])
{
        int sock = 0;
        struct sockaddr_in serv_addr;
        char cmd,option;
        char buffer[1024] = {0};
        //Create a socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
                printf("\n Socket creation error \n");
                return -1;
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        // Convert IPv4 addresses from text to binary form 
        if(inet_pton(AF_INET,SERVERIP, &serv_addr.sin_addr)<=0)
        {
                printf("\nInvalid address/ Address not supported \n");
                return -1;
        }
        //Establish a connection with a TCP server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
                printf("\nConnection Failed \n");
                return -1;
        }
        do
        {
                //Send and receive data
                printf("\nEnter 1 - on or 0 - off = ");
                scanf("%c",&cmd);
                send(sock , &cmd , 1 , 0 );
                read( sock , buffer, 1024);
                printf("%s\n",buffer );
                printf("Do you want to continue = ");
                scanf("\n%c",&option);
                getchar();
        }while(option == 'y' || option == 'Y');
        close(sock);
        return 0;
}