#include "headers.h"

// the following code blocks for trie is implemented using ChatGPT
// you can access chat at https://chat.openai.com/share/ba2d6744-9c83-4c10-8d04-7955c568fb58
// Define the structure for a trie node
typedef struct TrieNode {
	struct TrieNode *children[128]; // Assuming ASCII characters
	int isDirectory; // Flag to indicate if this node represents a directory
} TrieNode;

// Function to initialize a new trie node
TrieNode *createNode() {
	TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
	if (node) {
		node->isDirectory = 0;
		for (int i = 0; i < 128; i++) {
			node->children[i] = NULL;
		}
	}
	return node;
}

// Function to insert a directory or file path into the trie
void insertPath(TrieNode *root, const char *path, int isDirectory) {
	TrieNode *current = root;
	int len = strlen(path);
	
	for (int i = 0; i < len; i++) {
		int index = (int)path[i];
		if (!current->children[index]) {
			current->children[index] = createNode();
		}
		current = current->children[index];
	}
	
	current->isDirectory = isDirectory;
}

void printTrie(TrieNode *node, char *path, int level) {
	if (node == NULL) {
		return;
	}
	
	// Print the current node
	for (int i = 0; i < 128; i++) {
		if (node->children[i] != NULL) {
			path[level] = (char)i;
			printTrie(node->children[i], path, level + 1);
		}
	}
	
	// Check if the current node represents a directory
	if (node->isDirectory) {
		path[level] = '\0';
		printf("%s\n", path);
	}
}

// Function to search for a directory or file path in the trie
int searchPath(TrieNode *root, const char *path) {
	TrieNode *current = root;
	int len = strlen(path);
	
	for (int i = 0; i < len; i++) {
		int index = (int)path[i];
		if (!current->children[index]) {
			return 0; // Path not found
		}
		current = current->children[index];
	}
	
	// Check if the current node represents a directory or file
	return (current != NULL && (current->isDirectory || path[len-1] == '\0'));
}

// Recursive function to initialize the trie with the directory structure
void initTrieRecursive(TrieNode *root, const char *currentDirectory) {
	DIR *dir;
	struct dirent *entry;
	
	// Open the current directory
	if ((dir = opendir(currentDirectory)) != NULL) {
		// Read each entry in the directory
		while ((entry = readdir(dir)) != NULL) {
			// Ignore "." and ".." entries
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
				// Construct the full path of the entry
				char fullPath[256];
				snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDirectory, entry->d_name);
				
				// Determine if the entry is a directory
				int isDirectory = (entry->d_type == DT_DIR);
				
				// Insert the relative path into the trie
				insertPath(root, entry->d_name, isDirectory);
				
				// Recursively initialize the trie for subdirectories
				if (isDirectory) {
					initTrieRecursive(root, fullPath);
				}
			}
		}
		closedir(dir);
	} else {
		perror("Error opening directory");
	}
}

// Function to initialize the trie with the current directory structure
TrieNode *initTrie(const char *currentDirectory) {
	TrieNode *root = createNode();
	initTrieRecursive(root, currentDirectory);
	return root;
}

// Recursive function to free the memory allocated for trie nodes
void freeTrie(TrieNode *node) {
	if (node == NULL) {
		return;
	}
	
	for (int i = 0; i < 128; i++) {
		freeTrie(node->children[i]);
	}
	
	free(node);
}

typedef struct SS storage_server;

struct SS
{
	char ip[100];
	int ns_port;
	int socket_ns;
	int client_port;
	char directory[4096];
	int status;
	TrieNode *root;
};

int s_servers_count = 0;
storage_server s_servers[4096];
int server_sock;

struct ThreadArgs {
	int clientSocket;
};

