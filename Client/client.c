#include "client.h"

#define MAXDATASIZE 100000 // max number of bytes we can get at once
#define MAXNOSERVERS 15
char file_name[MAXDATASIZE];
int num_of_servers = 0 ;
struct server_info servers[MAXNOSERVERS];

int main(int argc, char *argv[])
{
   if(port_ip_err(argc))
        return 1;
   int res = connect_server(argv[1], argv[2], 0);
   if(res == 2)
       return 2;
    else if (res == 1)
        return 1;

    return 0;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

int port_ip_err(int argc){
    if (argc < 2) {
        char msg[]="usage: client hostname and port\n";
       write(STDERR_FILENO, msg, strlen(msg)-1);
        return 1;
    }
    return 0;
}

void send_msg (char* message, int sockfd){
  int size = strlen(message);
  // transmit a single message over the socket, if and only if message length is smaller than maximum buffer size
  if (size <= MAXDATASIZE) {
    int bytecount = send(sockfd, message, size + 1, 0);
    if (bytecount < 0)
      write(STDERR_FILENO, "Failed to send a message to sever.", 34);
    return;
}
}

void close_sock(int sockfd){
    close(sockfd);
}

int connect_server(char* ip_addr, char* PORT, int mode){

    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip_addr, PORT, &hints, &servinfo)) != 0) {
        write(STDERR_FILENO, "getaddrinfo err", 15);
        //gai_strerror(rv)
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
    if(mode == 0)
      write(STDERR_FILENO, " in mode 0\n",11);
    else 
      write(STDERR_FILENO, " in mode 1\n",11);


    write(STDOUT_FILENO, "\n", 1);
    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        write(STDERR_FILENO, "recv", 4);
        return 1;
    }
    write(STDOUT_FILENO, buf, numbytes);

    if(!mode)
    {
        send_filename(sockfd);
        recv_serverinfo(sockfd);

    }
    else
    {
        download_part(sockfd);
        write(STDOUT_FILENO, "Download Finished\n",18);

    }


    return 0;
}
void send_filename(int sockfd){

  write(STDOUT_FILENO, "\nEnter the file name you wanna download :\n", 43);
  int size = read(STDIN_FILENO, file_name, MAXDATASIZE);
   file_name[size-1] = '\0';
   char *out, *temp= "filename,";
    if((out = (char *)malloc(strlen(temp) + strlen(file_name) + 1)) != NULL){
        strcpy(out, temp);
        strcat(out, file_name);
    }
    else
        abort();
    send_msg(out, sockfd);
}

void recv_serverinfo(int sockfd){

  char buf[MAXDATASIZE];
  int numbytes;

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        write(STDERR_FILENO, "recv", 4);
        return ;
    }

  write(STDOUT_FILENO,buf, strlen(buf));
  write(STDOUT_FILENO,"\n",1);
  fill_sinfo(buf);
  sort_servers();
  //print_arr();
  get_file();

}

