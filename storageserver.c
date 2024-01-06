#include "headers.h"


int ns_sock, ns_port;
char working_directory[4096];
int client_port;
int server_sock;
struct ThreadArgs {
	int clientSocket;
};

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

// the following code is written with reference from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C
// this link was provided under resources in MP2
int init(int port){
	printf(BLUE"[+] Initialising socket for connection to name server...\n"END);
	
	char *ip = "127.0.0.1";
	char buffer[4096];
	
	int server_sock;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_size;
	
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0){
		printf(RED"Socket Error: %s"END, strerror(errno));
		return(1);
	}
	
	// set SO_REUSEADDR option
	// https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/ mentions it as a good practice and the following function was taken from MP1
	int reuseaddr = 1;
	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		fprintf(stderr, RED"Setsockopt failed"END);
		close(server_sock);
		exit(1);
	}
	
	printf(GREEN"[+] Socket created\n"END);
	
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	
	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		printf(RED"Bind Error: %s"END, strerror(errno));
		return(1);
	}
	
	printf(YELLOW"[+] Binded to the port number: %d\n"END, port);
	printf(BLUE"[+] Initialising connection to name server...\n"END);
	
	if(listen(server_sock, 1) != 0){
		printf(RED"Error listening: %s"END, strerror(errno));
		return(1);
	}
	
	printf(YELLOW"\nProvide the following port number to name server: %d\n\n"END, port);
	printf(BLUE"[+] Listening for name server...\n"END);
	
	addr_size = sizeof(client_addr);
	ns_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
	if(ns_sock < 0){
		printf(RED"Error while accepting name server connection"END);
		return(1);
	}
	
	printf(GREEN"[+] Name server connected\n"END);
	
	bzero(buffer, 4096);
	if(recv(ns_sock, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving request"END);
		close(ns_sock);
		return(1);
	}
	printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, "HI, THIS IS STORAGE SERVER");
	if(send(ns_sock, buffer, strlen(buffer), 0) < 0){
		printf(RED"Socket Error"END);
		close(ns_sock);
		exit(1);
	}
	printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	if(recv(ns_sock, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving request"END);
		close(ns_sock);
		return(1);
	}
	printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	sprintf(buffer, "%d", client_port);
	if(send(ns_sock, buffer, strlen(buffer), 0) < 0){
		printf(RED"Socket Error"END);
		close(ns_sock);
		exit(1);
	}
	printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	if(recv(ns_sock, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving request"END);
		close(ns_sock);
		return(1);
	}
	printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, working_directory);
	if(send(ns_sock, buffer, strlen(buffer), 0) < 0){
		printf(RED"Socket Error"END);
		close(ns_sock);
		exit(1);
	}
	printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
	
	return 0;
}

int prepare_for_client(int port){
	printf(BLUE"[+] Initialising socket for connection to client...\n"END);
	
	char *ip = "127.0.0.1";
	
	struct sockaddr_in server_addr;
	
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0){
		printf(RED"Socket Error: %s"END, strerror(errno));
		return(1);
	}
	
	// set SO_REUSEADDR option
	// https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/ mentions it as a good practice and the following function was taken from MP1
	int reuseaddr = 1;
	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		fprintf(stderr, RED"Setsockopt failed"END);
		close(server_sock);
		exit(1);
	}
	
	printf(GREEN"[+] Socket created\n"END);
	
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	
	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		printf(RED"Bind Error: %s"END, strerror(errno));
		return(1);
	}
	
	printf(YELLOW"[+] Binded to the port number: %d\n"END, port);
	
	return 0;
}

