#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_STOCKS 1000
#define MAX_ROWS 1000
#define MAX_COLS 10
#define BUFFER_SIZE 1024

typedef struct {
    char name[10];
    char date[20]; // Assuming the date format fits within 20 characters
    double open;
    double high;
    double low;
    double close;
    double adj_close;
    int volume;
} StockData;


void read_stock_data(const char *filename, StockData stock_data[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    char *token;
    int current_row = 0;

    size_t filename_len = strlen(filename);
    char *modified_filename = malloc(filename_len);
    if (modified_filename == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    strncpy(modified_filename, filename, filename_len - 4); // Copy the filename excluding the last 4 characters
    modified_filename[filename_len - 4] = '\0'; // Add the null terminator

    strcpy(stock_data[0].name, modified_filename);
    free(modified_filename);

    // Skip header row
    if (fgets(buffer, BUFFER_SIZE, file) == NULL) {
        fprintf(stderr, "Empty file or error reading header: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    //iterates throughout the csv
    //gets the data from each row and initializes the attributes for the objects
    //each element in the stock_data array is an object with each column as an attribute
    while (fgets(buffer, BUFFER_SIZE, file) != NULL && current_row < MAX_ROWS) { 
        token = strtok(buffer, ",");
        strcpy(stock_data[current_row].date, token);

        token = strtok(NULL, ",");
        stock_data[current_row].open = atof(token);

        token = strtok(NULL, ",");
        stock_data[current_row].high = atof(token);

        token = strtok(NULL, ",");
        stock_data[current_row].low = atof(token);

        token = strtok(NULL, ",");
        stock_data[current_row].close = atof(token);

        token = strtok(NULL, ",");
        stock_data[current_row].adj_close = atof(token);

        token = strtok(NULL, ",");
        stock_data[current_row].volume = atoi(token);

        current_row++;
    }

    fclose(file);
}

void send_message(int client_socket, const char *msg) {
    size_t len = strlen(msg);
    if (len > 255) {
        fprintf(stderr, "Message too long\n");
        return;
    }

    // char buffer[BUFFER_SIZE];
    // buffer[0] = (char)len;
    // strncpy(buffer + 1, msg, len);

    send(client_socket, msg, len, 0);
}

int range(const char *date1, const char *date2, const char *betweenDate) {
    //printf("date1: %s date2: %s betweenDate: %s\n", date1, date2, betweenDate);

    int year1, month1, day1, year2, month2, day2, yearBetween, monthBetween, dayBetween;

    // Parse date1
    sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);

    // Parse date2
    sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);

    // Parse betweenDate
    sscanf(betweenDate, "%d-%d-%d", &yearBetween, &monthBetween, &dayBetween);

    // Compare dates
    if (yearBetween > year1 && yearBetween < year2) {
        return 1; // betweenDate is within the range
    } else if (yearBetween == year1 && yearBetween == year2) {
        if (monthBetween > month1 && monthBetween < month2) {
            return 1; // betweenDate is within the range
        } else if (monthBetween == month1 && monthBetween == month2) {
            if (dayBetween >= day1 && dayBetween <= day2) {
                return 1; // betweenDate is within the range
            }
        }
    }

    return 0; // betweenDate is not within the range
}


int handle_client_request(int client_socket, StockData stock_data_one[], StockData stock_data_two[]) {
    char buffer[BUFFER_SIZE];
    ssize_t received;
    
    while ((received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[received] = '\0';
        printf("Received message from client: %s\n", buffer);

        // Implement logic to handle different commands
        if (strncmp(buffer, "List", 4) == 0) {
            // Implement List command logic
            send_message(client_socket, "TSLA | MSFT"); // Assuming TSLA and MSFT are available
        }
        else if (strncmp(buffer, "Prices", 6) == 0) {
            char stock_name[20], date[11];
            //skips the first word price and puts the stock name and date into assigned variables
            sscanf(buffer, "%*s %s %s", stock_name, date); 

            double price = 0.0;
            // Search for the stock price in the provided stock data based on stock_name and date
            for (int i = 0; i < MAX_ROWS; i++) {
                if (strcmp(stock_data_one[0].name, stock_name) == 0 && strcmp(stock_data_one[i].date, date) == 0) {
                    price = stock_data_one[i].close;
                    break;
                } else if (strcmp(stock_data_two[0].name, stock_name) == 0 && strcmp(stock_data_two[i].date, date) == 0) {
                    price = stock_data_two[i].close;
                    break;
                }
            }
            //if the date is not in the CSV then return unknown to the client
            if(price == 0.0){
                send_message(client_socket, "Unknown"); 
                break;
            }
            char price_str[20];
            snprintf(price_str, sizeof(price_str), "%.2f", price);

            send_message(client_socket, price_str); // Send the stock price information back to the client
        }
        else if (strncmp(buffer, "MaxProfit", 9) == 0) {
            // Implement MaxProfit command logic
            // Extract stock name, start date, and end date
            // Calculate maximum profit and send it back to the client
            // For example:
            char stock_name[20], start_date[11], end_date[11];

            //makes sure that the start_date is less than the end_date


            sscanf(buffer, "%*s %s %s %s", stock_name, start_date, end_date); 
            //find the closing stock price for both the start_date and end_date 

            double start_price = 3000.0; // Replace with your logic to fetch the stock price
            double difference = 0.0;
            //printf("stock name : %s stock array name: %s\n",stock_name, stock_data_one[0].name);
            //printf("date in between: %s \n",stock_data_one[0].date);

            //iterate through the assigned stock name array 
            if (strcmp(stock_name,stock_data_one[0].name)==0){
                for(int i = 0; i< MAX_ROWS; i++){
                    //checks if the current date is in range 
                    //printf("date in between: %s \n",stock_data_one[i].date);

                    int check = range(start_date,end_date,stock_data_one[i].date);
                    //printf("check : %d\n", check);
                    if (check == 1){ 
                        //printf("date in between: %s \n",stock_data_one[i].date);
                        //if the current price is greater than the start price sub
                        //printf("stockPriceInArr: %f startPrice: %f \n",stock_data_one[i].close,start_price);
                        if (stock_data_one[i].close < start_price){
                            start_price = stock_data_one[i].close;
                        }
                        if (difference < (stock_data_one[i].close - start_price)){
                            difference = stock_data_one[i].close - start_price;
                        }
                    }
                }
            }
            else{ //for stock array 2
                for(int i = 0; i< MAX_ROWS; i++){
                    //checks if the current date is in range 
                    //printf("date in between: %s \n",stock_data_one[i].date);

                    int check = range(start_date,end_date,stock_data_two[i].date);
                    //printf("check : %d\n", check);
                    if (check == 1){ 
                        //printf("date in between: %s \n",stock_data_one[i].date);
                        //if the current price is greater than the start price sub
                        //printf("stockPriceInArr: %f startPrice: %f \n",stock_data_one[i].close,start_price);
                        if (stock_data_two[i].close < start_price){
                            start_price = stock_data_two[i].close;
                        }
                        if (difference < (stock_data_two[i].close - start_price)){
                            difference = stock_data_two[i].close - start_price;
                        }
                    }
                }
            }

            //if the date is not in the CSV then return unknown to the client
            if(start_price == 3000.0){
                send_message(client_socket, "Unknown"); 
            }
            //printf("startPrice: %f endPrice: %f\n", start_price, end_price);
            //double difference = end_price - start_price;
            char price_str[20];
            snprintf(price_str, sizeof(price_str), "%.2f", difference);

            //get the difference
            send_message(client_socket, price_str); // Replace with actual profit
        }
        else if (strncmp(buffer, "quit", 4) == 0) {
            break;
        }
        else {
            send_message(client_socket, "Invalid syntax");
        }
    }
    
    return 1; //marks that it is done 
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <stock_file_1> <stock_file_2> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    StockData stock_data_one[MAX_ROWS];
    StockData stock_data_two[MAX_ROWS];

    // Read stock files
    read_stock_data(argv[1], stock_data_one);
    read_stock_data(argv[2], stock_data_two);

    int server_socket, client_socket;
    //creates instances of sockaddr_in and putting them into the server and client 
    struct sockaddr_in server_addr, client_addr; 
    socklen_t addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[3]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started\n");

    while (1) {
        //establishes a connection to the client 
        //server socket listens for incoming connection requests  
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Acceptance failed");
            exit(EXIT_FAILURE);
        }
        //handle_client_request(client_socket, stock_data_one, stock_data_two);
        if (handle_client_request(client_socket, stock_data_one, stock_data_two) == 1){
            close(server_socket);
            send_message(client_socket, "Quit");
            break;
        }
    }


    close(server_socket);
    return 0;
}