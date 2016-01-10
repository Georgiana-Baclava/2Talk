#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <dirent.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 1
#define MAX 256
#define MAXLEN 1024


/**
	2Talk-Project
	@author Georgiana-Liliana Baclava
**/




void start_Chat(char recv_name[MAX], int send_sockfd, int recv_sockfd){

    	/* Declaring the read_fds and tmp_fds clusters */
    	fd_set read_fds;
    	fd_set tmp_fds;
    	int fdmax;


    	/* Reset the read_fds and tmp_fds clusters */
    	FD_ZERO(&read_fds);
    	FD_ZERO(&tmp_fds);



    	/* Adding the file descriptor to read_fds cluster */
    	FD_SET(send_sockfd, &read_fds);
    	FD_SET(0, &read_fds);

	fdmax = send_sockfd;

	char command[MAXLEN];

	/* Informing the receiver about starting a chat with him */
	sprintf(command, "c");
	if(send(send_sockfd, command, strlen(command), 0) < 0){
		fprintf(stderr,"ERROR: Unable writing on socket!\n");
		return;
	}

	/* Receiving response from recipient */
	memset(command, 0, MAXLEN);
	if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
		fprintf(stderr,"ERROR: Unable reading from socket!\n");
		return;
	}
	if(strcmp(command, "ok") != 0){
		return;
	}


	while(1){
	
		tmp_fds = read_fds;

		/* Select the socket on which he will listen to */
		if(select(fdmax+1, &tmp_fds, NULL, NULL, NULL) == -1){
			fprintf(stderr,"ERROR CLIENT: Unable in select!\n");
			return;
		}



		/* Listening on socket no. 0: The client is receiving commands from console */
		if(FD_ISSET(0, &tmp_fds)){
			memset(command, 0, MAXLEN);
		
			if(fgets(command, sizeof(command), stdin) != NULL){
				command[strlen(command)-1] = '\0';

				if(send(send_sockfd, command, strlen(command), 0) < 0){
					fprintf(stderr,"ERROR: Unable writing on socket!\n");
					return;
				}

				if(strcmp(command,"exit") == 0){
					return;
				}
			}
		}

		/* Listening on server's socket: The client is receiving data from server */
		if(FD_ISSET(send_sockfd, &tmp_fds)){

			memset(command, 0, MAXLEN);

			/* Receiving data from server */
			if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
				fprintf(stderr,"ERROR: Unable reading from socket!\n");
				return;
			}

			if(strcmp(command,"exit") == 0)
				return;

			
			printf("%s: %s\n", recv_name, command);

		}
	}
}



void receive_Chat(char send_name[MAX], int send_sockfd, int recv_sockfd){
	/* Declaring the read_fds and tmp_fds clusters */
	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;


	/* Reset the read_fds and tmp_fds clusters */
    	FD_ZERO(&read_fds);
   	FD_ZERO(&tmp_fds);



    	/* Adding the file descriptor to read_fds cluster */
    	FD_SET(send_sockfd, &read_fds);
    	FD_SET(0, &read_fds);

	fdmax = send_sockfd;

	char command[MAXLEN];

	sprintf(command, "ok");
	if(send(send_sockfd, command, strlen(command), 0) < 0){
		fprintf(stderr,"ERROR: Unable writing on socket!\n");
		return;
	}

	printf("INFO: Chat activated!\n");

	while(1){
	
		tmp_fds = read_fds;

		/* Select the socket on which he will listen to */
		if(select(fdmax+1, &tmp_fds, NULL, NULL, NULL) == -1){
			fprintf(stderr,"ERROR CLIENT: Unable in select!\n");
			return;
		}



		/* Listening on socket no. 0: The client is receiving commands from console */
		if(FD_ISSET(0, &tmp_fds)){
			memset(command, 0, MAXLEN);
		
			if(fgets(command, sizeof(command), stdin) != NULL){
				command[strlen(command)-1] = '\0';

				if(send(send_sockfd, command, strlen(command), 0) < 0){
					fprintf(stderr,"ERROR: Unable writing on socket!\n");
					return;
				}

				if(strcmp(command,"exit") == 0){
					return;
				}
			}
		}

		/* Listening on server's socket: The client is receiving data from server */
		if(FD_ISSET(send_sockfd, &tmp_fds)){

			memset(command, 0, MAXLEN);

			/* Receiving data from server */
			if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
				fprintf(stderr,"ERROR: Unable reading from socket!\n");
				return;
			}

			if(strcmp(command,"exit") == 0)
				return;

			
			printf("%s: %s\n", send_name, command);

		}
	}

}






/* 
	Function for changing the current directory.
	Uses the predefined C function, chdir.
*/
void changeDir(char *command){

	char *p, *dir;
	p = dir = (char *)malloc(MAXLEN * sizeof(char));
	dir = strtok(command, " ");
	dir = strtok(NULL, " ");

	if(chdir(dir) == -1)
		fprintf(stderr,"ERROR: Unable changing directory!\n");

	free(p);
}





