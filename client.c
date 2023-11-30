#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("> ");
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // Send user input to the server
        send(client_socket, buffer, strlen(buffer), 0);

        // Receive response from the server
        //have a dummy variable seperate from the input bugger
        char server_response[BUFFER_SIZE];
        //intialize the buffer to all 0's 
        memset(server_response, 0, BUFFER_SIZE); 

        recv(client_socket, server_response, BUFFER_SIZE, 0);
        server_response[BUFFER_SIZE-1] = '\0';
        
        printf("%s\n", server_response);
        
        printf("> ");
    }

    close(client_socket);
    return 0;
}