/**
 * @shiyangw_assignment1
 * @author   Shiyang Wang <shiyangw@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../include/global.h"
#include "../include/logger.h"


#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
//init function
void distinguish_command(char cmd[128]);
int connect_to_host(char *server_ip, int server_port, int clientport);
void sortClient();
void addtoList(int fdaccept, int login_port);
int ValidAddressandPort(char *serverIPaddr, char *serverport_str);
int sendall(int socket_fd, char *buf, int *len);
void create_listdata(char *listdata_array);
void displayinlist();
void distinguish_command_client(char cmd[20], int client_socket, int log_status);
void displayinlist_client();
void package_send(char *msg, int sock_filed);
int sendip_in_list(char *sendip);
//define
#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256
#define DNS_SERVER "8.8.8.8"
#define DNS_SERVER_PORT 53
#define TCP 1
#define UDP 2
#define MSG_SIZE 256
#define IN 1
#define OUT 0
#define logged_in 1
#define logged_out 0

//param
int port;
int clientport;
int clientNum;
int type;              //mark server or client
int socket_filed;
struct Node {
    //use getnameinfo to get hostname
    char ip[128];
    int port;
    char hostname[128];
    int sock;
    int status;
    int messageCount;
};
struct Node clientList[5];
struct addrinfo *myaddress;
int client_num = 0;
//client!!!!
int client(char argv[20]) {

    int client_socket, head_socket, selret, sock_index, fdaccept, caddr_len, log_status, clientNum, temp_port = 0;
    char *cmd;
    char *message;
    char *temp[128];
    int loop1 = 0, loop2 = 0; //to loop the buffer
    char *token;
    struct sockaddr_in server_addr, client_addr;
    fd_set master_list, watch_list;
    log_status = OUT;
    clientNum = 0;
    //server = connect_to_host(argv[1], atoi(argv[2]));
    client_socket = socket(AF_INET, SOCK_STREAM, 0);                //Set up client socket.
    if (client_socket < 0)
        perror("Cannot create client socket");

    bzero(&client_addr, sizeof(client_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(port);

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    FD_SET(STDIN, &master_list);

    head_socket = STDIN;
    printf("client_socket:%d\n", client_socket);
    printf("head_socket%d\n", head_socket);
    while (TRUE) {
        memcpy(&watch_list, &master_list, sizeof(master_list));

        fprintf(stdout, "\n[PA1-Client@CSE489/589]$ ");
        fflush(stdout);
        int index_max = head_socket;
        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        //printf("selret:%d\n", selret);
        if (selret < 0)
            perror("select failed.");
        if (selret > 0) {
            int sock_index = 0;
            for (sock_index; sock_index <= index_max; sock_index += 1) {
	        	if(FD_ISSET(sock_index, &watch_list)){
	                //printf("sock_index is:%d\nindex_max is:%d\n", sock_index, index_max);
	                if (sock_index == STDIN) {

	                    char *msg = (char *) malloc(sizeof(char) * MSG_SIZE);
	                    memset(msg, '\0', MSG_SIZE);
	                    if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to msg
	                        exit(-1);

	                    printf("I got: %s\n", msg);

	                    cmd = msg;

	                    if (strstr(cmd, "LOGIN")) {
	                        int count = 0;
	                        char *d_array[10];
	                        char *arg = strtok(cmd, " ");
	                        while (arg) {
	                            d_array[count] = arg;
	                            count++;
	                            arg = strtok(NULL, " ");
	                        }


	                        char *serverIPaddr;
	                        serverIPaddr = d_array[1];
	                        int serverport = atoi(d_array[2]);

	                        client_socket = connect_to_host(serverIPaddr, serverport, clientport);
	                        FD_SET(client_socket, &master_list);
	                        //printf("connect success\n");
	                        if (client_socket != 0) {
	                            cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
	                            index_max = client_socket;
	                        } else
	                            cse4589_print_and_log("[%s:ERROR\n]", "LOGIN");
	                        char serverport_str[5];
	                        char clientport_str[5];
	                        sprintf(serverport_str, "%d", serverport);
	                        sprintf(clientport_str, "%d", clientport);

	                        if (ValidAddressandPort(serverIPaddr, serverport_str)) {
	                            if (log_status == IN) {
	                                if (client_socket != -1) {
	                                    char sendtoserver[16] = "already_in:";

	                                    int sendtoserver_len = strlen(sendtoserver);
	                                    strcat(sendtoserver, clientport_str);
	                                    sendtoserver_len = strlen(sendtoserver);
	                                    if (sendall(client_socket, sendtoserver, &sendtoserver_len) == -1) {
	                                        printf("Failed to send to server! \n");
	                                    }
	                                    //FD_SET(client_socket, &master_list);
	                                    //if (client_socket > head_socket) head_socket = client_socket;

	                                }
	                            } else if (log_status == OUT) {
	                                char sendtoserver[16] = "log_in:";
	                                int sendtoserver_len = strlen(sendtoserver);
	                                strcat(sendtoserver, clientport_str);
	                                sendtoserver_len = strlen(sendtoserver);
	                                if (sendall(client_socket, sendtoserver, &sendtoserver_len) == -1) {
	                                    printf("Failed to send to server! \n");
	                                } else log_status = IN;
	                                FD_SET(client_socket, &master_list);
	                                if (client_socket > head_socket) head_socket = client_socket;
	                            }


	                            cse4589_print_and_log("[%s:END]\n", "LOGIN");
	                        } else {
	                            cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
	                        }

	                    } else if (strstr(cmd, "BROADCAST") && log_status == IN) {
	                        int count = 0;
	                        char *msg;
	                        char *arg = strtok(cmd, " ");
	                        arg = strtok(NULL, " ");
	                        msg = arg;
	                        message = malloc(26 + strlen(msg));
                            memset(message, '\0', 26 + strlen(msg));
	                        strcpy(message, "BROADCAST~255.255.255.255~");
	                        strcat(message, msg);
	                        package_send(message, client_socket);
	                        free(message);
	                    } else if (strstr(cmd, "SEND") && log_status == IN) {
	                        char header[10];
	                        char sendip[64];
	                        char sendipandmessage[308] = {'\0'};
	                        char message_send[256];
	                        int iandm_len;

	                        char *temp = strtok(cmd, " ");
	                        if (temp != NULL) {
	                            strcpy(header, temp);
	                            temp = strtok(NULL, "");
	                        }
	                        strcpy(sendipandmessage, temp);
	                        iandm_len = strlen(sendipandmessage);
	                        strcpy(sendip, sendipandmessage);
	                        char *temp2 = strtok(sendip, " ");
	                        printf("sendip:%s\n", sendip);
	                        printf("send header:%s\n", header);
	                        printf("sendipandmessage%s\n", sendipandmessage);
	                        if (sendip_in_list(sendip) > 0) {
	                            printf("The destination client is in the list.\n");
	                            if (sendall(client_socket, sendipandmessage, &iandm_len) == -1) {
	                                //perror("sendall");
	                                printf("Failed to send to server!");
	                            }
	                            cse4589_print_and_log("[%s:SUCCESS]\n", "SEND");
	                        } else {
	                            cse4589_print_and_log("[%s:ERROR]\n", "SEND");
	                        }
	                        cse4589_print_and_log("[%s:END]\n", "SEND");
	                    }
                        else if(!strncmp(cmd,"REFRESH",7)){
							message = malloc(10);
							//strcpy(message,cmd);
							message = cmd;
							//printf("message is \n",message);
							package_send(message,client_socket);
							free(message);
                        }
                        else
	                        distinguish_command_client(cmd, client_socket, log_status);
	                    //fflush(stdout);
	                } else if (sock_index == client_socket) {
	                    /* Initialize buffer to receieve response */
	                    char *buffer = (char *) malloc(sizeof(char) * BUFFER_SIZE);
	                    memset(buffer, '\0', BUFFER_SIZE);

	                    if (recv(client_socket, buffer, BUFFER_SIZE, 0) > 0) {
	                        printf(" buffer is %s\n", buffer);
	                        if (strncmp(buffer, "listdata~", 9) == 0) {
                                loop1=0;
                                loop2=0;
                                int c_num=0;
	                            token = strtok(buffer, "\n");
	                            printf("token is %s\n", token);
	                            while (token != NULL) {
	                                temp[c_num] = token;
	                                printf("temp[%d] = %s\n",c_num,temp[c_num]);
	                                c_num++;
	                                token = strtok(NULL, "\n");
	                            }
	                            //clientList[clientNum].port = port;
	                            //printf("clientNum is %d\n",clientNum);
	                            for (loop1; loop1 < c_num; loop1++) {
	                                token = strtok(temp[loop1], " ");
	                                token = strtok(NULL, " ");
	                                //printf("token = %s\n",token);
	                                //printf("for loop1 = %d\n",loop1);
	                                switch (loop2) {
	                                    case 0:
	                                        strcpy(clientList[loop1].hostname, token);
	                                        //printf("[%d]hostname = %s\n",loop1,clientList[loop1].hostname);
	                                        loop2++;
	                                    case 1:
	                                        token = strtok(NULL, " ");
	                                        strcpy(clientList[loop1].ip, token);
	                                        //printf("[%d]ip = %s\n",loop1,clientList[loop1].ip);
	                                        loop2++;
	                                    case 2:
	                                        token = strtok(NULL, " ");
	                                        //printf("in case2 token is %s\n",token);
	                                        temp_port = atoi(token);
	                                        //printf("temp_port is %d\n",temp_port);
	                                        clientList[loop1].port = temp_port;
	                                        loop2 = 0;

	                                }
	                            }
	                        }
                            else if (strstr(buffer, "BROADCAST")){
                                printf("buffer:%s\n", buffer);
                                char *sender_ip=NULL;
                                char *received_message=NULL;
                                sender_ip=strtok(buffer," ");
                                sender_ip=strtok(NULL," ");
                                received_message=strtok(NULL,"");
                                //printf("receive message from:%s\n",sender_ip);
                                //printf("message:%s\n", received_message);
                                cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                               cse4589_print_and_log("msg from:%s\n[msg]:%s\n", sender_ip, received_message);
                               cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                            }

                            else {
	                            printf("buffer:%s\n", buffer);
                                char *sender_ip=NULL;
                                char *received_message=NULL;
                                sender_ip=strtok(buffer," ");
                                received_message=strtok(NULL,"");
                                //printf("receive message from:%s\n",sender_ip);
                                //printf("message:%s\n", received_message);
                                cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
	                            cse4589_print_and_log("msg from:%s\n[msg]:%s\n", sender_ip, received_message);
	                            cse4589_print_and_log("[%s:END]\n", "RECEIVED");
	                        }

	                    }
	                    //printf("Server responded: %s", buffer);
	                    fflush(stdout);
	                    //remove the end of LOGIN
	                }
                    /*else{
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);
                        //printf("kazhele1\n");
                        /*if(recv(client_socket, buffer, BUFFER_SIZE, 0) >= 0){
                            //printf("kazhele2\n");
                            if(!strncmp(buffer,"BOARDCAST",9))
                            //printf("kazhele3\n");
                                printf("I recv %s\n",buffer);
                            }

                    }*/

                }
            }  //for for(int socket_index=0; socket_index<=max_fd; socket_index+=1)
        }   //for if(select_result > 0)
    }      //for while

    return 0;

}

