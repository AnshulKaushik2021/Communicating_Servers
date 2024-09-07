#include "proj.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> -s/-r <filename>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    const char *command = argv[3];
    const char *filename = argv[4];

    int sockfd = create_socket();
    if (connect_to_server(sockfd, server_ip, server_port) < 0) {
        fprintf(stderr, "Error connecting to server\n");
        close(sockfd);
        return 1;
    }

    if (strcmp(command, "-s") == 0) {  // Sending file to server
        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("Error opening file");
            close(sockfd);
            return 1;
        }

        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        struct send_msg send_info = {CMD_SEND, file_size, ""};
        strncpy(send_info.filename, filename, sizeof(send_info.filename) - 1);

        if (send_msg(sockfd, &send_info, sizeof(send_info)) < 0) {
            fprintf(stderr, "Error sending file information\n");
            fclose(file);
            close(sockfd);
            return 1;
        }

        struct data_msg send_data;
        size_t bytes_read;
        while ((bytes_read = fread(send_data.buffer, 1, MAX_DATA_SIZE, file)) > 0) {
            send_data.msg_type = CMD_DATA;
            send_data.data_len = bytes_read;
            if (send_msg(sockfd, &send_data, sizeof(send_data)) < 0) {
                fprintf(stderr, "Error sending file data\n");
                fclose(file);
                close(sockfd);
                return 1;
            }
        }

        fclose(file);
        printf("Client: File transfer completed: %s\n", filename);
    } else if (strcmp(command, "-r") == 0) {  // Receiving file from server
        struct send_msg request = {CMD_RECV, 0, ""};
        strncpy(request.filename, filename, sizeof(request.filename) - 1);

        if (send_msg(sockfd, &request, sizeof(request)) < 0) {
            fprintf(stderr, "Error requesting file\n");
            close(sockfd);
            return 1;
        }

        struct resp_msg response;
        if (receive_msg(sockfd, &response, sizeof(response)) <= 0 || response.status != STAT_OK) {
            fprintf(stderr, "Server response error or file not found\n");
            close(sockfd);
            return 1;
        }

        FILE *file = fopen(filename, "wb");
        if (!file) {
            perror("Error opening file for writing");
            close(sockfd);
            return 1;
        }

        struct data_msg recv_data;
        int total_bytes = 0;
        while (total_bytes < response.filesize && receive_msg(sockfd, &recv_data, sizeof(recv_data)) > 0) {
            fwrite(recv_data.buffer, 1, recv_data.data_len, file);
            total_bytes += recv_data.data_len;
        }

        fclose(file);
        printf("Client: File received successfully: %s\n", filename);
    }

    close(sockfd);
    return 0;
}
