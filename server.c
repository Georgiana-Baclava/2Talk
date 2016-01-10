#include "utils.h"


/**
	2Talk-Project
	@author Georgiana-Liliana Baclava
**/








/*
	Initializing the server, creating a server socket for communication
	and binding the properties of the server.
*/
int initServer(int *sockfd, struct sockaddr_in *serv_addr, char *server_port){

	/* Opening server's socket */
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*sockfd < 0){
		fprintf(stderr,"ERROR SERVER: Unable opening socket!");
		return -1;
	}


	/* Server's information */
	memset((char *) serv_addr, 0, sizeof(*serv_addr));
	(*serv_addr).sin_family = AF_INET;
	(*serv_addr).sin_addr.s_addr = INADDR_ANY;// uses the ip address of the machine
	(*serv_addr).sin_port = htons(atoi(server_port));



	/* Binding socket's properties */
	if (bind(*sockfd, (struct sockaddr *) serv_addr, sizeof(struct sockaddr)) < 0){ 
        fprintf(stderr,"ERROR SERVER: Unable in binding!\n");
        return -1;
    }
    
    listen(*sockfd, MAX_CLIENTS);

    return 0;
}







/*  
	2Talk_Server_Function => ServerAction

	The server receives commands from console or data from client 
	and performs different operations like changing the current directory,
	sending/receiving a file to/from client.
*/
void ServerAction(char name[MAX], int sockfd){

	int cli_sockfd;
	struct sockaddr_in cli_addr;
	
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

    char command[MAXLEN], cli_name[MAX];
    int i;


/* 
	While server is active, he can receive comands from console 
	or data from client, selecting the socket on which he is listening
*/
	while(1){

    	tmp_fds = read_fds; 
		
		/* Select the socket on which he will listen to */
		if(select(fdmax+1, &tmp_fds, NULL, NULL, NULL) == -1){
			fprintf(stderr,"ERROR SERVER: Unable in select!\n");
			return;
		}
	

    	
	
		/* Multiplex */
    	for(i = 0; i <= fdmax; i++){
    		
			if (FD_ISSET(i, &tmp_fds)) {


				/* Listening on socket no. 0: The server is receiving commands from console */
				if(i == 0){
					while(1){
					   	if(fgets(command, sizeof(command), stdin) != NULL){
					   		command[strlen(command)-1] = '\0';


					   		/* The server wants to chat with client */
					   		if(strcmp(command, "chat") == 0){
					   			start_Chat(cli_name, cli_sockfd, cli_sockfd);
					   			break;
					   		}


					   		/* The server wants to change the current directory */
					   		if(strstr(command, "cd ") != NULL){
			   					changeDir(command);
			   					break;
			   				}

			   				/* The server wants to send a file to client */
					   		else if(strstr(command, "file-transfer") != NULL){
								char *p, *file;
								p = file = (char *)malloc(MAXLEN * sizeof(char));
					   			file = strtok(command, " ");
					   			file = strtok(NULL, " ");
					   			
					   			send_File(file, cli_sockfd, cli_sockfd);
					   			free(p);
					   			break;
					   		}

						   	/* If the server wants to end the conversation */
					    	else if(strcmp(command, "quit") == 0){
					    		printf("INFO SERVER: Server quitting!\n");
								return;
							}	

							else{
								fprintf(stderr,"ERROR SERVER: Invalid command!\n");
							}
					    }
					}
				}


				/* Listening on server's socket: New client wants to connect to server */
			    else if (i == sockfd) {

			    	socklen_t clilen = sizeof(cli_addr);

			    	/* Accepting the connection */
					if ((cli_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
						fprintf(stderr,"ERROR SERVER: Unable to accept the current client!\n");
						return;
					}

					else{


						memset(command, 0, MAXLEN);
						if(recv(cli_sockfd, command, sizeof(command), 0) <= 0){
							fprintf(stderr,"ERROR SERVER: Unable reading from socket!\n");
							return;
						}
						
						/* Checking if it is the expected client for connection */
						if(strcmp(command, "Lili") != 0){
							memset(command, 0, sizeof(command));
							sprintf(command, "Reject");
							if(send(cli_sockfd, command, strlen(command), 0) < 0){
								fprintf(stderr,"ERROR SERVER: Unable writing on socket!\n");
								return;
							}	

						}
						else{
							snprintf(cli_name, MAX, "%s", command);
							memset(command, 0, MAXLEN);
							sprintf(command, "%s", name);
							if(send(cli_sockfd, command, strlen(command), 0) < 0){
								fprintf(stderr,"ERROR SERVER: Unable writing on socket!\n");
								return;
							}
							
							printf("INFO SERVER: Client connected!\n");

							/* We add the new client socket returned by accept to the file descriptors reading cluster */
							FD_SET(cli_sockfd, &read_fds);
							if(cli_sockfd > fdmax){
								fdmax = cli_sockfd;
							}

						}
			    	}
				}


				/* Listening on clients's socket: The server is receiving data from client */
			    else{
			    	
			    	memset(command, 0, MAXLEN);

					/* Receiving data from client */
					int n = recv(i, command, sizeof(command), 0);


					/* Checking error in receiving */
					if(n < 0){
						fprintf(stderr,"ERROR CLIENT: Unable receiving data from client!\n");
						close(i);
						FD_CLR(i, &read_fds);
						return;
					}

					/* Client ended the conversation */
					if(n == 0){
						printf("INFO CLIENT: Client is out!\n");
						close(i);
						FD_CLR(i, &read_fds);
						return;
					}

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
							receive_Chat(cli_name, cli_sockfd, i);
						}


						/* File transfer command */
						if(strstr(command, "f") != NULL){

							char *file = (char *)malloc(MAXLEN * sizeof(char));
							strcpy(file, strtok(command, " "));
							strcpy(file, strtok(NULL, " "));
							int size = atoi(strtok(NULL, " "));

							
							/* Sending information to client on cli_sockfd socket
							 and receiving  the file on i socket */
							receive_File(file, size, cli_sockfd, i);
							free(file);
						}



						/* If the client ended the conversation */
						if(strcmp(command, "quit") == 0){
							printf("INFO SERVER: Client is out!\n");
							close(i);
							FD_CLR(i, &read_fds);
							return;
						}
					}

			    }
			}
		}
	}
}






/*
	2Talk_Server_Function => main

	In the main function it is created and initialized the server and
	if the client manages to connect to server, they will comunicate and in the end
	the connection will be closed.

*/
int main(int argc, char *argv[]){

	/* How the server must be called */
	if(argc < 3){
		fprintf(stderr,"USAGE SERVER: %s server_name server_port\n", argv[0]);
		return -1;
	}

	char name[MAX];
	snprintf(name, MAX, "%s", argv[1]);

	int sockfd;
	struct sockaddr_in serv_addr;

	/* Server initialization */
	if(initServer(&sockfd, &serv_addr, argv[2]) < 0){
		return -1;
	} 

    /* 2Talk function */
    ServerAction(name, sockfd);

	/* Closing the existing connection */
	close(sockfd);
	return 0;
}
