### Sockets
***
The client-server model and various socket-related system calls are utilized to a create a multi-threaded server to evaluate the throughput of different mechanisms when using TCP/ IP to do point-to-point communication over a network. 

The client will send data over using three possible mechanisms depending on the test being performed: 
* Multiple writes: Invokes the write() system call for each data buffer, resulting in calling write() as many times as the number of data buffers
* writev: Allocates an array of iovec data structures, each having its iov_base field pointer point to a different data buffer as well as storing the buffer size in its iov_len field; and thereafter calls writev() to send all data buffers at once
* Single write: Allocates a single array of data buffers and calls write() to send this array at once

#### Files
***
The server.cpp and client.cpp are included. The client performs the following actions:
* Establishes a TCP connection with the server
* Sends a message to the server containing the number of iterations of the test it will perform (each iteration sends 1500 bytes of data)
* Perform the appropriate number of tests with the server measuring the time it takes 
* Receives the server's acknowledgment message back including the number of socket read() calls performed 
* Print information about the test including the time it took, number of reads, and throughput (Gbps)
* Close the socket

The following command line arguments are required by client.cpp:
| argv[#] | Argument | Description |
| ------ | ------ | ------ |
| argv[1] | serverName | The name of server |
| argv[2] | port | The IP port number used by the server |
| argv[3] | repetition | The repetition of sending a set of data buffers | 
| argv[4] | nbufs | The number of data buffers |
| argv[5] | bufsize | The size of each data buffer (in bytes) |
| argv[6] | type | The transfer mechanism: 1, 2, 3 |

The server performs the following actions:
The main thread will run a dispatch loop which:
* Accepts a new connection
* Creates a thread to service that request
The servicing thread will do the following:
* Allocate a buffer with size 1500 bytes to read in data being sent by the client
* Receive a message by the client with the number of iterations to perform
* Read from the client the appropriate number of iterations of 1500 bytes sized amounts of data 
* Send the number of read() calls made as an acknowledgment to the client
* Close this connection
* Terminate the servicing
 thread 
The following command line arguments are required by client.cpp:

| argv[#] | Argument | Description |
| ------ | ------ | ------ |
| argv[1] | port | A server IP port |

Languages used: C++, C

#### Compilation & Running
***
Generate server executable:
```sh
g++ server.cpp -lpthread -o server
```
Generate client executable:
```sh
g++ client.cpp -o client
```

Run server from command line:

```sh
./server argv1
```
Run client from command line:

```sh
./client argv1 argv2 argv3 argv4 argv5 argv6
```
