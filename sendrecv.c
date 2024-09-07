#include "proj.h"

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Failed to create socket");
        exit(1);
    }
    return sockfd;
}

int connect_to_server(int sockfd, const char *server_ip, int server_port)
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection Failed");
        return -1;
    }
    return 0;
}

int send_msg(int sockfd, void *msg, int size)
{
    if (write(sockfd, msg, size) < size)
    {
        perror("Failed to send message");
        return -1;
    }
    return 0;
}

int receive_msg(int sockfd, void *msg, int size)
{
    int total_received = 0, bytes_received;
    while (total_received < size)
    {
        bytes_received = read(sockfd, msg + total_received, size - total_received);
        if (bytes_received < 0)
        {
            perror("Error reading from socket");
            return -1;
        }
        else if (bytes_received == 0)
        {
            break; // Connection closed
        }
        total_received += bytes_received;
    }
    return total_received;
}
