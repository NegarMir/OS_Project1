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
#include <fcntl.h>
#include "cserver.h"

#define MAXDATASIZE 100000
#define BACKLOG 10     // how many pending connections queue will hold

int active = 1 ;
char file_part_no[MAXDATASIZE] ;
char file_name[MAXDATASIZE];
char*  mainserver_PORT;
//char* PORT;

int main(int argc, char* argv[])
{
    int counter, numbytes;
    int active = 0 ;
    char buf[MAXDATASIZE];
    if(parameter_err(argc)) return 0 ;
    get_filename();
    get_file_part_no();
    connect_to_main_server(argv[1]);
    run(argv[1]);
    return 0;
}


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
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


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

 int parameter_err(int argc){

    if(argc==1){
        const char msg[] = "\n valid operation server [port number]\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return 1;
    }
    return 0 ;
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
    address.sin_port = htons( port );  
        
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        write(STDERR_FILENO,"bind failed",11);  
        exit(EXIT_FAILURE);  
    }  
    write(STDOUT_FILENO,"\nListener on port ",18);  
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
            memset(buffer, 0, sizeof(buffer));
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
                    on_new_message(sd, buffer, valread);
                }  
            }  
        }  
    }  
        
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

void on_new_message(int sockfd,char buf[], int valread){
   
    char* temp = "\nNEW MESSAGE FROM";
    char id[5] = {0x0} ;
    sprintf(id,"%4d", sockfd);

    write(STDOUT_FILENO, temp, strlen(temp));
    write(STDOUT_FILENO, id, strlen(id));
    write(STDOUT_FILENO, " : ",3);
    write(STDOUT_FILENO, buf, valread);
    write(STDOUT_FILENO, "\n",1);
    parse(sockfd, buf);

}

void on_terminated_connection(int sockfd){
   char* temp = "\nTERMINATED CONNECTION : ";
    char id[5] = {0x0} ;
    sprintf(id,"%4d", sockfd);
    write(STDOUT_FILENO, temp, strlen(temp));
    write(STDOUT_FILENO, id, strlen(id));
}

void get_file_part_no(){

    write(STDOUT_FILENO, "Enter the part number of the file that this server contains.\n", 61);
    int size = read(STDIN_FILENO, file_part_no, MAXDATASIZE);
    memcpy(file_part_no, file_part_no, size);
    file_part_no[size-1] = '\0';
}

void parse(int sockfd, char input[MAXDATASIZE]){

    char buff[MAXDATASIZE];
    memset(buff, 0, sizeof(buff));
    if(!strcmp(input,"download"))
    {
        write(STDOUT_FILENO,file_name,sizeof(file_name));
        int retval = open(file_name, O_RDONLY, O_NONBLOCK);

        if(retval < 0)
        {
            write(STDERR_FILENO,"ERR : FILE NOT FOUND.\n",22);
            exit(EXIT_FAILURE);
        }

        char* msg;
        int numbytes = read (retval,buff,sizeof(buff)-1);
        msg = malloc(sizeof(char) * numbytes);
        strcpy(msg,buff);
        send_msg(sockfd, buff);
    }
  
}

int connect_to_main_server(char* PORT){

    int sockfd, numbytes, rv;
    char buf[MAXDATASIZE], mains_PORT[MAXDATASIZE], ip_addr[MAXDATASIZE], s[INET6_ADDRSTRLEN];
    char * ptr  , * ptr1 ;
    struct addrinfo hints, *servinfo, *p;
    write(STDOUT_FILENO, "Enter main server's ip address.\n", 32);
    int size = read(STDIN_FILENO, ip_addr, MAXDATASIZE);
    ip_addr[size - 1] = '\0';
    ptr = malloc((size-1)*sizeof(char));
    ptr = &ip_addr[0];
    write(STDOUT_FILENO, "Enter main server's port number.\n", 33);
    size = read(STDIN_FILENO, mains_PORT, MAXDATASIZE);
    memcpy(mains_PORT, mains_PORT, size);
    mains_PORT[size - 1] = '\0';
    ptr1 = malloc((size-1)*sizeof(char));
    ptr1 = & mains_PORT[0];
    memcpy(ptr1,ptr1,size-1);



    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ptr, ptr1, &hints, &servinfo)) != 0) {
        write(STDERR_FILENO, "getaddrinfo err\n", 16);
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            write(STDERR_FILENO, "client: socket err",18 );
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            write(STDERR_FILENO, "client: connect", 15);
            continue;
        }

        break;
    }

    if (p == NULL) {
        char msg[] =  "CLIENT: FAILED TO CONNECT\n";
        write(STDERR_FILENO, msg, strlen(msg)-1);
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
    char* token = "CLIENT : CONNECTING TO ";
    char* new_msg = append_str(token,s);
    write(STDOUT_FILENO, new_msg, strlen(new_msg));
    write(STDOUT_FILENO, "\n",1);

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        write(STDERR_FILENO, "recv", 4);
        return 1;
    }
    write(STDOUT_FILENO, buf, numbytes);
    send_file_info(sockfd, PORT);
   /* while(1)
    {

    }*/
    return 0;
}

void get_filename(){

   write(STDOUT_FILENO, "Enter the file name you have:\n", 31);
   int size = read(STDIN_FILENO, file_name, MAXDATASIZE);
   file_name[size-1] = '\0';
}

void send_file_info(int sockfd, char* PORT){

      char* str1 = "fileinfo,";
      char* str2 = ",";
      char* IP = "127.0.0.1";

      char* str3 = (char*) malloc(1 + strlen(str1)+ 
                    strlen(file_name) + 1 + strlen(file_part_no) + 1 + strlen(IP) + 1 + strlen(PORT));

      strcpy(str3, str1);
      strcat(str3, file_name);
      strcat(str3,str2);
      strcat(str3,file_part_no);
      strcat(str3, str2);
      strcat(str3, IP);
      strcat(str3 , str2);
      strcat(str3, PORT);
      send_msg(sockfd, str3);
}