
/**
 * @author  Marissa Christenson, Dana Haberkorn
 * @since   05/30/21
 * client.cpp
 * 
 * This is the client.cpp file. It consists of a main function that creates 
 * a socket and then requests to connect to the multi-threaded server. 
 * Upon connecting, it writes the number of repetitions of the test that 
 * will be performed to the server. 
 * 
 * It takes 6 arguments: the server name that it will connect to, the port 
 * number used by the server, the number of repetitions of sending a set of 
 * data buffers, the number of data buffers, and size of each data buffer 
 * and the type of test to be performed. 
 * 
 * There are three test types. Each have a different mechanism to accomplish 
 * throughout. Test type 1 invokes the write system call for each data buffer 
 * and calls write as many times as there are number of data buffers. Test 
 * type 2 allocates an array of iovec data structures and calls writev to send 
 * all data buffers at once. Test type 3 is a single write that allocates one 
 * buffer of the appropriate size and calls write once to send this buffer of 
 * data. 
 * 
 * The program times the process and receives the number of read calls the 
 * server performs. Using this data, it calculates the throughput for each 
 * mechanism. It sends the test type, time to send, #reads, and throughput 
 * to cout. 
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
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <regex>
#include <cmath>
using namespace std;

/**
 * Creates a socket and requests connection to the server before sending the 
 * number of repetitions of a test to be performed and evaluating the 
 * throughput of each mechanism when using TCP/IP do to do point-to-
 * point communication over a network. argv holds 6 arguments: the server name 
 * port number of the server, the reps of sending a set of data buffers, 
 * the number of data buffers, the size of each data buffer, and the test type.
 * No functions internal to this file are called. 
 * @param argc number of arguments
 * @param argv array of program arguments
 * @return none
 * @custom.preconditions  must receive appropriate arguments
 * @custom.postconditions  evaluate throughput when using TCP/IP 
 */
int main(int argc, char *argv[])
{
   // Argument validation
   if (argc != 7) {
      cerr << "Usage: " << argv[0] << " #args" << endl;
      return -1;
   }

   char * serverName = argv[1];
   char * clientPort = argv[2];
   int reps = atoi(argv[3]);
   int nbufs = atoi(argv[4]);
   int bufsize = atoi(argv[5]);
   int type = atoi(argv[6]);

   int clientSd = -1;
   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int rc;

   if (reps <= 0 || (nbufs * bufsize != 1500)) {
      cout<<"Fail at args: repetition, nbufs, bufsise"<<endl;
      return -1;
   }else if (!regex_match (argv[6], regex("^([1-3])$"))) {
      cout<<"Fail at args: type"<<endl;
      return -1;
   }

   memset(&hints, 0, sizeof(hints));
   // Allow IPv4 or IPv6, use AF_INET6 to force IPv6
   hints.ai_family = AF_UNSPEC;
   // TCP
   hints.ai_socktype = SOCK_STREAM;		
   // Optional Options
   hints.ai_flags = 0;
   // Allow any protocol
   hints.ai_protocol = 0;
   
   if ((rc = getaddrinfo(serverName, clientPort, &hints, &result)) != 0) {
      cerr << "ERROR: " << gai_strerror(rc) << endl;
      exit(EXIT_FAILURE);
   }

   // Iterate through addresses and connect
   for (rp = result; rp != nullptr; rp = rp->ai_next) {

      if ((clientSd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
         continue;
      }

      // A socket has been successfully created
      if ((rc = connect(clientSd, rp->ai_addr, rp->ai_addrlen)) < 0) {
         cerr << "Connection Failed" << endl;
         close(clientSd);
         return -1;
      }else {
         break;
      }
   }

   if (!rp) {
      cerr << "No valid address" << endl;
      exit(EXIT_FAILURE);
   }else {
      //cout << "Client Socket: " << clientSd << endl;
   }
   
   freeaddrinfo(result);
   char databuf[nbufs][bufsize] = {'\0'};
   // Send the number of repetitions through socket
   write(clientSd, argv[3], strlen(argv[3]));
   // Read acknowledgment of iterations 
   read(clientSd, databuf, strlen(argv[3]));
   int repetitions = reps;
   chrono::system_clock::time_point startTp;
   
   // The different types of test processes
   switch (type) {
      case 1:
         startTp = chrono::system_clock::now(); 
         while (repetitions > 0) {
            for (int i = 0; i < nbufs; i++) { 
               write(clientSd, databuf[i], bufsize);
            }
            repetitions--;
         }
         break;

      case 2:
      {
         struct iovec vector[nbufs]; 
         for (int i = 0; i < nbufs; i++) { 
            vector[i].iov_base = databuf[i]; 
            vector[i].iov_len = bufsize; 
         } 
         startTp = chrono::system_clock::now();
         while (repetitions > 0) {
            writev(clientSd, vector, nbufs);            
            repetitions--;
         }
      }
         break;
         
      case 3:
         startTp = chrono::system_clock::now();
         while (repetitions > 0) {
            write(clientSd, databuf, nbufs * bufsize);
            repetitions--;
         }
         break;
   }
   chrono::system_clock::time_point endTp = chrono::system_clock::now();

   // Number of reads performed by server 
   read(clientSd, databuf, nbufs * bufsize);
   int numReads = atoi(*databuf);

   cout << "Test " << type << ": Time = " << chrono::duration_cast<std::chrono::microseconds>(endTp - startTp).count();
   cout << " usec #reads = " << numReads << " Throughput: ";
   cout << (((nbufs * bufsize * reps) * (7.451*(pow(10, -9))))) / (chrono::duration_cast<std::chrono::microseconds>(endTp - startTp).count() * (1.0*pow(10, -6)))
   << " Gbps" << endl;

   return close(clientSd);
}