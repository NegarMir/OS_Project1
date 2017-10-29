#include <stdio.h>
#include <string.h>

struct server_info{
	char* ip_addr;
	char* port ; 
	char* file_name;
	char* part;

};

void *get_in_addr(struct sockaddr *sa);
char *append_str(char *str1 , char *str2);
int port_ip_err(int argc);
void send_msg(char* message, int sockfd);
void close_sock(int sockfd);
int connect_server (char* ip_addr , char* port);
void send_filename();
void recv_serverinfo(int sockfd);
void fill_sinfo(char buf[]);
void free_info(char** part , char** port , char** ip_addr);
void download_file();
void sort_servers();
void print_arr();