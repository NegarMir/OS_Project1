#include <stdio.h>
#include <string.h>

char *append_str (char *str1 , char *str2);
char* itoa (int i, char b[]);
void *get_in_addr(struct sockaddr *sa);
int parameter_err(int);
void run(char* PORT);
void send_msg (int indentifier, char* msg);
char recv_msg(int indentifier);
void stop ();
void on_new_connection(int indentifier);
void on_standard_input(char* line);
void on_terminated_connection(int sockfd);
void on_new_message (int indentifier, char[]);
void parse (char [], int sockfd);