/*
Problem 4: client side
sending message to server, and get the result from server.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
int main(int argc, char *argv[])
{
    /////////////////////////////////
    // No need to change           //
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256], content[256];
    // the port number of server
    portno = 2050;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }
    server = gethostbyname("127.0.0.1");
    if(server == NULL)
    {
        printf("ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);
    
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("ERROR connecting\n");
        exit(1);
    }
    printf("Please enter the message:\n");
    ////////////////////////////////////

    //   Finish your client here   /////

    ////////////////////////////////////

    while(1){ 
        gets(content);
        // send message to server 
        if( send(sockfd, content, strlen(content), 0) < 0)   
        {  
            perror("ERROR send message");  // exception case
            exit(1);  
        }
        // leave from server
        if(!strcmp(content, ":q"))
            break; 
        // get encrypted message
        if((n = recv(sockfd, buffer, 256,0)) < 0) 
        {  
           perror("ERROR receiving");  // exception case
           exit(1);  
        } 
        // add an end symbol for buffer
        buffer[n]  = '\0';  
        printf("From server: %s\n",buffer); 
    }
    printf("client closing ...\n");
    close(sockfd); 
    exit(0);
    ////////////////////////////////////

    return 0;
}