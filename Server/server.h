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

struct server_info{
	char* ip_addr;
	char* port ; 
	char* file_name;
	char* part;

};

char *append_str (char *str1 , char *str2);
char* itoa (int i, char b[]);
void *get_in_addr(struct sockaddr *sa);
int parameter_err(int);
void run(char* PORT, char* ip_addr);
void send_msg (int indentifier, char* msg);
char recv_msg(int indentifier);
void stop ();
void on_new_connection(int indentifier);
void on_standard_input(char* line);
void on_terminated_connection(int sockfd);
void on_new_message (int indentifier, char[], int valread);
void parse (char [], int sockfd);
void fill_server_info (char* token);
void send_sinfo_to_client(int sockfd);
void inc_num_of_servers();
