#include "utils.h"


/**
	2Talk-Project
	@author Georgiana-Liliana Baclava
**/








/*
	Client's structure that must be initialized for attempting afterwards
	the connection with the server.
*/
typedef struct{
	int sockfd, cli_sockfd;
	struct sockaddr_in serv_addr, cli_addr;
} Client, *AClient;





/*
	Initializing the properties of the client, creating a server socket for
	communication and trying to establish a connection with the server.
*/
int initClient(AClient newClient, char *client_port, char *server_ip, char *server_port){


	/* Creating socket for the connection with server */
	newClient->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(newClient->sockfd < 0){
		fprintf(stderr,"ERROR CLIENT: Unable opening socket!");
		return -1;
	}


	/* Server's information */
	newClient->serv_addr.sin_family = AF_INET;
	newClient->serv_addr.sin_port = htons(atoi(server_port));
	inet_aton(server_ip, &(newClient->serv_addr).sin_addr);



	/* Trying to connect to server */
	if(connect(newClient->sockfd, (struct sockaddr*) &(newClient->serv_addr), sizeof(newClient->serv_addr)) < 0){
		fprintf(stderr,"ERROR CLIENT: Unable connecting to server!\n");
		return -1;
	}



	/* Creating a client socket */
	newClient->cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(newClient->cli_sockfd < 0){
		fprintf(stderr,"ERROR CLIENT: Unable opening client socket!");
		return -1;
	}


	/* Client's information */
	newClient->cli_addr.sin_family = AF_INET;
	newClient->cli_addr.sin_port = htons(atoi(client_port));
	newClient->cli_addr.sin_addr.s_addr = INADDR_ANY;



	/* Binding socket's properties */
	if(bind(newClient->cli_sockfd, (struct sockaddr*) &newClient->cli_addr, sizeof(struct sockaddr)) < 0){
		fprintf(stderr,"ERROR CLIENT: Unable in binding!\n");
		return -1;
	}


	/* Adding the current client to the list of the clients that can connect to the server */
	listen(newClient->cli_sockfd, MAX_CLIENTS);

	return 0;
}









/*  
	2Talk_Client_Function => connectToHost

	The client is issuing a request to communicate with the server by sending 
	him his authentication data.
	The response of the server will be accepting or rejecting the 
	connection with the client.
*/
int connectToHost(int sockfd, struct sockaddr_in serv_addr, char name[MAX], char server_name[MAX]){

	char data[MAXLEN], state[MAXLEN];


	/* Sending authentication data to server */
	sprintf(data, "%s", name);
	if(send(sockfd, data, strlen(data), 0) < 0){
		fprintf(stderr,"ERROR CLIENT: Unable writing on socket!\n");
		return -1;
	}

	/* Receive accept or reject form server */
	if(recv(sockfd, state, sizeof(state), 0) <= 0){
		fprintf(stderr,"ERROR CLIENT: Unable reading from socket!\n");
		return -1;
	}

	/* Verifying client's state */
	if(strcmp(state, "Reject") == 0){
		printf("INFO CLIENT: Client rejected!\n");
		return 0;
	}

	else{
		snprintf(server_name,MAX,"%s",state);
		printf("INFO CLIENT: Client connected!\n");
		return 1;
	}
}









