#include <stdio.h>
#include <string.h>

void *get_in_addr(struct sockaddr *sa);
char *append_str(char *str1 , char *str2);
int port_ip_err(int argc);
void send_msg(char* message, int sockfd);
void close_sock(int sockfd);
int connect_server (char* ip_addr , char* port);
void send_filename();