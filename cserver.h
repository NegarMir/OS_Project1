#include <stdio.h>
#include <string.h>

char *append_str (char *str1 , char *str2);
char* itoa (int i, char b[]);
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int parameter_err(int);
int bind_err(struct addrinfo* p);
int listen_err(int sockfd);
void run (int sockfd, int new_fd, struct sockaddr_storage their_addr, char* s);
void send_msg (int indentifier, char* msg);
char recv_msg(int indentifier);
void stop ();
void on_new_connection(int indentifier);
void on_standard_input(char* line);
int on_new_message (int indentifier);
void get_file_part_no();
void get_filename();
void send_file_info();
int connect_to_main_server();