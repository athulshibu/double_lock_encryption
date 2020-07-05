#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

#define BUF_SIZE 30

static char private_key[] = {'1','0','0','0','1','0','1','0'};

void error_handling(char* message);
void dec_to_bin (char* message, char bin_mes[BUF_SIZE][8]);
void bin_to_dec(char bin_mes[BUF_SIZE][8], int n, char* message);
void xor(char* bin);
void receive_message(int sock, char* message);
void send_message(int sock, char* message);

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_adr;
    if (argc != 3) {
        printf("Usage : %s <IP> <Port>\n", argv[0]);
        exit(1);
    }
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() Error!!!");
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));
    while (1 > 0) {
        char message[25];
        for(int i = 0 ; i < BUF_SIZE ; i++)
            message[i] = '\0';
        printf("\nClient (\'q\' to Quit): ");
        fgets(message,BUF_SIZE,stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n")) {
            printf("\nQuitting!!!\n");
            send_message(sock, "q");
            break;
        }
        while (!connect(sock, (struct sockaddr*) & serv_adr, sizeof(serv_adr)));
        send_message(sock, message);
        for(int i = 0 ; i < BUF_SIZE ; i++)
            message[i] = '\0';
        receive_message(sock, message);
        if (!strcmp(message, "q") || !strcmp(message, "Q")) {
            printf("\nServer has Disconnected!!!\n");
            break;
        }
        printf("\nServer: %s", message);
    }
    printf("Operation Terminated!!!\n");
    close(sock);
    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void dec_to_bin (char* message, char bin_mes[BUF_SIZE][8]) {
    int n = strlen(message);
    int bin_mes_int[BUF_SIZE][8];
    for (int i = 0 ; i < n ; i++) {
        int x = (int)message[i];
        for (int j = 7 ; j >= 0 ; j--) {
            bin_mes_int[i][j] = x % 2;
            x /= 2;
        }
    }
    for (int i = 0 ; i < n ; i++)
        for ( int j = 0 ; j < 8 ; j++) {
            char temp = bin_mes_int[i][j] + 48;
            bin_mes[i][j] = temp;
        }
}

void bin_to_dec(char bin_mes[BUF_SIZE][8], int n, char* message) {
    int bin_mes_int[BUF_SIZE][8];
    for (int i = 0 ; i < n ; i++) 
        for ( int j = 0 ; j < 8 ; j++){ 
            int temp = bin_mes[i][j];
            bin_mes_int[i][j] = temp - 48;
        }
    for (int i = 0 ; i < n ; i++) {
        int temp = 0;
        for ( int j = 0 ; j < 8 ; j++) 
            if(bin_mes_int[i][j] == 1)
                temp += (int)pow(2,7-j);
        char temp_char = temp;
        message[i] = temp_char;
    }
}

void xor(char bin[8]) {
    for (int i = 0 ; i < 8 ; i++)
        if(bin[i] == private_key[i])
            bin[i] = '1';
        else
            bin[i] = '0';
}

void error_handling(char* message);

void receive_message(int client_socket, char* message) {
    //read(client_socket, message, sizeof(message));
    char bin_mes[BUF_SIZE][8];
    int n = 0;
    while (1 > 0) {
        char bin_mes_str[8];
        read(client_socket, &bin_mes_str, sizeof(bin_mes_str));
        if (!strcmp(bin_mes_str, "done")) {
            bin_to_dec(bin_mes,n,message);
            return;
        }
        xor(bin_mes_str);
        write(client_socket, bin_mes_str, sizeof(bin_mes_str));
        read(client_socket, &bin_mes_str, sizeof(bin_mes_str));
        xor(bin_mes_str);
        strcpy(bin_mes[n],bin_mes_str);
        n++;
    }
    bin_to_dec(bin_mes,n,message);
}

void send_message(int sock, char* message) {
    //write(sock, message, sizeof(message));
    char bin_mes[BUF_SIZE][8];
    dec_to_bin (message, bin_mes);
    for(int i = 0 ; i < strlen(message) ; i++) {
        char bin_mes_str[8];
        for(int j = 0 ; j < 8 ; j++)
            bin_mes_str[j] = bin_mes[i][j];
        xor(bin_mes_str);
        write(sock, bin_mes_str, sizeof(bin_mes_str));
        read(sock, &bin_mes_str, sizeof(bin_mes_str));
        xor(bin_mes_str);
        write(sock, bin_mes_str, sizeof(bin_mes_str));
        usleep(100);
    }
    char* temp = "done";
    write(sock, temp, sizeof(temp));
}

