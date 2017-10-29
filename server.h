#include <stdio.h>
#include <string.h>

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
void run(char* PORT);
void send_msg (int indentifier, char* msg);
char recv_msg(int indentifier);
void stop ();
void on_new_connection(int indentifier);
void on_standard_input(char* line);
void on_terminated_connection(int sockfd);
void on_new_message (int indentifier, char[], int valread);
void parse (char [], int sockfd);
void sort_arr();
void fill_server_info (char* token);
void inc_num_of_servers();