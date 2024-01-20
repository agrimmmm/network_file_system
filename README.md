# NFS Server Documentation

## Table of Contents:

- [Code Compilation and Execution](#code-compilation-and-execution)
  1. [Clone the repository](#1-clone-the-repository)
  2. [Navigate to the appropriate directory](#2-navigate-to-the-appropriate-directory)
  3. [Start the Naming Server](#3-start-the-naming-server)
  4. [Start Storage Servers](#4-start-storage-servers)
  5. [Start the Client](#5-start-the-client)

- [Working of NFS Server](#working-of-nfs-server)
  1. [Naming Server (NM)](#1-naming-server-nm)
    - [Handle Client Thread](#handle-client-thread)
    - [Handle Storage Server Thread](#handle-storage-server-thread)
  2. [Implemented Functions](#2-implemented-functions)
    - [Reading, Writing, and Retrieving Information about Files](#reading-writing-and-retrieving-information-about-files)
    - [Creating and Deleting Files and Folders](#creating-and-deleting-files-and-folders)
    - [Copying Files/Directories Between Storage Servers](#copying-filesdirectories-between-storage-servers)

- [Data Structures Used](#data-structures-used)
  1. [Tries Data Structures](#1-tries-data-structures)
  2. [Least Recently Used (LRU)](#2-least-recently-used-lru)
  3. [SS_Info](#3-ss_info)

- [Locking Mechanisms](#locking-mechanisms)

- [Reader-Writer Locks](#reader-writer-locks)

## Code Compilation and Execution

1. Clone the repository:

    ```bash
    git clone https://github.com/serc-courses/final-project-011.git
    ```

2. Navigate to the appropriate directory:

    ```bash
    cd final-project-011/
    ```

3. Start the Naming Server:

    ```bash
    ./naming_server
    ```

4. Start Storage Servers:

    ```bash
    ./storage_server <nm_port> <client_port>
    ```

5. Start the Client:

    ```bash
    ./client
    ```

## Working of NFS Server

### Naming Server (NM)

The role of the Naming Server (NM) is pivotal in managing the communication between clients and storage servers within the NFS (Network File System) Server infrastructure. The NM's functionality is characterized by the orchestration of two essential threads designed to facilitate seamless interactions in a distributed file system environment.

#### 1. Handle Client Thread
The Handle Client Thread within the Naming Server serves as a crucial component responsible for managing client interactions. The workflow of this thread involves the following intricacies:

- **Thread Multiplicity:**
This thread dynamically generates 100 sub-threads, each meticulously assigned to cater to the specific needs of individual clients. This design promotes a scalable and concurrent approach, enabling the NM to efficiently handle numerous client requests simultaneously.

- **File-Related Operations:**
    Clients, under the purview of the Handle Client Thread, can initiate diverse file-related operations. These operations encompass fundamental tasks such as reading, writing, and retrieving information pertinent to files stored within the NFS Server.


#### 2. Handle Storage Server Thread

The Handle Storage Server Thread within the Naming Server is engineered to manage the interaction with storage servers. The intricacies of this thread are delineated as follows:

- **Thread Generation:**
This thread exhibits the capability to spawn 100 additional threads for each storage server. This design is instrumental in accommodating concurrent connections from multiple clients to individual storage servers, thereby optimizing the overall server throughput.

- **Reserved Connection:**
Each storage server possesses a dedicated and reserved connection intended exclusively for communication with the Naming Server. This reserved channel facilitates swift and secure information exchange, ensuring precise coordination in file operations.

The symbiotic functioning of these two threads within the Naming Server establishes a robust and scalable infrastructure, capable of handling a myriad of client-server interactions within the NFS Server ecosystem.

## Implemented Functions

### Reading, Writing, and Retrieving Information about Files

#### Client Request:
Clients initiate file operations by providing the file path. NM (Naming Server) locates the correct Storage Server.

#### NM Facilitation:
NM identifies the Storage Server and returns IP address and client port to the client.

#### Direct Communication:
Client communicates directly with the designated SS (Storage Server). The operation continues until a predefined condition is met.

### Creating and Deleting Files and Folders

#### Client Request:
Clients request creation or deletion operations. NM forwards the request to the appropriate SS.

#### SS Execution:
SS processes the request and performs the specified action.

#### Acknowledgment and Feedback:
SS sends ACK or STOP packet to NM. NM conveys this information to the client, providing feedback.

### Copying Files/Directories Between Storage Servers

#### Client Request:
Clients request copying files or directories between SS. NM manages the transfer between the relevant SS.

#### NM Execution:
NM orchestrates the copying process, ensuring accurate data transfer.

#### Acknowledgment and Client Notification:
NM sends an ACK packet to the client upon successful completion.

## To run the commands, use the following format:

- **Read File:**
  
    ```bash
    read <filepath>
    ```
    
- **Write to File:**
  
    ```bash
    write <filepath>
    ```
    
- **Retrieve File Information:**
  
    ```bash
    retrieve <filepath>
    ```
    
- **Create File:**
  
    ```bash
    create_file <filepath>
    ```
    
- **Create Folder:**
  
    ```bash
    create_folder <folderpath>
    ```
    
- **Delete File:**
  
    ```bash
    delete_file <filepath>
    ```
    
- **Delete Folder:**
  
    ```bash
    delete_folder <folderpath>
    ```
    
- **Copy File:**
  
    ```bash
    copy_file <old_filepath> <destination
    ```
    
- **Copy Folder:**
  
    ```bash
    copy_folder <old_folderpath> <destination
    ```


### Data Structures Used

1. **Tries Data Structures:**
   Utilized within the Naming Server, the Tries Data Structure plays a crucial role in storing file paths and their corresponding storage numbers. This hierarchical structure enables efficient retrieval and management of file-related information.

2. **Least Recently Used (LRU):**
   The Least Recently Used (LRU) data structure serves as a dynamic cache for tracking recently accessed files. With a set capacity of 15 files, LRU ensures optimal resource utilization by retaining information about the most recently used files and facilitating quicker access.

3. **SS_Info:**
   The SS_Info data structure proves essential for maintaining a comprehensive record of connected storage servers. It encompasses vital details such as the storage server number, port and IP address. This information is instrumental in orchestrating seamless communication and coordination between the Naming Server and Storage Servers.

### Locking Mechanisms

- `trie_lock`:
  The `trie_lock` serves as a critical locking mechanism implemented to secure the Trie data structures within the Naming Server. This ensures thread safety during concurrent operations involving file path storage and retrieval.

- `lru_lock`:
  The `lru_lock` is employed to safeguard the LRU data structure, preventing potential race conditions and ensuring the integrity of the Least Recently Used cache. This locking mechanism guarantees that operations on the LRU, such as updating or accessing file usage information, occur in a synchronized manner.

### Reader-Writer Locks

Reader-Writer locks are strategically implemented to regulate access to shared resources during file operations. This mechanism ensures that while one client is writing to a file, concurrent write attempts by other clients are restricted. By facilitating a controlled environment for file modifications, the NFS Server maintains data consistency and mitigates potential conflicts arising from concurrent write operations.