/*  
	2Talk_Client_Function => ClientAction

	The client receives commands from console or data from server 
	and performs different operations like changing the current directory,
	sending/receiving a file to/from server.
*/
void ClientAction(int sockfd, char server_name[MAX]){


	/* Declaring the read_fds and tmp_fds clusters */
	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;


	/* Reset the read_fds and tmp_fds clusters */
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);



    /* Adding the file descriptor to read_fds cluster */
    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);

    fdmax = sockfd;
	char command[MAXLEN];



	/* 
		While client is active, he can receive comands from console 
		or data from server, selecting the socket on which he is listening
	*/
	while(1){

		tmp_fds = read_fds;

		/* Select the socket on which he will listen to */
		if(select(fdmax+1, &tmp_fds, NULL, NULL, NULL) == -1){
			fprintf(stderr,"ERROR CLIENT: Unable in select!\n");
			return;
		}



		/* Listening on socket no. 0: The client is receiving commands from console */
		if(FD_ISSET(0, &tmp_fds)){

			while(1){
				if(fgets(command, sizeof(command), stdin) != NULL){
			   		command[strlen(command)-1] = '\0';



			   		/* The client wants to chat with server */
			   		if(strcmp(command, "chat") == 0){
			   			start_Chat(server_name, sockfd, sockfd);
			   			break;
			   		}


			   		/* The client wants to change the current directory */
			   		else if(strstr(command, "cd ") != NULL){
			   			changeDir(command);
			   			break;
			   		}


			   		/* The client wants to send a file to server */
			   		else if(strstr(command, "file-transfer") != NULL){
						char *p, *file;
						p = file = (char *)malloc(MAXLEN * sizeof(char));
			   			file = strtok(command, " ");
			   			file = strtok(NULL, " ");

			   			send_File(file, sockfd, sockfd);
			   			free(p);
			   			break;
			   		}


					/* If the client wants to end the conversation */
					else if(strcmp(command, "quit") == 0){
						/* Sending command to server */
						if(send(sockfd, command, strlen(command), 0) < 0){
							fprintf(stderr,"ERROR CLIENT: Unable writing on socket!\n");
							return;
						}
						printf("INFO CLIENT: Client quitting!\n");
						return;
					}

					else{
						fprintf(stderr,"ERROR CLIENT: Invalid command!\n");
					}
				}
			}
		}


		/* Listening on server's socket: The client is receiving data from server */
		if(FD_ISSET(sockfd, &tmp_fds)){

			memset(command, 0, MAXLEN);

			/* Receiving data from server */
			int n = recv(sockfd, command, sizeof(command), 0);
			
			if(n > 0){
				/* Audio command */
				if(strcmp(command, "audio") == 0){

				}

				/* Video command */
				if(strcmp(command, "video") == 0){

				}


				/* Audio and video command */
				if(strcmp(command, "audio-video") == 0){

				}


				/* Chat command */
				if(strcmp(command, "c") == 0){
					receive_Chat(server_name, sockfd, sockfd);
				}


				/* File transfer command */
				if(strstr(command, "f") != NULL){

					char *file = (char *)malloc(MAXLEN * sizeof(char));
					strcpy(file, strtok(command, " "));
					strcpy(file, strtok(NULL, " "));
					int size = atoi(strtok(NULL, " "));

					receive_File(file, size, sockfd, sockfd);
					free(file);
				}
			}


			/* Checking error in receiving */
			if(n < 0){
				fprintf(stderr,"ERROR CLIENT: Unable receiving data from server!\n");
				close(sockfd);
				FD_CLR(sockfd, &read_fds);
				return;
			}

			/* Server ended the conversation */
			if(n == 0){
				printf("INFO CLIENT: Server is out!\n");
				close(sockfd);
				FD_CLR(sockfd, &read_fds);
				return;
			}
		}
	}
}






/* 
	2Talk_Client_Function => closeConnection

	Function for closing the existing connection.
*/
void closeConnection(int sockfd, int cli_sockfd){
	close(sockfd);
	close(cli_sockfd);
}










/*
	2Talk_Client_Function => main

	In the main function it is created and initialized a new client,
	it is tried a connection with the server and after the communication between them,
	the connection is closed.

*/
int main(int argc, char* argv[]){


	/* How the client must be called */
	if(argc < 5){
		fprintf(stderr,"USAGE CLIENT: %s client_name client_port server_ip server_port\n", argv[0]);
		return -1;
	}


	char server_name[MAX];

	/* Creating a new client */
	AClient newClient = (AClient)malloc(sizeof(Client));



	/* Client initialization */
	if(initClient(newClient, argv[2], argv[3], argv[4]) < 0){
		free(newClient);
		return -1;
	}




	/* Attempting to connect to server */
	if(connectToHost(newClient->sockfd, newClient->serv_addr, argv[1], server_name) <= 0){
		closeConnection(newClient->sockfd, newClient->cli_sockfd);
		free(newClient);
		return -1;
	}



	/* 2Talk function */
	ClientAction(newClient->sockfd, server_name);



	/* Closing the current connection */
	closeConnection(newClient->sockfd, newClient->cli_sockfd);
	free(newClient);

	return 0;

}
