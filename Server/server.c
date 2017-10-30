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

#define MAXDATASIZE 100000
#define BACKLOG 10     // how many pending connections queue will hold
#define MAXNOSERVERS 15
int active = 1 ;
int num_of_servers = 0;
char file_name[MAXDATASIZE];
struct server_info servers[MAXNOSERVERS];

int main(int argc, char* argv[])
{


    int counter, numbytes;
    int active = 0 ;
    char buf[MAXDATASIZE];
    if(parameter_err(argc)) return 0 ;
    char*  PORT = argv[2] ;
    char* ip_addr = argv[1];
    run(PORT, ip_addr);

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

    if(argc == 2 || argc == 1){
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

void on_new_message(int sockfd, char buf[], int valread){

    int numbytes;
    
    char* temp = "\nNEW MESSAGE FROM";
    char id[5] = {0x0} ;
    sprintf(id,"%4d", sockfd);
    write(STDOUT_FILENO, temp, strlen(temp));
    write(STDOUT_FILENO, id, strlen(id));
    write(STDOUT_FILENO, ": ",2);
    write(STDOUT_FILENO, buf, valread);
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
        send_sinfo_to_client(sockfd);
        return;
     }
     else if(!strcmp(token,"fileinfo"))
     {
     	token = strtok(NULL,",");
     	fill_server_info(token);
     	return;

     }
}

void fill_server_info(char* token){

    int i = 0;

	while(token != NULL)
	{
		if(i == 0)
		{
			servers[num_of_servers].file_name = malloc(sizeof(char ) * strlen(token)) ;
			strcpy(servers[num_of_servers].file_name, token);
		
		}
		else if (i == 1)
		{
			servers[num_of_servers].part = malloc(sizeof(char) * strlen (token));
			strcpy(servers[num_of_servers].part, token);
		
		}
		else if (i == 2)
		{
			servers[num_of_servers].ip_addr = malloc(sizeof(char) * strlen (token));
			strcpy(servers[num_of_servers].ip_addr, token);

		}
		else
		{
			servers[num_of_servers].port = malloc(sizeof(char) * strlen (token));
			strcpy(servers[num_of_servers].port, token);

		}
		token = strtok(NULL, ",");
		i = i + 1 ;

	}
	inc_num_of_servers();
}

void inc_num_of_servers()
{
	num_of_servers = num_of_servers + 1 ;
}

void send_sinfo_to_client(int sockfd)
{
	write(STDOUT_FILENO,"\nSending info to client\n",23);

	char* delimit = ",";
	char* temp = "info";
	int size = 0 ;
	for(int i = 0 ; i < num_of_servers ; i++)
	{
		size = size + 1 +  strlen(servers[i].port) + 1 + strlen(servers[i].part) + 1 + strlen(servers[i].ip_addr);

	}
	size = size + strlen(temp) + 1;
	char* msg = (char*)malloc(size);
	strcpy(msg, temp);

	for(int i = 0 ; i < num_of_servers ; i++)
	{
        strcat(msg,delimit);
		strcat(msg,servers[i].ip_addr);
		strcat(msg, delimit);
		strcat(msg, servers[i].port);
		strcat(msg, delimit);
		strcat(msg, servers[i].part);

	}

	//write(STDOUT_FILENO,msg,strlen(msg));
	//write(STDOUT_FILENO,"\n",1);
	send_msg(sockfd,msg);
	free(msg);

}

void run(char* PORT, char* ip_addr){

    int opt = 1;  
    int master_socket , addrlen , new_socket , client_socket[30], activity, i , valread , sd;  
    int max_sd;  
    char buf[MAXDATASIZE];
    struct sockaddr_in address;  
         
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
    address.sin_addr.s_addr = inet_addr(ip_addr);  
    int port = atoi(PORT);
    address.sin_port = htons( port );  
        
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        write(STDERR_FILENO,"bind failed\n",12);  
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
    write(STDOUT_FILENO, "Waiting for connections ...\n", 29);  

    while(1)  
    {   
        /*char input[MAXDATASIZE];
        char* q = ":q";
        memset(input, 0, MAXDATASIZE);

        int input_size = read(STDIN_FILENO, input, MAXDATASIZE);
        input[input_size - 1] = '\0';
        
        write(STDOUT_FILENO, input,sizeof(input));
        if(input_size > 0)
        {
            if(!strcmp(input, ":q"))
                return;
        }*/
        

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
        	memset(buf, 0, sizeof(buf));
            sd = client_socket[i];  
                
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                
                if ((valread = read( sd , buf, 1024)) == 0)  
                {  
                    on_terminated_connection(sd);  
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                    
                //HANDLE MESSAGE
                else
                {  
                    on_new_message(sd, buf, valread);
                }  
            }  
        }  
    }  
        
     
} 

