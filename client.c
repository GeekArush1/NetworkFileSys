#include "headers.h"

int sock;

// the following code is written with reference from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C
// this link was provided under resources in MP2
int init(int port){
	char *ip = "127.0.0.1";
	
	struct sockaddr_in addr;
	socklen_t addr_size;
	char buffer[4096];
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		printf(RED"Socket Error: %s"END, strerror(errno));
		return(1);
	}
	printf(GREEN"[+] Socket created\n"END);
	
	// set SO_REUSEADDR option
	// https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/ mentions it as a good practice and the following function was taken from MP1
	int reuseaddr = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		fprintf(stderr, RED"Setsockopt failed"END);
		close(sock);
		exit(1);
	}
	
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = inet_addr(ip);
	
	if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0){
		printf(RED"Connection Error: %s"END, strerror(errno));
		close(sock);
		return(1);
	}
	printf(GREEN"[+] Connected to name server\n"END);
	
	bzero(buffer, 4096);
	strcpy(buffer, "HELLO, THIS IS CLIENT.");
	if(send(sock, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(sock);
		return(1);
	}
	printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	if(recv(sock, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(sock);
		return(1);
	}
	printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
	
	return 0;
}

// the following code is written with reference from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C
// this link was provided under resources in MP2
int ss_connection_handler(int port, char* cmd, char* path){
	char *ip = "127.0.0.1";
	
	struct sockaddr_in addr;
	socklen_t addr_size;
	char buffer[4096];
	int ss_sock;
	
	ss_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (ss_sock < 0){
		printf(RED"Socket Error: %s"END, strerror(errno));
		return(1);
	}
	printf(GREEN"[+] Socket created\n"END);
	
	// set SO_REUSEADDR option
	// https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/ mentions it as a good practice and the function was taken from MP1
	int reuseaddr = 1;
	if(setsockopt(ss_sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		fprintf(stderr, RED"Setsockopt failed"END);
		close(ss_sock);
		exit(1);
	}
	
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = inet_addr(ip);
	
	if(connect(ss_sock, (struct sockaddr*)&addr, sizeof(addr)) != 0){
		printf(RED"Connection Error: %s"END, strerror(errno));
		close(ss_sock);
		return(1);
	}
	printf(GREEN"[+] Connected to storage server\n"END);
	
	bzero(buffer, 4096);
	strcpy(buffer, "HELLO, THIS IS CLIENT.");
	if(send(ss_sock, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(ss_sock);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	if(recv(ss_sock, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(ss_sock);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, cmd);
	if(send(ss_sock, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(ss_sock);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	if(recv(ss_sock, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(ss_sock);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, path);
	if(send(ss_sock, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(ss_sock);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server: %s\n"END, buffer);
	
	bzero(buffer, 4096);
	if(recv(ss_sock, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(ss_sock);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server: %s\n"END, buffer);
	
	if(strcmp(cmd, "WRITE") == 0){
		bzero(buffer, 4096);
		printf("\nEnter the content to be written: ");
		scanf("%s", buffer);
		printf("\n");
		if(send(ss_sock, buffer, strlen(buffer), 0) < 0){
			printf(RED"Error while sending request"END);
			close(ss_sock);
			return(1);
		}
		printf(YELLOW"[+] Sent to storage server: %s\n"END, buffer);
		
		bzero(buffer, 4096);
		if(recv(ss_sock, buffer, sizeof(buffer), 0) < 0){
			printf(RED"Error while receiving response"END);
			close(ss_sock);
			return(1);
		}
		printf(YELLOW"[+] Received from storage server: %s\n"END, buffer);
	}
	
	if(strcmp(buffer, "FAIL") == 0){
		return 1;
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(0);
	}
	
	printf(BLUE"[+] Initialising connection to name servers...\n"END);
	
	if(init(atoi(argv[1])) != 0){
		printf(RED"Initialisation Error: %s"END, strerror(errno));
		exit(1);
	}
	
	printf(GREEN"[+] Connected to name server\n"END);
	
	while(1){
		char buffer[4096];
		
		printf("\nMenu:\n");
		printf("\tREAD : Prints the contents of the file\n");
		printf("\tWRITE : Write data to the file\n");
		printf("\tINFO : Prints the information of the file\n");
		printf("\tCREATE : Creates the file/folder at the path in storage sever <ss_index>\n");
		printf("\tDELETE : Delete the file/folder at the path\n");
		printf("\tCOPY : Copies the file/folder at the source path to the destination path\n");
		printf("\tEND : Closes connection and terminates\n");
		printf("\nEnter your choice: ");
		char input[4096], path1[4096], path2[4096], type[4096];
		scanf("%s", input);
		if(strcmp(input, "END") == 0){
			bzero(buffer, 4096);
			strcpy(buffer, input);
			if(send(sock, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending request"END);
				close(sock);
				exit(1);
			}
			printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
			close(sock);
			exit(1);
		} else if(strcmp(input, "COPY") == 0){
			printf("Enter destination path: ");
			scanf("%s", path1);
			printf("Enter source path: ");
			scanf("%s", path2);
		} else if(strcmp(input, "CREATE") == 0){
			printf("Enter storage server index (just the number): ");
			scanf("%s", path1);
			printf("Enter source path: ");
			scanf("%s", path2);
			printf("Enter type (0 for folder, 1 for file): ");
			scanf("%s", type);
		} else{
			printf("Enter source path: ");
			scanf("%s", path2);
		}
		
		printf("\n");
		
		bzero(buffer, 4096);
		strcpy(buffer, input);
		if(send(sock, buffer, strlen(buffer), 0) < 0){
			printf(RED"Error while sending request"END);
			close(sock);
			exit(1);
		}
		printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
		
		bzero(buffer, 4096);
		if(recv(sock, buffer, sizeof(buffer), 0) < 0){
			printf(RED"Error while receiving response"END);
			close(sock);
			exit(1);
		}
		printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
		
		bzero(buffer, 4096);
		strcpy(buffer, path2);
		if(send(sock, buffer, strlen(buffer), 0) < 0){
			printf(RED"Error while sending request"END);
			close(sock);
			exit(1);
		}
		printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
		
		bzero(buffer, 4096);
		if(recv(sock, buffer, sizeof(buffer), 0) < 0){
			printf(RED"Error while receiving response"END);
			close(sock);
			exit(1);
		}
		printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
		
		if(strcmp(input, "COPY") == 0){
			bzero(buffer, 4096);
			strcpy(buffer, path1);
			if(send(sock, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending request"END);
				close(sock);
				exit(1);
			}
			printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
			
			bzero(buffer, 4096);
			if(recv(sock, buffer, sizeof(buffer), 0) < 0){
				printf(RED"Error while receiving response"END);
				close(sock);
				exit(1);
			}
			printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
		} else if(strcmp(input, "CREATE") == 0){
			bzero(buffer, 4096);
			strcpy(buffer, path1);
			if(send(sock, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending request"END);
				close(sock);
				exit(1);
			}
			printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
			
			bzero(buffer, 4096);
			if(recv(sock, buffer, sizeof(buffer), 0) < 0){
				printf(RED"Error while receiving response"END);
				close(sock);
				exit(1);
			}
			printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
			bzero(buffer, 4096);
			strcpy(buffer, type);
			if(send(sock, buffer, strlen(buffer), 0) < 0){
				printf(RED"Error while sending request"END);
				close(sock);
				exit(1);
			}
			printf(YELLOW"[+] Sent to name server: %s\n"END, buffer);
			
			bzero(buffer, 4096);
			if(recv(sock, buffer, sizeof(buffer), 0) < 0){
				printf(RED"Error while receiving response"END);
				close(sock);
				exit(1);
			}
			printf(YELLOW"[+] Received from name server: %s\n"END, buffer);
		}
		
		if(strcmp(buffer, "DONE") == 0){
			printf(GREEN"[+] Operation Complete\n"END);
		} else if(strncmp(buffer, "FAIL", 4) == 0){
			printf(RED"[+] %s"END, buffer);
		} else{
			printf(BLUE"[+] Initialising connection to storage server\n"END);
			if(ss_connection_handler(atoi(buffer), path1, path2) != 0){
				printf(RED"Error executing the command"END);
				close(sock);
				exit(1);
			}
		}
	}
}