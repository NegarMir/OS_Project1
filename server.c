#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <sys/time.h> ////FD_SET, FD_ISSET, FD_ZERO macros 
#include "server.h"

#define MAXDATASIZE 200
#define BACKLOG 10     // how many pending connections queue will hold
int active = 1 ;
char file_name[MAXDATASIZE];

int main(int argc, char* argv[])
{


    int counter, numbytes;
    int active = 0 ;
    char buf[MAXDATASIZE];
    if(parameter_err(argc)) return 0 ;
    char*  PORT = argv[1] ;
    run(PORT);

    return 0;
}

char *append_str (char *str1 , char *str2){
    char * new_str ;
    if((new_str = malloc(strlen(str1)+strlen(str2)+1)) != NULL){
        new_str[0] = '\0';   // ensures the memory is an empty string
        strcat(new_str,str1);
        strcat(new_str,str2);
        return new_str;
}
   else
    abort(); 
}

char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}

 int parameter_err(int argc){

    if(argc==1){
        const char msg[] = "\n valid operation server [port number]\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return 1;
    }
    return 0 ;
 }

void send_msg (int indentifier, char* msg)
 {

   send(indentifier, msg, strlen(msg), 0);
 }

char recv_msg(int indentifier)
 {
    char buff[MAXDATASIZE];
    recv(indentifier, buff, MAXDATASIZE, 0);
    return *buff;
 }

void stop(){
    char msg[] = "Server is already stopped.\n";
    if(!active)
        write(STDERR_FILENO, msg, strlen(msg));
    active = 0 ;
 }

void on_new_connection(int indentifier){
    char* msg = "\nNEW CONNECTION : ";
    char tmp[5] = {0x0} ;
    write(STDOUT_FILENO, msg, strlen(msg));
    sprintf(tmp,"%4d", indentifier);
    write(STDOUT_FILENO, tmp, strlen(tmp));
 }

void on_standard_input(char* line)
{
  char* msg = "STDIN" ;
  char* quit = ":q";
  write(STDOUT_FILENO, msg, strlen(msg));
  write(STDOUT_FILENO, line, strlen(line));
  if (strcmp(line,quit))
    stop();
}

void on_new_message(int sockfd, char buf[]){

    int numbytes;
    
    char* temp = "\nNEW MESSAGE FROM";
    char id[5] = {0x0} ;
    sprintf(id,"%4d", sockfd);
    write(STDOUT_FILENO, temp, strlen(temp));
    write(STDOUT_FILENO, id, strlen(id));
    write(STDOUT_FILENO, ": ",2);
    write(STDOUT_FILENO, buf, strlen(buf));
    parse(buf, sockfd);

}

void on_terminated_connection(int sockfd){
   char* temp = "\nTERMINATED CONNECTION : ";
    char id[5] = {0x0} ;
    sprintf(id,"%4d", sockfd);
    write(STDOUT_FILENO, temp, strlen(temp));
    write(STDOUT_FILENO, id, strlen(id));
}

void parse(char input[],int sockfd){

     char *token = strtok(input, ",");
     if(!strcmp(token,"filename"))
     {
        token = strtok(NULL, ",");
        memcpy(file_name, token ,strlen(token));
        return;
     }
}

void run(char* PORT){

    int opt = 1;  
    int master_socket , addrlen , new_socket , client_socket[30], activity, i , valread , sd;  
    int max_sd;  
    struct sockaddr_in address;  
    char buffer[1025]; 
         
    fd_set readfds;  
    char *message = "#CONNECTION STABLISHED\n";  
    
    for (i = 0; i < BACKLOG; i++)    //initialise all client_socket[] to 0 so not checked 
    {  
        client_socket[i] = 0;  
    }  
        
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  //create a master socket 
    {  
      write(STDERR_FILENO,"socket failed",13);  
        exit(EXIT_FAILURE);  
    }  
    
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        write(STDERR_FILENO,"setsockopt",10); 
        exit(EXIT_FAILURE);  
    }  
    
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    int port = atoi(PORT);
    printf("%d\n",port );
    address.sin_port = htons( port );  
        
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        write(STDERR_FILENO,"bind failed",11);  
        exit(EXIT_FAILURE);  
    }  
    write(STDOUT_FILENO,"Listener on port ",17);  
    write(STDOUT_FILENO, PORT, strlen(PORT)); 
    write(STDOUT_FILENO,"\n",1);
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 4) < 0)  
    {  
        write(STDOUT_FILENO, "listen",6);  
        exit(EXIT_FAILURE);  
    }  
        
    //accept the incoming connection 
    addrlen = sizeof(address);  
    write(STDOUT_FILENO, "Waiting for connections ...\n", 28);  
        
    while(1)  
    {  
        FD_ZERO(&readfds);   //clear the socket set    
        FD_SET(master_socket, &readfds);  //add master socket to set 
        max_sd = master_socket;  
            
        //add child sockets to set 
        for ( i = 0 ; i < BACKLOG ; i++)  
        {  
            sd = client_socket[i];  //socket descriptor 
            if(sd > 0)  //if valid socket descriptor then add to read list 
                FD_SET( sd , &readfds);  
                
            if(sd > max_sd)   //highest file descriptor number, need it for the select function 
                max_sd = sd;  
        }  

        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); //wait for an activity on one of the sockets 
      
        if ((activity < 0) && (errno!=EINTR))  
        {
            write(STDERR_FILENO,"select error", 12);  
        }  
            
        if (FD_ISSET(master_socket, &readfds)) //If something happened on the master socket,then its an incoming connection 
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                write(STDOUT_FILENO,"accept err", 10);  
                exit(EXIT_FAILURE);  
            }  
            
            on_new_connection(new_socket);
            send_msg(new_socket, message);
               
            //add new socket to array of sockets 
            for (i = 0; i < BACKLOG; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    break;  
                }  
            }  
        }  
            
        //else its some IO operation on some other socket
        for (i = 0; i < BACKLOG; i++)  
        {  
            sd = client_socket[i];  
                
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 1024)) == 0)  
                {  
                    on_terminated_connection(sd);  
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                    
                //HANDLE MESSAGE
                else
                {  
                    on_new_message(sd, buffer);
                }  
            }  
        }  
    }  
        
     
} 
