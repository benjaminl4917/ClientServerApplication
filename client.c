#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 256

int validDate(const char *date1) {
    //printf("date1: %s date2: %s betweenDate: %s\n", date1, date2, betweenDate);

    int year, month, day;

    // Parse date
    sscanf(date1, "%d-%d-%d", &year, &month, &day);
    
    if (year < 0 ){
        return 0;
    }
    //index 1 = January index 12 = December
    int validDay[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
     //check if the month makes sense 
    if (month <= 0 || month>12){
        return 0;
    }
    if (day > validDay[month] || day<=0){
        return 0;
    }
    return 1; //return 1 if the date is valid 
}

int checkStartEnd(const char *date1, const char *date2) {
    //printf("date1: %s date2: %s betweenDate: %s\n", date1, date2, betweenDate);

    int year1, month1, day1, year2, month2, day2;

    // Parse date1
    sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);

    // Parse date2
    sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);


    if (year1 > year2){
        return 0;
    }
    else if (year1 == year2){
        if (month1 > month2){
            return 0;
        }
        else if (month1 == month2 && day1 > day2){
            return 0;
        }
    }

    return 1;
}

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
    int quit = 0;

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("> ");
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // Send user input to the server
        if (strncmp(buffer, "Prices", 6)  == 0){
            char stock_name[20], date[11];
            sscanf(buffer, "%*s %s %s", stock_name, date); 

            if(strcmp(stock_name, "MSFT") != 0 && strcmp(stock_name, "TSLA") != 0){
                printf("Invalid Syntax\n");
                printf("> ");
                continue;
            }
            //makes sure the date is a real date
            if (validDate((date)) == 0){ 
                printf("Invalid Syntax\n");
                printf("> ");
                continue;
            }
        }
        else if(strncmp(buffer, "MaxProfit", 9) == 0){
            char stock_name[20], start_date[11], end_date[11];
            sscanf(buffer, "%*s %s %s %s", stock_name,start_date, end_date); 

            //makes sure the date is a real date 
            if(strcmp(stock_name, "MSFT") != 0 && strcmp(stock_name, "TSLA") != 0){
                printf("Invalid Syntax\n");
                printf("> ");
                continue;
            }
            if (validDate((start_date)) == 0 || validDate((end_date)) == 0 ){ 
                printf("Invalid Syntax\n");
                printf("> ");
                continue;
            } //make sure that the start_date is before the end_date
            else if(checkStartEnd(start_date, end_date) == 0){
                printf("Unknown\n");
                printf("> ");
                continue;
            }
        }

        send(client_socket, buffer, strlen(buffer), 0);

        // Receive response from the server
        //have a dummy variable seperate from the input bugger
        char server_response[BUFFER_SIZE];
        //intialize the buffer to all 0's 
        memset(server_response, 0, BUFFER_SIZE); 

        recv(client_socket, server_response, BUFFER_SIZE, 0);
        server_response[BUFFER_SIZE-1] = '\0';


        if(strcmp(server_response,"Quit") == 0){
            printf("client closing\n");
            quit = 1;
            break;
        }
        printf("%s\n", server_response);
        
        printf("> ");
    }

    if (quit == 1){
        close(client_socket);
    }
    return 0;
}