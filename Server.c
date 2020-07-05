#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <math.h>

#define BUF_SIZE 30

static char private_key[] = {'1','1','0','1','0','0','0','1'};

void error_handling(char* message);
void dec_to_bin (char* message, char bin_mes[BUF_SIZE][8]);
void bin_to_dec(char bin_mes[BUF_SIZE][8], int n, char* message);
void xor(char* bin);
void receive_message(int sock, char* message);
void send_message(int sock, char* message);

int main(int argc, char* argv[]) {
    int server_socket, client_socket;
    char host_name[25], ip_addr[16];
    int i;
    struct hostent* host;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2) {
        printf("Usage : %s <Port>\n", argv[0]);
        exit(1);
    }
    printf("Waiting for Client...\n");
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        error_handling("socket() Error!!!");
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    if (bind(server_socket, (struct sockaddr*) & serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() Error!!!");
    if (listen(server_socket, 5) == -1)
        error_handling("listen() Error!!!");
    clnt_adr_sz = sizeof(clnt_adr);
    client_socket = accept(server_socket, (struct sockaddr*) & clnt_adr, &clnt_adr_sz);
    printf("Connected to Client!!!\n");
    while (1 > 0) {
        char message[BUF_SIZE];
        for(int i = 0 ; i < BUF_SIZE ; i++)
            message[i] = '\0';
        receive_message(client_socket, message);
        if (!strcmp(message, "q") || !strcmp(message, "Q")) {
            printf("\nClient has Disconnected!!!\n");
            break;
        }
        printf("\nClient: %s", message);
        printf("\nServer (\'q\' to Quit): ");
        fgets(message,BUF_SIZE,stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n")) {
            printf("\nQuitting!!!\n");
            send_message(client_socket, "q");
            break;
        }
        send_message(client_socket,message);
    }
    close(client_socket);
    close(server_socket);
    printf("Operation Terminated!!!\n");
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

void xor(char* bin) {
    for (int i = 0 ; i < 8 ; i++)
        if(bin[i] == private_key[i])
            bin[i] = '1';
        else
            bin[i] = '0';
}

void receive_message(int sock, char* message) {
    //read(client_socket, message, sizeof(message));
    char bin_mes[BUF_SIZE][8];
    int n = 0;
    while (1 > 0) {
        char bin_mes_str[8];
        read(sock, &bin_mes_str, 8);
        if (!strcmp(bin_mes_str, "done")) {
            bin_to_dec(bin_mes,n,message);
            return;
        }
        xor(bin_mes_str);
        write(sock, bin_mes_str, sizeof(bin_mes_str));
        read(sock, &bin_mes_str, sizeof(bin_mes_str));
        xor(bin_mes_str);
        strcpy(bin_mes[n],bin_mes_str);
        n++;
    }
    bin_to_dec(bin_mes,n,message);
}

void send_message(int sock, char* message) {
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

