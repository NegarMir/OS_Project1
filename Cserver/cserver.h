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

char *append_str (char *str1 , char *str2);
char* itoa (int i, char b[]);
int parameter_err(int);
void run (char* PORT, char* ip_addr);
void send_msg (int indentifier, char* msg);
char recv_msg(int indentifier);
void stop ();
void on_new_connection(int indentifier);
void on_standard_input(char* line);
void on_new_message (int indentifier, char[], int valread);
void on_terminated_connection(int sockfd);
void parse(int sockfd, char[]);
void get_file_part_no();
void get_filename();
void send_file_info(int sockfd, char* PORT, char* ip_addr);
int connect_to_main_server(char* PORT, char* ip_addr);