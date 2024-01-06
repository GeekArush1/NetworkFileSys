# Running:

- `make` to compile all files
- To initialise the name server: `./n <port>`
- To initialise storage server: `./s <port_for_ns> <port_for_client> <directory>`
- To initialise client: `./c <ns_port>`

# Assumption:

- The IP address is hardcoded to localhost
- Max number of clients is 20 as mentioned in doubts document
- File size cannot be greater than buffer size (i.e., 4096 bytes). According to the doubt document, file size will not exceed ram size as was the case in MP0. Though ram was considered to be the buffer size and hence this assumption.
- Certain functionalities require input from terminal because there is no helper program for routing
- If any error is encountered, it is handled such that the program terminates that very moment instead of continuing to execute
- The user does not deviate from the provided menus and provides accurate paths
- Whenever a file/folder is created, the parent directory is assumed to exist. Otherwise the user is expected to request to create the parent directory first and then create file/folder
- Write functionality is assumed to be append in all cases
- Other assumptions allowed in doubts document like exclusive directories for different storage servers, etc
