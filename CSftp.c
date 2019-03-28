#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include "dir.h"
#include "usage.h"

// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.

int main(int argc, char *argv[]) {
    // Check the command line arguments
    if (argc != 2) {
      usage(argv[0]);
      return -1;
    }

    // get port number from argument passed in 
    int port_num = atoi(argv[1]);
    // check if port number is valid
    if (port_num == 0) {
      printf("Invalid port number.\n");
      return -1;
    }
    printf("Port number: %d\n", port_num);

    struct addrinfo;
    // create a socket
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor == -1) {
      printf("Socket failed to be created.\n");
      return -1;
    }
    printf("Socket fd: %d\n", socket_descriptor);



    // This is how to call the function in dir.c to get a listing of a directory.
    // It requires a file descriptor, so in your code you would pass in the file descriptor 
    // returned for the ftp server's data connection
    
    // printf("Printed %d directory entries\n", listFiles(1, "."));
    return 0;

}