int connect_to_host(char *server_ip, int server_port, int clientport) {
    int fdsocket, len;
    struct sockaddr_in remote_server_addr, client;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (fdsocket < 0)
        perror("Failed to create socket");
    /*client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    client.sin_port = htons(clientport);

    if(bind(fdsocket,(struct sockaddr *)&client, sizeof(client)) < 0)
            perror("Bind failed!");*/

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);

    if (connect(fdsocket, (struct sockaddr *) &remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");

    return fdsocket;
}


//server!!!!!
int server(int argv) {
    int server_socket, head_socket, selret, sock_index, fdaccept = 0, caddr_len, c_number = 0;
    char *message;
    char *temp_token;
    char *msg;
    char *temp_str;
    struct sockaddr_in server_addr, client_addr;
    fd_set master_list, watch_list;
    //socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        perror("Cannot create socket\n");

    //fill up sockaddr
    port = argv;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    //bind
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        perror("Bind failed");

    //listen
    if (listen(server_socket, BACKLOG) < 0)
        perror("Unable to listen on port");
    ////////////////////////////////////////////////////////////////////////
    //zero select FD sets
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    //Add a socket
    FD_SET(server_socket, &master_list);
    //Add STDIN
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;


    while (TRUE) {
        memcpy(&watch_list, &master_list, sizeof(master_list));

        //printf("\n[PA1-Server@CSE489/589]$ ");
        //fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if (selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if (selret > 0) {
            /* Loop through socket descriptors to check which ones are ready */
            for (sock_index = 0; sock_index <= head_socket; sock_index += 1) {

                if (FD_ISSET(sock_index, &watch_list)) {

                    /* Check if new command on STDIN */
                    if (sock_index == STDIN) {
                        char *cmd = (char *) malloc(sizeof(char) * CMD_SIZE);

                        memset(cmd, '\0', CMD_SIZE);
                        if (fgets(cmd, CMD_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to cmd
                            exit(-1);

                        printf("\nI got: %s\n", cmd);

                        //Process PA1 commands here ...
                        //Use a function to distinguish command and run them
                        distinguish_command(cmd);
                        //printf("ip is %s\n",get_host_ip_addr());
                        free(cmd);
                    }
                        /* Check if new client is requesting connection */
                    else if (sock_index == server_socket) {
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *) &client_addr, &caddr_len);
                        if (fdaccept < 0)
                            perror("Accept failed.");

                        printf("\nRemote Host connected!\n");
                        clientList[c_number].sock = fdaccept;
                        c_number += 1;
                        client_num += 1;
                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if (fdaccept > head_socket)
                            head_socket = fdaccept;
                    }
                        /* Read from existing clients */
                    else {
                        /* Initialize buffer to receieve response */
                        char *buffer = (char *) malloc(sizeof(char) * BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if (recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0) {
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");
                        /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        } else {
                            //Process incoming data from existing clients here ...
                            int login_port = 0;
                            printf("\nClient sent me: %s\n", buffer);
                            if (strncmp(buffer, "already_in", 10) == 0) {
                                printf("This client has already logged into the server.");
                            } else if (strncmp(buffer, "log_in", 6) == 0) {
                                char loginfor[24];
                                strcpy(loginfor, buffer);
                                char *login_port_str;
                                login_port_str = strtok(loginfor, ":");
                                printf("First part: %s\n", login_port_str);
                                login_port_str = strtok(NULL, "\n");
                                printf("Receiving the port number: %s\n", login_port_str);
                                login_port = atoi(login_port_str);
                                memset(buffer, '\0', BUFFER_SIZE);
                                addtoList(fdaccept, login_port);
                                sortClient();

                                char clientlist_data[512] = {'\0'};
                                create_listdata(clientlist_data);

                                char *clientlist_data_str;
                                clientlist_data_str = clientlist_data;
                                int clientlist_data_len = strlen(clientlist_data_str);
                                if (sendall(fdaccept, clientlist_data_str, &clientlist_data_len) == -1) {
                                    printf("Failed to send to Client!\n");
                                }
                            } else if (strncmp(buffer, "BROADCAST", 9) == 0) {
                                msg = malloc(256);
                                int i = 0;
                                //printf("c_number is %d\n",c_number);
                                temp_token = strtok(buffer, "~");
                                //printf("temptoken is %s\n",temp_token);
                                //strcpy(temp_str,temp_token);
                                temp_str = temp_token;
                                //printf("tempstr is %s\n",temp_str);
                                strcat(msg, temp_str);
                                //printf("msg is %s\n",msg);
                                strcat(msg, " ");
                                //printf("temp_token = %s\n",temp_token);
                                temp_token = strtok(NULL, "~");
                                //strcpy(temp_str,temp_token);
                                temp_str = temp_token;
                                strcat(msg, temp_str);
                                strcat(msg, " ");
                                //printf("temp_token = %s\n",temp_token);
                                temp_token = strtok(NULL, "~");
                                //strcpy(temp_str,temp_token);
                                temp_str = temp_token;
                                strcat(msg, temp_str);
                                strcat(msg, " ");
                                //printf("temp_token = %s\n",temp_token);
                                for (i; i < c_number; i++) {
                                    message = malloc(24 + strlen(msg) * sizeof(char));
                                    strcpy(message, msg);
                                    printf("message is %s\n", message);
                                    //if(send(clientList[i].sock,message,strlen(message),0)<=0)
                                    //printf("Broadcast send failed\n");
                                    if(clientList[i].sock!=sock_index){
                                        printf("sock_index is %d     sock is %d\n",sock_index,clientList[i].sock);
                                        package_send(message, clientList[i].sock);
                                    }
                                    //printf("clientList[%d].sock = %d\n",i,clientList[i].sock);
                                }
                                free(message);
                                free(msg);
                                printf("I know to BROADCAST\n");
                                //printf("I recv %s\n", buffer);

                            }
                            else if(strncmp(buffer,"REFRESH",7) == 0){
								char clientlist_data[512]={'\0'};
                                create_listdata(clientlist_data);
                                printf("data is %s\n",clientlist_data);
                            	char *clientlist_data_str;
								//strcpy(clientlist_data_str,"REFRESH~");
								//strcpy(clientlist_data_str,clientlist_data);
                                //clientlist_data_str = malloc(512);
                                //sprintf(clientlist_data_str,"%s",clientlist_data);
								clientlist_data_str=clientlist_data;
								printf("clientlist_data_str is %s\n",clientlist_data_str);

                            	int clientlist_data_len=strlen(clientlist_data_str);
                            	if(sendall(sock_index,clientlist_data_str,&clientlist_data_len)==-1){
                                    printf("Failed to send to refresh Client!\n");
								}

								//package_send(clientlist_data_str,sock_index);
							}



                            else{
                                char receiver_ip[64];
                                char sender_ip[64];
                                int k = 0;
                                int ip_len, len;
                                int receiver_sock = -1;
                                char message[256];
                                while (strlen(clientList[k].ip) != 0) {
                                    if (clientList[k].sock == sock_index) {
                                        strcpy(sender_ip, clientList[k].ip);
                                    }
                                    k++;
                                }
                                strcpy(receiver_ip, buffer);
                                char *temp = strtok(receiver_ip, " ");
                                //printf("receiver_ip:%s\n", receiver_ip);       //ip
                                //printf("temp:%s\n", temp);                    //ip
                                //printf("buffer:%s\n", buffer);                   //ip and message
                                ip_len = strlen(receiver_ip);
                                len = ip_len + 1;
                                strcpy(message, buffer + len);
                                //printf("message:%s\n", message);                //right
                                k = 0;
                                while (strlen(clientList[k].ip) != 0) {
                                    if (strcmp(receiver_ip, clientList[k].ip) == 0) {
                                        receiver_sock = clientList[k].sock;
                                    }
                                    k++;
                                }
                                char sendtoreceiver[256];
                                char *sendmessage = (char *) malloc(sizeof(char) * BUFFER_SIZE);
                                memset(sendmessage, '\0', BUFFER_SIZE);
                                sprintf(sendtoreceiver, "%s%s%s", sender_ip, " ", message);
                                printf("sendtoreceiver:%s\n", sendtoreceiver);
                                sendmessage = sendtoreceiver;
                                int message_len = strlen(sendmessage);
                                printf("sendmessage%s\n", sendmessage);
                                // if add receive/send count, add here.
                                if (receiver_sock != -1) {
                                    if (sendall(receiver_sock, sendmessage, &message_len) == -1)
                                        printf("Failed to send to receiver!\n");
                                    else {
                                        cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
                                        cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", sender_ip, receiver_ip,message);
                                        cse4589_print_and_log("[%s:END]\n", "RELAYED");
                                    }
                                } else {
                                    printf("Can not find the socket!");
                                }


                            }
                            //printnewclient();



                            printf("ECHOing it back to the remote host ... ");
                            if (send(sock_index, buffer, strlen(buffer), 0) == strlen(buffer))
                                printf("Done!\n");
                            fflush(stdout);
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    printf("this is server\n");
    return 0;
}


int main(int argc, char **argv) {
    /*Init. Logger*/
    cse4589_init_log(argv[2]);

    /*Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));
    if (argc != 3) {
        printf("Usage:%s [s/c] [port]\n", argv[0]);
        exit(1);
    }
    //headnode->next=NULL;
    //Judgh the second para is server or client
    if ((strcmp(argv[1], "s") == 0) || (strcmp(argv[1], "c") == 0)) {
        port = atoi(argv[2]);
        clientport = atoi(argv[2]);
        //jump into server or client
        if ((strcmp(argv[1], "s") == 0)) {
            //jump into server
            server(port);
            type = 1;    //type=1,server
        } else {
            //jump into client
            client(port);
            type = 2;    //type=2,client
        }
    } else {
        printf("Usage2:%s [s/c] [port]\n", argv[0]);
        exit(1);
    }

    /*Start Here*/

    return 0;
}


//get ip address
char *get_host_ip_addr() {
    int sockfd;
    struct sockaddr_in myAddress;
    struct sockaddr_in serverAddress;
    //Create UDP Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
    }

    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, DNS_SERVER, &serverAddress.sin_addr.s_addr);
    //serverAddress.sin_addr.s_addr = htonl(DNS_SERVER);
    serverAddress.sin_port = htons(DNS_SERVER_PORT);

    if (connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to connect to DNS_SERVER");
    }

    socklen_t l = INET_ADDRSTRLEN;

    getsockname(sockfd, (struct sockaddr *) &myAddress, &l);
    char *ip;
    ip = inet_ntoa(myAddress.sin_addr);
    //printf("IP....%s /n",ip);
    close(sockfd);
    return ip;
}

//this is a function to distinguish cmd and run them
void distinguish_command(char cmd[20]) {
    char ip[128];
    char your_ubit_name[20];
    int l;
    strcpy(your_ubit_name, "shiyangw");
    l = strlen(cmd);
    if (cmd[l - 1] == '\n') {
        cmd[l - 1] = '\0';
    }
    if (!strncmp(cmd, "AUTHOR", 6)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",
                              your_ubit_name);
        cse4589_print_and_log("[%s:END]\n", cmd);
    } else if (!strncmp(cmd, "IP", 2)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("IP:%s\n", get_host_ip_addr());
        cse4589_print_and_log("[%s:END]\n", cmd);
    } else if (!strncmp(cmd, "PORT", 4)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("PORT:%d\n", port);
        cse4589_print_and_log("[%s:END]\n", cmd);
    } else if (!strncmp(cmd, "LIST", 4)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        displayinlist();
        cse4589_print_and_log("[%s:END]\n", cmd);
    }

}

void distinguish_command_client(char cmd[20], int client_socket, int log_status) {
    char ip[128];
    char your_ubit_name[20];
    int l;
    strcpy(your_ubit_name, "shiyangw");
    l = strlen(cmd);
    if (cmd[l - 1] == '\n') {
        cmd[l - 1] = '\0';
    }
    if (!strncmp(cmd, "AUTHOR", 6)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",
                              your_ubit_name);
        cse4589_print_and_log("[%s:END]\n", cmd);
    } else if (!strncmp(cmd, "IP", 2)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("IP:%s\n", get_host_ip_addr());
        cse4589_print_and_log("[%s:END]\n", cmd);
    } else if (!strncmp(cmd, "PORT", 4)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("PORT:%d\n", port);
        cse4589_print_and_log("[%s:END]\n", cmd);
    } else if (!strncmp(cmd, "LIST", 4)) {
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        //printf("%-35s%-20s%-8d\n", clientList[1].hostname, clientList[1].ip, clientList[1].port);
        displayinlist_client();
        cse4589_print_and_log("[%s:END]\n", cmd);
    } else if (!strncmp(cmd, "EXIT", 4)) {
        cse4589_print_and_log((char *) "[%s:SUCCESS]\n", cmd);
        close(client_socket);
        cse4589_print_and_log((char *) "[%s:END]\n", cmd);
        exit(0);
    } else if (!strncmp(cmd, "LOGOUT", 6)) {
        cse4589_print_and_log((char *) "[%s:SUCCESS]\n", cmd);
        close(client_socket);
        cse4589_print_and_log((char *) "[%s:END]\n", cmd);
        exit(0);
    }

}


void sortClient() {
    struct Node clientnode;
    int i = 1;
    for (i; i < clientNum; i++) {
        int j = 0;
        while (j < i) {
            if (clientList[i].port < clientList[j].port) {
                clientnode = clientList[i];
                clientList[i] = clientList[j];
                clientList[j] = clientnode;
            }
            j++;
        }
    }
}

void addtoList(int fdaccept, int login_port) {
    struct sockaddr_in newaddr;
    int addr_size = sizeof(struct sockaddr_in);
    getpeername(fdaccept, (struct sockaddr *) &newaddr, &addr_size);
    char *IP_new = inet_ntoa(newaddr.sin_addr);
    if (!inet_aton(IP_new, &newaddr)) {
        printf("Error in getting new Client IP!");
    }
    struct hostent *hostnew = gethostbyaddr(&newaddr, strlen(IP_new), AF_INET);

    clientList[clientNum].sock = fdaccept;
    strcpy(clientList[clientNum].ip, IP_new);
    clientList[clientNum].port = login_port;
    clientList[clientNum].status = logged_in;
    if (hostnew->h_name == NULL) {
        strcpy(clientList[clientNum].hostname, "host");
    } else {
        strcpy(clientList[clientNum].hostname, hostnew->h_name);
    }
    clientList[clientNum].messageCount = 0;
    clientNum++;
    printf("The number of clients is:%d\n", clientNum);

}

int sendall(int socket_fd, char *buf, int *len) {
    int s, whole;                //already sent bytes
    int leftbytes = *len;      //not sent bytes
    for (whole = 0; whole < *len; whole++) {
        s = send(socket_fd, buf + whole, leftbytes, 0);
        if (s < 0)
            break;
        leftbytes = leftbytes - s;
    }
    if (s >= 0) {
        return 1;
    } else return -1;

}

void package_send(char *msg, int sock_filed) {
    int msglen;
    char *message = (char *) malloc(10 + strlen(msg));
    //sprintf(message,"%5lu",strlen(msg));
    strcat(message, msg);
    msglen = strlen(message);
    sendall(sock_filed, message, &msglen);
    free(message);
}

int ValidAddressandPort(char *serverIPaddr, char *serverport_str) {
    struct sockaddr_in addr;
    int IP = 0;
    if (inet_pton(AF_INET, serverIPaddr, &(addr.sin_addr)) > 0) {
        IP = inet_pton(AF_INET, serverIPaddr, &(addr.sin_addr));
        printf("Valid IP address:%d \n", IP);
    }
    //else if printf("invalid IP address.")
    int port = 1;
    int length = strlen(serverport_str);
    int i = 0;
    //printf("test3\n");             right
    while (i < length - 1) {
        if (isdigit(serverport_str[i]) == 0) {
            //printf("invalid port number digit at %d\n", i);
            port = -1;
        }
        i = i + 1;
    }
    //printf("port:%d\n",port);    right
    if ((IP > 0) && (port > 0)) {
        //printf("Good!\n");    right
        return 1;
    } else
        return -1;
}

void create_listdata(char *listdata_array){
    int clientNum=0;
    int i=0;
	int temp_port;
	char temp_array[256];
    //strcpy(listdata_array,"listdata~\n");
    //while(strlen(clientList[clientNum].ip)!=0){
	while(clientNum<client_num){
			printf("client_num is %d\n",client_num );
			printf("clientNum is %d\n",clientNum );
    	 char port_str[8];
    	 //char port_str2[5];
    	 int temp_port=clientList[clientNum].port;
         printf("temp_port is %d\n",temp_port);
         //sprintf(port_str,"%d",temp_port);
		 //printf("port_str is %s", port_str);
         //strcpy(port_str2,port_str);
         //int index=clientNum+1;
         //char index_str[1];
         //char index_str2[1];
         //sprintf(index_str, "%d", index);
         //printf("port_str kazhu1");
         //strcpy(index_str2,index_str);
         printf("port_str kazhu2\n");
		 /*strcat(listdata_array,index_str2);
         strcat(listdata_array," ");
         strcat(listdata_array,clientList[clientNum].hostname);
         strcat(listdata_array," ");
         strcat(listdata_array,clientList[clientNum].ip);
         strcat(listdata_array," ");
         strcat(listdata_array,port_str);
         strcat(listdata_array,"\n");
		 //printf("listdata is %s\n",listdata_array);*/
		sprintf(temp_array, "%s%s%s%s%s%s%d%s", "listdata~"," ", clientList[clientNum].hostname, " ",clientList[clientNum].ip, " ", temp_port, "\n");
        printf("port_str kazhu3\n");
		strcat(listdata_array,temp_array);
        printf("listdata is %s\n",listdata_array);
        clientNum=clientNum+1;
    }
	//printf("at last listdata is %s\n",listdata_array);
}

void displayinlist() {
    int i = 0;
    int index;
    while (strlen(clientList[i].ip) != 0) {
        index = i + 1;
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", index, clientList[i].hostname, clientList[i].ip,clientList[i].port);
        i++;
    }
}

void displayinlist_client() {
    int i = 0;
    while (strlen(clientList[i].ip) != 0) {
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i + 1, clientList[i].hostname, clientList[i].ip,clientList[i].port);
        i++;
    }
}

int sendip_in_list(char *sendip) {
    int i = 0;
    int in = 0;
    while (strlen(clientList[i].ip) != 0) {
        if (strcmp(sendip, clientList[i].ip) == 0)
            in = 1;
        i++;
    }
    return in;
}
