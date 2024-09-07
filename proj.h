#ifndef PROJ_H
#define PROJ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_DATA_SIZE 1024
#define MAX_FILE_SIZE 7168  // Max file size that can be transferred
#define CMD_SEND 1
#define CMD_RECV 2
#define CMD_RESP 3
#define CMD_DATA 4
#define STAT_OK 1
#define STAT_FAIL -1

struct send_msg {
    int msg_type; // CMD_SEND or CMD_RECV
    int file_size; // size of file to be sent (if applicable; if not set it to 0)
    char filename[128]; // name of file
};

struct resp_msg {
    int msg_type;
    int status;
    int filesize;
};

struct data_msg {
    int msg_type;  // CMD_DATA
    int data_len;  // Correct field name for data length
    char buffer[MAX_DATA_SIZE];  // Buffer to hold the data
};


int create_socket();
int connect_to_server(int sockfd, const char *server_ip, int server_port);
int send_msg(int sockfd, void *msg, int size);
int receive_msg(int sockfd, void *msg, int size);

#endif
