#include "proj.h"

int main() {
    int sockfd = create_socket();
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(5000); // Server port

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    listen(sockfd, 5);
    printf("Server listening\n");

    while (1) {
        int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        struct send_msg send_info;
        if (receive_msg(client_fd, &send_info, sizeof(send_info)) <= 0) {
            perror("Failed to receive message");
            close(client_fd);
            continue;
        }

        struct resp_msg response = {CMD_RESP, STAT_OK, 0};
        FILE *file;

        switch (send_info.msg_type) {
        case CMD_SEND:
            file = fopen(send_info.filename, "wb");
            if (!file) {
                perror("Error opening file for writing");
                response.status = STAT_FAIL;
                send_msg(client_fd, &response, sizeof(response));
                close(client_fd);
                continue;
            }

            struct data_msg recv_data;
            int total_bytes = 0;
            while (total_bytes < send_info.file_size && receive_msg(client_fd, &recv_data, sizeof(recv_data)) > 0) {
                fwrite(recv_data.buffer, 1, recv_data.data_len, file);
                total_bytes += recv_data.data_len;
            }

            fclose(file);
            printf("Server: File transfer completed: %s\n", send_info.filename);
            break;

        case CMD_RECV:
            file = fopen(send_info.filename, "rb");
            if (!file) {
                perror("Error opening file for sending");
                response.status = STAT_FAIL;
                send_msg(client_fd, &response, sizeof(response));
                close(client_fd);
                continue;
            }

            fseek(file, 0, SEEK_END);
            int file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            response.filesize = file_size;

            send_msg(client_fd, &response, sizeof(response));

            char buffer[MAX_DATA_SIZE];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                struct data_msg send_data = {CMD_DATA, (int)bytes_read, ""};
                memcpy(send_data.buffer, buffer, bytes_read);
                if (send_msg(client_fd, &send_data, sizeof(send_data)) < 0) {
                    fprintf(stderr, "Error sending file data\n");
                    break;
                }
            }

            // Send an end-of-file message
            struct data_msg end_of_file = {CMD_DATA, 0, ""}; // Zero-length data signifies end of file
            send_msg(client_fd, &end_of_file, sizeof(end_of_file));

            fclose(file);
            printf("Server: File sent successfully: %s\n", send_info.filename);
            break;
        }

        close(client_fd);
    }

    close(sockfd);
    return 0;
}