void* client_handler(void* args){
	struct ThreadArgs* threadArgs = (struct ThreadArgs*)args;
	int clientSocket = threadArgs->clientSocket;
	
	char buffer[4096];
	char command[4096];
	char source_path[4096];
	
	bzero(buffer, 4096);
	if(recv(clientSocket, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(clientSocket);
		exit(1);
	}
	
	bzero(buffer, 4096);
	strcpy(buffer, "HELLO CLIENT, SEND COMMAND");
	if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(clientSocket);
		exit(1);
	}
	
	bzero(buffer, 4096);
	if(recv(clientSocket, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(clientSocket);
		exit(1);
	}
	strcpy(command, buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, "SEND SOURCE PATH");
	if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(clientSocket);
		exit(1);
	}
	
	bzero(buffer, 4096);
	if(recv(clientSocket, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(clientSocket);
		exit(1);
	}
	strcpy(source_path, buffer);
	
	if(strcmp(command, "READ") == 0){
		FILE *file;
		file = fopen(source_path, "r");
		
		if (file == NULL) {
			printf(RED"Error opening file"END);
			bzero(buffer, 4096);
			strcpy(buffer, "FAIL");
			if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending request"END);
			}
			close(clientSocket);
			free(args);
			pthread_exit(NULL);
		}
		
		bzero(buffer, 4096);
		size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
		if(bytesRead <= 0){
			printf(RED"Error reading file"END);
			bzero(buffer, 4096);
			strcpy(buffer, "FAIL");
			if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending request"END);
			}
			close(clientSocket);
			free(args);
			pthread_exit(NULL);
		}
		
		fclose(file);
		buffer[bytesRead] = '\0';
		
		if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
			printf(RED"Error while sending request"END);
			close(clientSocket);
			free(args);
			pthread_exit(NULL);
		}
		close(clientSocket);
		
		free(args);
		pthread_exit(NULL);
	} else if(strcmp(command, "WRITE") == 0){
		pthread_mutex_lock(&fileMutex);
		
		bzero(buffer, 4096);
		strcpy(buffer, "SEND CONTENT");
		if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
			printf(RED"Error while sending request"END);
			close(clientSocket);
			pthread_mutex_unlock(&fileMutex);
			free(args);
			pthread_exit(NULL);
			
			return 0;
		}
		
		bzero(buffer, 4096);
		if(recv(clientSocket, buffer, sizeof(buffer), 0) < 0){
			printf(RED"Error while receiving response"END);
			close(clientSocket);
			pthread_mutex_unlock(&fileMutex);
			free(args);
			pthread_exit(NULL);
			
			return 0;
		}
		
		FILE *file;
		file = fopen(source_path, "a");
		
		if (file == NULL) {
			printf(RED"Error while opening file"END);
			bzero(buffer, 4096);
			strcpy(buffer, "FAIL");
			if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending request"END);
			}
			close(clientSocket);
			pthread_mutex_unlock(&fileMutex);
			free(args);
			pthread_exit(NULL);
		} else{
			// Write the buffer to the file
			if (fprintf(file, "%s", buffer) < 0) {
				// Close the file
				fclose(file);
				printf(RED"Error while writing"END);
				bzero(buffer, 4096);
				strcpy(buffer, "FAIL");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
				}
				close(clientSocket);
				pthread_mutex_unlock(&fileMutex);
				free(args);
				pthread_exit(NULL);
			} else{
				// Close the file
				fclose(file);
				
				bzero(buffer, 4096);
				strcpy(buffer, "DONE");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
				}
				close(clientSocket);
				pthread_mutex_unlock(&fileMutex);
				free(args);
				pthread_exit(NULL);
			}
		}
	} else if(strcmp(command, "INFO") == 0){
		// code from MP1 and https://stackoverflow.com/a/10323127
		struct stat info;
		stat(source_path, &info);
		struct passwd* pwd_entry;
		pwd_entry = getpwuid(info.st_uid);
		struct group* group_entry;
		group_entry = getgrgid(info.st_gid);
		printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
		printf( (info.st_mode & S_IRUSR) ? "r" : "-");
		printf( (info.st_mode & S_IWUSR) ? "w" : "-");
		printf( (info.st_mode & S_IXUSR) ? "x" : "-");
		printf( (info.st_mode & S_IRGRP) ? "r" : "-");
		printf( (info.st_mode & S_IWGRP) ? "w" : "-");
		printf( (info.st_mode & S_IXGRP) ? "x" : "-");
		printf( (info.st_mode & S_IROTH) ? "r" : "-");
		printf( (info.st_mode & S_IWOTH) ? "w" : "-");
		printf( (info.st_mode & S_IXOTH) ? "x" : "-");
		sprintf(buffer, " %d %s %s %lld %s\n", info.st_nlink, pwd_entry->pw_name, group_entry->gr_name, info.st_size, strtok(ctime(&info.st_mtime), "\n"));
		
		if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
			printf(RED"Error while sending request"END);
			close(clientSocket);
			exit(1);
		}
		close(clientSocket);
		
		free(args);
		pthread_exit(NULL);
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Usage: %s <port> <port> <directory>\n", argv[0]);
		exit(0);
	}
	
	strncpy(working_directory, argv[3], sizeof(&argv[3]));
	ns_port = atoi(argv[1]);
	client_port = atoi(argv[2]);
	
	printf(BLUE"[+] Initialising storage server...\n"END);
	
	if(chdir(working_directory) == -1){
		printf(RED"Directory Error: %s"END, strerror(errno));
		exit(1);
	}
	getcwd(working_directory, 4096);
	printf(YELLOW"[+] Directory set to: %s\n"END, working_directory);
	
	if(init(atoi(argv[1])) != 0){
		printf(RED"Initialisation Error: %s"END, strerror(errno));
		exit(1);
	}
	
	printf(GREEN"[+] Storage server initialised\n"END);
	
	if(prepare_for_client(client_port) != 0){
		printf(RED"Error while preparing for client"END);
		close(ns_sock);
		exit(1);
	}
	
	char buffer[4096];
	// From Doubt Document: [VB] If you want to assume a max limit on the number of clients for maintaining an array of free ports take it as 20.
	pthread_t threadPool[20];
	int thread_counter = -1;
	
	while(1){
		bzero(buffer, 4096);
		if(recv(ns_sock, buffer, sizeof(buffer), 0) <= 0){
			printf(RED"Error while receiving request"END);
			close(ns_sock);
			exit(1);
		}
		printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
		
		if(strcmp(buffer, "END") == 0){
			close(server_sock);
			close(ns_sock);
			exit(1);
		} else if(strcmp(buffer, "PING") == 0){
			bzero(buffer, 4096);
			strcpy(buffer, "OK");
			if(send(ns_sock, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending"END);
				close(ns_sock);
				exit(1);
			}
			printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
		} else if(strcmp(buffer, "CLIENT") == 0){
			// the following code for threads is written with help from https://chat.openai.com/share/bbf0dcc5-3497-4db7-a498-933f1d031aad
			thread_counter++;
			
			if(thread_counter == 20){
				printf(BLUE"Out of threads, waiting for threads...\n"END);
				
				for (int i = 0; i < 20; ++i) {
					pthread_join(threadPool[i], NULL);
				}
				
				thread_counter = 0;
			}
			
			if(listen(server_sock, 1) != 0){
				printf(RED"Listen Error: %s"END, strerror(errno));
				exit(1);
			}
			printf(BLUE"[+] Listening for new client...\n"END);
			
			int clientSocket;
			struct sockaddr_in clientAddr;
			socklen_t addrSize = sizeof(struct sockaddr_in);
			if ((clientSocket = accept(server_sock, (struct sockaddr*)&clientAddr, &addrSize)) < 0) {
				printf(RED"Error while accepting client connection"END);
				exit(1);
			}
			
			printf(GREEN"[+] Client connected\n"END);
			printf(BLUE"[+] Allocating memory for client\n"END);
			struct ThreadArgs* args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs));
			if (args == NULL) {
				printf(RED"Memory allocation failed"END);
				close(clientSocket);
				exit(1);
			}
			
			printf(GREEN"[+] Memory Allocated\n"END);
			
			args->clientSocket = clientSocket;
			
			int result = pthread_create(&threadPool[thread_counter], NULL, client_handler, (void*)args);
			
			if (result != 0) {
				printf(RED"Thread Error: %s"END, strerror(errno));
				free(args);
				close(clientSocket);
				exit(1);
			}
		}
	}
}