// the following code is written with reference from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C
// this link was provided under resources in MP2
int connect_storage_server(int port){
	printf(BLUE"\n[+] Initialising connection to storage server %d...\n"END, s_servers_count);
	
	char *ip = "127.0.0.1";
	
	strncpy(s_servers[s_servers_count].ip, ip, sizeof(&ip));
	s_servers[s_servers_count].ns_port = port;
	
	struct sockaddr_in addr;
	socklen_t addr_size;
	char buffer[4096];
	
	s_servers[s_servers_count].socket_ns = socket(AF_INET, SOCK_STREAM, 0);
	if (s_servers[s_servers_count].socket_ns < 0){
		printf(RED"Socket Error: %s"END, strerror(errno));
		return (1);
	}
	printf(GREEN"[+] Socket created\n"END);
	
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = inet_addr(ip);
	
	if(connect(s_servers[s_servers_count].socket_ns, (struct sockaddr*)&addr, sizeof(addr)) != 0){
		printf(RED"Error while establishing connection"END);
		close(s_servers[s_servers_count].socket_ns);
		return (1);
	}
	printf(GREEN"[+] Connected to storage server %d\n"END, s_servers_count);
	
	bzero(buffer, 4096);
	strcpy(buffer, "HELLO, THIS IS NAME SERVER.");
	if(send(s_servers[s_servers_count].socket_ns, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(s_servers[s_servers_count].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server %d: %s\n"END, s_servers_count, buffer);
	
	bzero(buffer, 4096);
	if(recv(s_servers[s_servers_count].socket_ns, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(s_servers[s_servers_count].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server %d: %s\n"END, s_servers_count, buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, "Send client port");
	if(send(s_servers[s_servers_count].socket_ns, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(s_servers[s_servers_count].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server %d: %s\n"END, s_servers_count, buffer);
	
	bzero(buffer, 4096);
	if(recv(s_servers[s_servers_count].socket_ns, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(s_servers[s_servers_count].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server %d: %s\n"END, s_servers_count, buffer);
	s_servers[s_servers_count].client_port = atoi(buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, "Send directory");
	if(send(s_servers[s_servers_count].socket_ns, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(s_servers[s_servers_count].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server %d: %s\n"END, s_servers_count, buffer);
	
	bzero(buffer, 4096);
	if(recv(s_servers[s_servers_count].socket_ns, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(s_servers[s_servers_count].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server %d: %s\n"END, s_servers_count, buffer);
	strncpy(s_servers[s_servers_count].directory, buffer, sizeof(buffer));
	s_servers[s_servers_count].root = initTrie(s_servers[s_servers_count].directory);
	
	s_servers[s_servers_count].status = 1;
	
	printf(GREEN"[+] Initialised storage server %d\n"END, s_servers_count);
	
	return 0;
}

// the following code is written with reference from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C
// this link was provided under resources in MP2
int reconnect(int port){
	printf(YELLOW"[+] Reconnecting to old storage server at port %d\n"END, port);
	
	int index = 0;
	for(int i = 1; i <= s_servers_count; i++){
		if(s_servers[i].ns_port == port){
			index = i;
			break;
		}
	}
	
	char *ip = "127.0.0.1";
	
	struct sockaddr_in addr;
	socklen_t addr_size;
	char buffer[4096];
	
	s_servers[index].socket_ns = socket(AF_INET, SOCK_STREAM, 0);
	if (s_servers[index].socket_ns < 0){
		printf(RED"Socket Error: %s"END, strerror(errno));
		return (1);
	}
	printf(GREEN"[+] Socket created\n"END);
	
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = inet_addr(ip);
	
	if(connect(s_servers[index].socket_ns, (struct sockaddr*)&addr, sizeof(addr)) != 0){
		printf(RED"Error while establishing connection"END);
		close(s_servers[index].socket_ns);
		return (1);
	}
	printf(GREEN"[+] Connected to storage server %d\n"END, index);
	
	bzero(buffer, 4096);
	strcpy(buffer, "WELCOME BACK, THIS IS NAME SERVER.");
	if(send(s_servers[index].socket_ns, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(s_servers[index].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server %d: %s\n"END, index, buffer);
	
	bzero(buffer, 4096);
	if(recv(s_servers[index].socket_ns, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(s_servers[index].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server %d: %s\n"END, index, buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, "Send client port for cross-checking");
	if(send(s_servers[index].socket_ns, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(s_servers[index].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server %d: %s\n"END, index, buffer);
	
	bzero(buffer, 4096);
	if(recv(s_servers[index].socket_ns, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(s_servers[index].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server %d: %s\n"END, index, buffer);
	s_servers[index].client_port = atoi(buffer);
	
	bzero(buffer, 4096);
	strcpy(buffer, "Send directory for cross-checking");
	if(send(s_servers[index].socket_ns, buffer, strlen(buffer), 0) < 0){
		printf(RED"Error while sending request"END);
		close(s_servers[index].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Sent to storage server %d: %s\n"END, index, buffer);
	
	bzero(buffer, 4096);
	if(recv(s_servers[index].socket_ns, buffer, sizeof(buffer), 0) < 0){
		printf(RED"Error while receiving response"END);
		close(s_servers[index].socket_ns);
		return(1);
	}
	printf(YELLOW"[+] Received from storage server %d: %s\n"END, index, buffer);
	strncpy(s_servers[index].directory, buffer, sizeof(buffer));
	s_servers[s_servers_count].root = initTrie(s_servers[s_servers_count].directory);
	
	s_servers[index].status = 1;
	
	printf(GREEN"[+] Initialised storage server %d\n"END, index);
	
	return 0;
}

// the following code is written with reference from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C
// this link was provided under resources in MP2
int prepare_for_client(int port){
	printf(BLUE"[+] Initialising socket for connection to clients...\n"END);
	
	char *ip = "127.0.0.1";
	
	struct sockaddr_in server_addr;
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
	
	return 0;
}

// the following code is written with reference from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C
// this link was provided under resources in MP2
void* client_handler(void* args){
	struct ThreadArgs* threadArgs = (struct ThreadArgs*)args;
	int clientSocket = threadArgs->clientSocket;
	
	char buffer[4096];
	char command[4096];
	char source_path[4096];
	char destination_path[4096];
	
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
	
	while(1){
		bzero(buffer, 4096);
		if(recv(clientSocket, buffer, sizeof(buffer), 0) < 0){
			printf(RED"Error while receiving response"END);
			close(clientSocket);
			exit(1);
		}
		strcpy(command, buffer);
		
		if(strcmp(buffer, "END") == 0){
			close(clientSocket);
			
			free(args);
			pthread_exit(NULL);
			break;
			return 0;
		}
		
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
		
		if(strcmp(command, "COPY") == 0){
			bzero(buffer, 4096);
			strcpy(buffer, "SEND DESTINATION PATH");
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
			strcpy(destination_path, buffer);
			
			int found_src = 0;
			for(int i = 1; i <= s_servers_count; i++){
				if(searchPath(s_servers[i].root, source_path)){
					char temp[4096];
					strcpy(temp, s_servers[i].directory);
					strcat(temp, "/");
					strcat(temp, source_path);
					strcpy(source_path, temp);
					found_src = i;
					break;
				}
			}
			if(found_src == 0){
				bzero(buffer, 4096);
				strcpy(buffer, "FAIL: Source directory does not exist");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			} else{
				int found_dest = 0;
				for(int i = 1; i <= s_servers_count; i++){
					if(searchPath(s_servers[i].root, destination_path)){
						char temp[4096];
						strcpy(temp, s_servers[i].directory);
						strcat(temp, "/");
						strcat(temp, destination_path);
						strcpy(destination_path, temp);
						found_dest = i;
						break;
					}
				}
				if(found_dest == 0){
					bzero(buffer, 4096);
					strcpy(buffer, "FAIL: Destination directory does not exist");
					if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
						printf(RED"Error while sending request"END);
						close(clientSocket);
						exit(1);
					}
				} else{
					// based on https://chat.openai.com/share/97bd8184-4cf9-4f2a-ab72-bc87e9b4a2d2
					char *const argv[] = {"cp", "-r", source_path, destination_path, NULL};
					
					pid_t pid = fork();
					if(pid < 0){
						fprintf(stderr, RED"Error: %s\n"END, strerror(errno));
						close(clientSocket);
						exit(1);
					} else if(pid == 0){
						if (execvp("cp", argv) == -1) {
							bzero(buffer, 4096);
							strcpy(buffer, "FAIL: Error occurred while copying");
							if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
								printf(RED"Error while sending request"END);
								close(clientSocket);
								exit(1);
							}
						}
					} else{
						waitpid(pid, NULL, 0);
					}
					freeTrie(s_servers[found_dest].root);
					s_servers[found_dest].root = initTrie(s_servers[found_dest].directory);
					bzero(buffer, 4096);
					strcpy(buffer, "DONE");
					if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
						printf(RED"Error while sending request"END);
						close(clientSocket);
						exit(1);
					}
				}
			}
		} else if(strcmp(command, "READ") == 0){
			int send_port;
			
			int found = 0;
			for(int i = 1; i < s_servers_count; i++){
				if(searchPath(s_servers[i].root, source_path)){
					send_port = s_servers[i].client_port;
					found = 1;
					break;
				}
			}
			
			if(found == 0){
				bzero(buffer, 4096);
				strcpy(buffer, "FAIL: No such path exists");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			} else{
				bzero(buffer, 4096);
				sprintf(buffer, "%d", send_port);
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			}
		} else if(strcmp(command, "WRITE") == 0){
			int send_port;
			
			int found = 0;
			for(int i = 1; i < s_servers_count; i++){
				if(searchPath(s_servers[i].root, source_path)){
					send_port = s_servers[i].client_port;
					found = 1;
					break;
				}
			}
			
			if(found == 0){
				bzero(buffer, 4096);
				strcpy(buffer, "FAIL: No such path exists");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			} else{
				bzero(buffer, 4096);
				sprintf(buffer, "%d", send_port);
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			}
		} else if(strcmp(command, "INFO") == 0){
			int send_port;
			
			int found = 0;
			for(int i = 1; i < s_servers_count; i++){
				if(searchPath(s_servers[i].root, source_path)){
					send_port = s_servers[i].client_port;
					found = 1;
					break;
				}
			}
			
			if(found == 0){
				bzero(buffer, 4096);
				strcpy(buffer, "FAIL: No such path exists");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			} else{
				bzero(buffer, 4096);
				sprintf(buffer, "%d", send_port);
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			}
		} else if(strcmp(command, "CREATE") == 0){
			int type = 0;
			bzero(buffer, 4096);
			strcpy(buffer, "SEND STORAGE SERVER INDEX");
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
			strcpy(destination_path, buffer);
			
			bzero(buffer, 4096);
			strcpy(buffer, "SEND TYPE");
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
			type = atoi(buffer);
			
			int found = 0;
			for(int i = 1; i < s_servers_count; i++){
				if(searchPath(s_servers[i].root, source_path)){
					found = 1;
					break;
				}
			}
			if(found == 1){
				bzero(buffer, 4096);
				strcpy(buffer, "FAIL: Path already exists");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			} else{
				char temp[4096];
				strcpy(temp, s_servers[atoi(destination_path)].directory);
				strcat(temp,"/");
				strcat(temp, source_path);
				strcpy(source_path, temp);
				// based on https://chat.openai.com/share/97bd8184-4cf9-4f2a-ab72-bc87e9b4a2d2
				if(type == 0){
					if (mkdir(source_path, 0777) == -1) {
						bzero(buffer, 4096);
						strcpy(buffer, "FAIL: Could not create directory");
						printf("\n%s\n", buffer);
						if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
							printf(RED"Error while sending request"END);
							close(clientSocket);
							exit(1);
						}
					} else {
						freeTrie(s_servers[atoi(destination_path)].root);
						s_servers[atoi(destination_path)].root = initTrie(s_servers[atoi(destination_path)].directory);
						bzero(buffer, 4096);
						strcpy(buffer, "DONE");
						if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
							printf(RED"Error while sending request"END);
							close(clientSocket);
							exit(1);
						}
					}
				} else if(type == 1){
					FILE *file = fopen(source_path, "w");
					if (file != NULL) {
						fclose(file);
						freeTrie(s_servers[atoi(destination_path)].root);
						s_servers[atoi(destination_path)].root = initTrie(s_servers[atoi(destination_path)].directory);
						bzero(buffer, 4096);
						strcpy(buffer, "DONE");
						if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
							printf(RED"Error while sending request"END);
							close(clientSocket);
							exit(1);
						}
					} else {
						bzero(buffer, 4096);
						strcpy(buffer, "FAIL: Could not create file");
						printf("\n%s\n", buffer);
						if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
							printf(RED"Error while sending request"END);
							close(clientSocket);
							exit(1);
						}
					}
				}
			}
		} else if(strcmp(command, "DELETE") == 0){
			int found_src = 0;
			for(int i = 1; i <= s_servers_count; i++){
				if(searchPath(s_servers[i].root, source_path)){
					strcpy(destination_path, source_path);
					char temp[4096];
					strcpy(temp, s_servers[i].directory);
					strcat(temp, "/");
					strcat(temp, source_path);
					strcpy(source_path, temp);
					found_src = i;
					break;
				}
			}
			if(found_src == 0){
				bzero(buffer, 4096);
				strcpy(buffer, "FAIL: Path does not exist");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			} else{
				// based on https://chat.openai.com/share/97bd8184-4cf9-4f2a-ab72-bc87e9b4a2d2
				char *const argv[] = {"rm", "-r", source_path, NULL};
				
				pid_t pid = fork();
				if(pid < 0){
					fprintf(stderr, RED"Error: %s\n"END, strerror(errno));
					close(clientSocket);
					exit(1);
				} else if(pid == 0){
					if (execvp("rm", argv) == -1) {
						bzero(buffer, 4096);
						strcpy(buffer, "FAIL: Could not remove path");
						if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
							printf(RED"Error while sending request"END);
							close(clientSocket);
							exit(1);
						}
					}
				} else{
					waitpid(pid, NULL, 0);
				}
				freeTrie(s_servers[found_src].root);
				s_servers[found_src].root = initTrie(s_servers[found_src].directory);
				bzero(buffer, 4096);
				strcpy(buffer, "DONE");
				if(send(clientSocket, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while sending request"END);
					close(clientSocket);
					exit(1);
				}
			}
		}
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(0);
	}
	
	printf(BLUE"[+] Initialising connection to storage servers...\n"END);
	
	while(1){
		printf("\n");
		printf("Enter the port number for storage server %d (enter 0 to indicate end of storage servers): ", ++s_servers_count);
		int port;
		scanf("%d", &port);
		if(port == 0){
			s_servers_count--;
			break;
		}
		if(connect_storage_server(port) != 0){
			printf(RED"Connection Error: %s"END, strerror(errno));
			exit(1);
		}
	}
	
	printf(GREEN"\n[+] Initialised connection to storage servers\n"END);
	
	if(prepare_for_client(atoi(argv[1])) != 0){
		printf(RED"Initialisation Error: %s"END, strerror(errno));
		exit(1);
	}
	
	// From Doubt Document: [VB] If you want to assume a max limit on the number of clients for maintaining an array of free ports take it as 20.
	pthread_t threadPool[20];
	int thread_counter = -1;
	
	while(1){
		for(int i = 1; i <= s_servers_count; i++){
			if(s_servers[i].status != 0){
				char buffer[4096];
				bzero(buffer, 4096);
				strcpy(buffer, "PING");
				if(send(s_servers[i].socket_ns, buffer, strlen(buffer), 0) < 0){
					printf(RED"Error while pinging storage server"END);
					close(s_servers[i].socket_ns);
					exit(1);
				}
				bzero(buffer, 4096);
				if(recv(s_servers[i].socket_ns, buffer, sizeof(buffer), 0) <= 0){
					close(s_servers[i].socket_ns);
					s_servers[i].status = 0;
					freeTrie(s_servers[i].root);
				}
			}
			if(s_servers[i].status == 1){
				printf(GREEN"[+] Storage Server (%d) %d: OK\n"END, s_servers[i].ns_port, i);
			} else{
				printf(RED"[+] Storage Server (%d) %d: DEAD\n"END, s_servers[i].ns_port, i);
			}
		}
		printf("\nMenu:\n");
		printf("\tCLIENT : Prepares name server to connect to new client\n");
		printf("\tSTORAGE : Connects to new storage server/reconnects to old storage server at the port (Distinction between old and new is based on port)\n");
		printf("\tSKIP : Skips current iteration\n");
		printf("\tEND: Ends the program\n");
		printf("\nEnter your choice: ");
		char input[4096];
		scanf("%s", input);
		int port_number;
		if(strcmp(input, "SKIP") == 0){
			continue;
		} else if(strcmp(input, "END") == 0){
			for(int i = 1; i <= s_servers_count; i++){
				if(s_servers[i].status != 0){
					char buffer[4096];
					bzero(buffer, 4096);
					strcpy(buffer, "END");
					if(send(s_servers[i].socket_ns, buffer, strlen(buffer), 0) < 0){
						printf(RED"Error while pinging storage server"END);
						close(s_servers[i].socket_ns);
						exit(1);
					}
					close(s_servers[i].socket_ns);
					freeTrie(s_servers[i].root);
				}
			}
			close(server_sock);
			exit(1);
		}
		if(strcmp(input, "STORAGE") == 0){
			printf("\nEnter port number: ");
			scanf("%d", &port_number);
		}
		
		printf("\n");
		
		// the following code for threads is written with help from https://chat.openai.com/share/bbf0dcc5-3497-4db7-a498-933f1d031aad
		if(strcmp(input, "CLIENT") == 0){
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
		if(strcmp(input, "STORAGE") == 0){
			int found = 0;
			for(int i = 1; i <= s_servers_count; i++){
				if(s_servers[i].ns_port == port_number){
					found = 1;
					if(reconnect(s_servers[i].ns_port) != 0){
						printf(RED"Connection Error: %s"END, strerror(errno));
						exit(1);
					}
					break;
				}
			}
			if(found == 0){
				s_servers_count++;
				if(connect_storage_server(port_number) != 0){
					printf(RED"Connection Error: %s"END, strerror(errno));
					exit(1);
				}
			}
		}
	}
}