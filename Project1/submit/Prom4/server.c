/*
Problem 4: server side
use multi-thread techique to serve clients, at most 2 clients can be served simultaneously.
basic idea: use a global variable to keep the number of client in serving, and use mutex to protect this variable.
for each newly coming client,  allocate a thread for it. when a process sending message to server,
check whether it can be served, and return feedback.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define NUM_THREADS_AVAL 2

int num_thread=0,num_working_thread=0; // num_threading: keep the number of client, num_working_thread: keep the number of client in service

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex to keep synchronization of global variable

void *server(void *newsockfd);
int main(int argc, char *argv[])
{   
    ////////////////////////
    // No need to change  //
    int i,flag;
    int sockfd, newsockfd, portno, clilen, n, connectfd;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // the port number (>2000 generally) of server is randomly assigned 
    portno = 2050;
    serv_addr.sin_port = htons(portno); // transfer to net byte order
    // ip of server
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) //binding
    {
        printf("ERROR binding\n");
        exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("Server initiating...\n");
    //////////////////////////////////////////////////

    // Finish your multi-thread service here /////////

    //////////////////////////////////////////////////

    while(1){  
        pthread_t tid;   // creating a new thread id
        if( (connectfd = accept(sockfd, (struct sockaddr*)NULL, NULL)) == -1)
        {  
            printf("ERROR accepting\n");  
            continue;  
        }
        else
            num_thread++;

        // creating a thread
        if(pthread_create(&tid,NULL,server,(void *)(&connectfd)) == -1)
        {
            perror("ERROR thread creating");  
            close(connectfd);  
            close(sockfd); 
            exit(0); 
        }
    }  
    close(sockfd);   
    //////////////////////////////////////////////////
    return 0;
}
void *server(void *sockfd)
{

    int newsockfd = (int)(*((int*)sockfd));
    int n,i,block=1; // block is a flag deciding whether to encrypt for each thread 
    char buffer[256];

    ///////////////////////////////////////////

    //Finish your Encryption service here    //

    ///////////////////////////////////////////
    while(1)
    {
        if((n = recv(newsockfd, buffer, 256, 0)) < 0) // receiving
        {
            perror("ERROR receiving");  // exception case
            break;
        }
        buffer[n] = '\0';
        // client wants to quit regardless of the service state
        if(!strcmp(buffer, ":q"))     
        {
            printf("Server thread closing ...\n");
            break;
        }
        // to check the current whether current can be served 
        pthread_mutex_lock( &mutex );
        if(block && num_working_thread<NUM_THREADS_AVAL)
        {
            num_working_thread++;
            block=0;
        }
        pthread_mutex_unlock( &mutex );

        if(!block)
        {
            printf("Receiving message: %s\n", buffer); 
            // encrypting
            for(i=0; i<n; i++)
            {
                if(buffer[i]>='a'&&buffer[i]<='z')
                    buffer[i] = (buffer[i]-'a'+3)%26+'a';
                if(buffer[i]>='A'&&buffer[i]<='Z')
                    buffer[i] = (buffer[i]-'A'+3)%26+'A';
            }
            // sending
            if( send(newsockfd, buffer, strlen(buffer),0) < 0)
            {
                perror("ERROR sending");
                break;
            } 
        }
        else
        {
            // tell client to wait
            if( send(newsockfd, "Please wait ...", 16,0) < 0)
            {
                perror("ERROR sending");
                break;
            }
        }
    }
    close(newsockfd); 
    // release service place after finish service
    pthread_mutex_lock( &mutex );
    num_thread--;
    if(!block)
        num_working_thread--;
    pthread_mutex_unlock( &mutex );
    pthread_exit(0);
}