void fill_sinfo(char buffer[])
{
    char *token = strtok(buffer,",");
    if(!strcmp(token,"notfound"))
    {
        write(STDOUT_FILENO,"ERR: FILE NOT FOUND ON SERVER.\n",31);
        exit(EXIT_FAILURE);
    }
    else
        write(STDOUT_FILENO,"RECEIVING INFORMATION...\n",25);
    token = strtok(NULL,",");
    while(token != NULL)
    {
        int i = 0 ;
        for(i = 0 ; i < 3 ; i++)
        {
            if(i == 0){

                    servers[num_of_servers].ip_addr = malloc(sizeof(char) * strlen (token));
                    strcpy(servers[num_of_servers].ip_addr, token);
                    token = strtok(NULL,",");
                }
            else if (i == 1){

                servers[num_of_servers].port = malloc(sizeof(char) * strlen (token));
                strcpy(servers[num_of_servers].port, token);
                token = strtok(NULL,",");

            }
            else{

                servers[num_of_servers].part = malloc(sizeof(char) * strlen (token));
                strcpy(servers[num_of_servers].part, token);
                token = strtok(NULL,",");
            }


        }
        num_of_servers = num_of_servers + 1;

    }
    write(STDOUT_FILENO,"RECEIVED SUCCESSFULLY\n",22);


}
void sort_servers()
{
  int d , c;
  char* part;
  char* ip_addr;
  char* port;

  for (c = 1 ; c <= num_of_servers - 1; c++) {
    d = c;

    while ( d > 0 && atoi(servers[d].part) < atoi(servers[d-1].part) ) {

      part = malloc(sizeof(char) * strlen(servers[d].part));
      ip_addr = malloc(sizeof(char) * strlen(servers[d].ip_addr));
      port = malloc(sizeof(char) * strlen(servers[d].port));
      if(part == NULL || port == NULL || ip_addr == NULL)
        exit(EXIT_FAILURE);

      strcpy(part,servers[d].part);
      strcpy(ip_addr,servers[d].ip_addr);
      strcpy(port,servers[d].port);


      servers[d].part = malloc(sizeof(char)* strlen(servers[d-1].part));
      servers[d].port = malloc(sizeof(char)* strlen(servers[d-1].port));
      servers[d].ip_addr = malloc(sizeof(char)* strlen(servers[d-1].ip_addr));

      if(servers[d].part == NULL || servers[d].port == NULL || servers[d].ip_addr == NULL)
        exit(EXIT_FAILURE);

      strcpy(servers[d].part,servers[d-1].part);
      strcpy(servers[d].port,servers[d-1].port);
      strcpy(servers[d].ip_addr,servers[d-1].ip_addr);


      servers[d-1].port = malloc( sizeof(char) * strlen(port));
      servers[d-1].ip_addr = malloc( sizeof(char) * strlen(ip_addr));
      servers[d-1].part = malloc( sizeof(char) * strlen(part));

      strcpy(servers[d-1].part, part);
      strcpy(servers[d-1].port, port);
      strcpy(servers[d-1].ip_addr, ip_addr);

      d--;
    }
  }

}

void free_info(char** part , char** port , char** ip_addr)
{
    if(*part != NULL){
        free(*part);
        *part = NULL;
    }
    if(*port != NULL){
        free(*port);
        *port = NULL;
    }
    if(*ip_addr != NULL){
        free(*ip_addr);
        *ip_addr = NULL;
    }

}

void print_arr()
{
    int i = 0;
    for(i = 0 ; i<num_of_servers ; i++)
    {
        write(STDOUT_FILENO,servers[i].ip_addr,strlen(servers[i].ip_addr));
        write(STDOUT_FILENO,"\n",1);
        write(STDOUT_FILENO,servers[i].port,strlen(servers[i].port));
        write(STDOUT_FILENO,"\n",1);
        write(STDOUT_FILENO,servers[i].part,strlen(servers[i].part));
        write(STDOUT_FILENO,"\n",1);
    }
}

void get_file(){

    int i = 0;
    for(i = 0 ; i< num_of_servers ; i++)
    {
        connect_server(servers[i].ip_addr,servers[i].port, 1);
    }

}

void download_part(int sockfd){

    write(STDOUT_FILENO,"Downloading parts...\n",21);
    char* msg = "download";
    send_msg(msg, sockfd);
    char buff[MAXDATASIZE];
    int numbytes, file_ds;

    
    if ((file_ds = open(file_name, O_RDWR | O_APPEND | O_CREAT | O_NONBLOCK, 0666)) == -1){
      write(STDERR_FILENO,"ERR: FILE COULD NOT OPEN.\n", 41);
      exit(EXIT_FAILURE);

    }
   
    if ((numbytes = recv(sockfd, buff, MAXDATASIZE-1, 0)) == -1) {
        write(STDERR_FILENO, "recv\n", 5);
        exit(EXIT_FAILURE);
    }

    else{
      
      write(file_ds, buff, numbytes);

    }

      close(file_ds);

}
