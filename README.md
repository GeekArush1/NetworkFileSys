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

# Features
**NFS Components**

**Clients**:
Clients act as the primary interface for users requesting access to files within the NFS. They initiate various file-related operations such as reading, writing, deleting, and more.

**Naming Server (NM)**
: The Naming Server stands as a central hub, orchestrating communication between clients and storage servers. It manages the directory structure and provides crucial information about file locations to clients.

**Storage Servers (SS)**
: Storage Servers are the backbone of the NFS, responsible for the physical storage and retrieval of files and folders. They manage data persistence and distribution across the network.

**Client Operations** 

The NFS provides clients with a suite of essential file operations:

Writing a File/Folder: Clients can actively create and update the content of files and folders within the NFS.

Reading a File: Reading operations empower clients to retrieve the contents of files stored within the NFS.

Deleting a File/Folder: Clients retain the ability to remove files and folders from the network file system, contributing to efficient space management.

Creating a File/Folder: The NFS allows clients to generate new files and folders, facilitating the expansion and organization of the file system.

All Files and Folders in a Folder: Clients can retrieve comprehensive listings of files and subfolders within a specified directory, aiding in efficient file system exploration.

Getting Additional Information: Clients can access supplementary information about specific files, such as file size, access rights, timestamps, and other metadata.

**Project Specifications**

The project was guided by detailed specifications, covering initialization, functionalities of naming and storage servers, client interactions, multiple clients support, error handling, efficient search, redundancy/replication, and bookkeeping.

**Initialization:**
Naming Server (NM) and Storage Servers (SS) were initialized, setting the foundation for the NFS. SSs dynamically added their entries to the NM, ensuring adaptability to changes and scaling requirements. Functionalities:

NM facilitated client requests, directing them to the correct SS for file operations. SSs executed commands issued by the NM, including creating and deleting files/directories and copying files between servers. Advanced Features:

Support for multiple clients and concurrent file reading. Robust error handling with distinct error codes. Efficient search in the Naming Server using data structures like Tries and Hashmaps. Redundancy/replication for fault tolerance, including failure detection and SS recovery.

**Bookkeeping:**
Implemented a logging mechanism for traceability and debugging. Recorded IP addresses and ports for each communication, aiding in issue diagnosis.
