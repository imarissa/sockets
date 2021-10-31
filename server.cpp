
/**
 * @author  Marissa Christenson, Dana Haberkorn
 * @since   05/30/21
 * server.cpp
 * 
 * This is the server.cpp file. It uses system calls and multiple threads. 
 * It consists of two functions: a thread_func and a main function. 
 * main is run by the main server thread which creates a socket 
 * with SOCK_STREAM protocol and binds this socket to a local address 
 * provided as input. This main server thread only listens for up to 5 
 * concurrent connections, and upon recieving a request for connection by a 
 * client, it accepts the request which returns a socket. 
 * 
 * It then creates a thread which enters the thread_func function with the 
 * socket descriptor as an argument, which it uses to communicate with the 
 * client via read and write system calls. It then recieves the number of 
 * repetitions the client will send a set of data buffers. For each rep, 
 * the server thread must read 1500 bytes of data and then return the number 
 * of read system calls it performed to complete the number of repetitions 
 * before closing the socket and exiting. 
 * 
 * This class takes a single argument that represents a port number. There are 
 * two global variables: BUFSIZE, the size of the buffer used to read from 
 * the socket and NUM_CONNECTIONS, the max number of concurrent connections 
 * the main server thread will listen for. 
 * 
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
using namespace std;

// Size of the buffer used to read from socket
const int BUFSIZE = 1500;

// Max number of concurrent connections main server thread listens for
const int NUM_CONNECTIONS = 5;

/**
 * The function called by server threads created by main server thread. 
 * They perform the appropriate number of reps of 1500 byte readings 
 * from the socket after recieving the number of iterations to perform. Then, 
 * they return the number of read system calls they performed before closing 
 * the socket and exiting. 
 * No other methods are called.
 * @param arg socket descriptor used to communicate with client
 * @return none
 * @custom.preconditions  must receive appropriate socket descriptor
 * @custom.postconditions  iterations of 1500 bytes read from socket
 **/
void *thread_func(void *arg) {

   int newSd = *((int *) arg);
   char dataBuf[BUFSIZE + 1] = {'\0'};
   int numReadCalls = 0;
   int bytesRead = 0;
   int temp = 0;

   // Read number of iterations to perform
   read(newSd, dataBuf, BUFSIZE);
   // Acknowledge number of iterations to perform
   write(newSd, dataBuf, strlen(dataBuf));
   int iterations = atoi(dataBuf);

   // Perform the appropriate number of iterations
   // Read 1500 bytes per iteration
   // Track number of read calls performed
   for (int i = 1; i <= iterations; i++) {
      bytesRead = 0;
      temp = 0;
      while (bytesRead < 1500) {
         if((temp = read(newSd, dataBuf, BUFSIZE - bytesRead)) != -1) {
            bytesRead += temp;
         }
         numReadCalls++;
      }
   }
   //cout<<"Num read calls: "<< numReadCalls<< endl;

   // Send number of read calls performed
   const char * numRead = to_string(numReadCalls).c_str();
   write(newSd, numRead, strlen(numRead));
   close(newSd);
   pthread_exit(nullptr);
}

/**
 * The main function has a main server thread that listens for requests 
 * to connect. It accepts these requests and receives a socket descriptor 
 * before it creates a new thread of execution that uses the socket descriptor 
 * to communicate with the client. The arguments passed to the function 
 * include the port number used.
 * Calls thread_func function 
 * @param argc number of arguments
 * @param argv array of program arguments
 * @return none
 * @custom.preconditions  must receive appropriate port number
 * @custom.postconditions  server created
 **/
int main(int argc, char *argv[])
{
   // Argument validation
   if (argc != 2) {
      cerr << "Usage: " << argv[0] << " #args" << endl;
      return -1;
   }

   int serverPort = atoi(argv[1]);
   int serverSd = -1;
   int rc;

   // Build address
   sockaddr_in acceptSocketAddress;
   bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
   acceptSocketAddress.sin_family = AF_INET;
   acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
   acceptSocketAddress.sin_port = htons(serverPort);

   // Open socket and bind
   serverSd = socket(AF_INET, SOCK_STREAM, 0);
   const int on = 1;
   setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
   //cout << "Socket #: " << serverSd << endl;


   if ((rc = bind(serverSd, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress))) < 0) {
      cerr << "Bind Failed" << endl;
      close(serverSd);
      return -1;
   }

   // Setting number of pending connections
   listen(serverSd, NUM_CONNECTIONS);       
   sockaddr_in newSockAddr;
   pthread_t tid;
   while (true) {
      socklen_t newSockAddrSize = sizeof(newSockAddr);
      int newSd;
      newSd = accept(serverSd, (sockaddr *) &newSockAddr, &newSockAddrSize);
      //cout << "Accepted Socket #: " << newSd <<endl;
      tid = pthread_create(&tid, NULL, thread_func, (void *)&newSd);
   }

   return 0;
}