/* 
	Function for sending a file from the local file system.
	Opens the specified file, sends it's size and after that sends
	parts of MAXLEN size to the recipient.
*/
void send_File(char *file, int send_sockfd, int recv_sockfd){

	char command[MAXLEN];
	int size = 0;

	/* Opening the file */
	FILE *f = fopen(file, "rb");
	if(!f){
		fprintf(stderr,"ERROR: Unable opening the specified file!\n");
		return;
	}

	/* Finding the size of the file */
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	/* Sending information to recipient that a file begins to be transferred */
	snprintf(command, MAXLEN, "f");
	snprintf(command+strlen(command), MAXLEN, " ");
	snprintf(command+strlen(command), MAXLEN, "%s", file);
	snprintf(command+strlen(command), MAXLEN, " ");
	snprintf(command+strlen(command), MAXLEN, "%d", size);   			

	if(send(send_sockfd, command, strlen(command), 0) < 0){
		fprintf(stderr,"ERROR: Unable writing on socket!\n");
		return;
	}


	/* Receiving response from recipient */
	memset(command, 0, MAXLEN);
	if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
		fprintf(stderr,"ERROR: Unable reading from socket!\n");
		return;
	}
	if(strcmp(command, "Accept") != 0){
		return;
	}

	int i = 0, no = size/MAXLEN, rest = size%MAXLEN;
	memset(command, 0, MAXLEN);


	/* Sending parts of the file */
	for(i = 0; i < no; i++){
					
		fread(command, 1, MAXLEN, f);
						
		/* Sending a part of the file to recipient */
		if(send(send_sockfd, command, MAXLEN, 0) < 0){
			fprintf(stderr,"ERROR: Unable writing on socket!\n");
			return;
		}

		memset(command, 0, MAXLEN);
		if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
			fprintf(stderr,"ERROR: Unable reading from socket!\n");
			return;
		}
					
		memset(command, 0, MAXLEN);
	}

					
	/* Sending the rest from the file to recipient */
	fread(command, 1, rest, f);

	if(send(send_sockfd, command, rest, 0) < 0){
		fprintf(stderr,"ERROR: Unable writing on socket!\n");
		return;
	}

	memset(command, 0, MAXLEN);
	if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
		fprintf(stderr,"ERROR: Unable reading from socket!\n");
		return;
	}

	if(strcmp(command, "Done!") == 0){
		printf("INFO: File transferred successfully!\n");
	}
	else{
		fprintf(stderr,"ERROR: Unable transferring the file!\n");
	}

	memset(command, 0, MAXLEN);

	fclose(f);
}





/* 
	Function for receiving a file on the local file system.
	Receives the size of the file, copies a file on the local machine
	using the name and the content given by the sender.
	
*/
void receive_File(char *file, int size, int send_sockfd, int recv_sockfd){

	
	char command[MAXLEN];
							
	/* Accepting the file transmitted */
	snprintf(command, MAXLEN, "Accept");

	if(send(send_sockfd, command, strlen(command), 0) < 0){
		fprintf(stderr,"ERROR: Unable writing on socket!\n");
		return;
	}

	/* Creating a new file based on the initial name of the file transmitted */		
	char new_file[MAXLEN];
	snprintf(new_file, MAXLEN, "new_");
	strcat(new_file, file);
							
	FILE *f = fopen(new_file, "wb");
	if(!f){
		fprintf(stderr,"ERROR: Unable creating new file!\n");
		return;
	}


	int i, no = size/MAXLEN, rest = size%MAXLEN;
	memset(command, 0, MAXLEN);


	/* Receiving parts of the file */
	for(i = 0; i < no; i++){
				
		/* Receiving and writing one part of a file */		
		if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
			fprintf(stderr,"ERROR: Unable reading from socket!\n");
			return;
		}
		fwrite(command, 1, MAXLEN, f);
					
		/* Sending message for continuing the transfer */				
		memset(command, 0, MAXLEN);
		sprintf(command, "Continue");
		if(send(send_sockfd, command, strlen(command), 0) < 0){
			fprintf(stderr,"ERROR SERVER: Unable writing on socket!\n");
			return;
		}
		memset(command, 0, MAXLEN);
	}


	/* Receiving and writing last part of a file */
	if(recv(recv_sockfd, command, sizeof(command), 0) <= 0){
		fprintf(stderr,"ERROR: Unable reading from socket!\n");
		return;
	}
	fwrite(command, 1, rest, f);

	/* Finalizing the transfer */
	memset(command, 0, MAXLEN);
	sprintf(command, "Done!");
	if(send(send_sockfd, command, strlen(command), 0) < 0){
		fprintf(stderr,"ERROR SERVER: Unable writing on socket!\n");
		return;
	}

	printf("INFO: File received successfully!\n");

	memset(command, 0, MAXLEN);

	fclose(f);
}
