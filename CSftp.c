#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include "dir.h"
#include "usage.h"

// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.

int main(int argc, char *argv[])
{
  struct sockaddr_in address;
  int port_num, socket_fd, new_socket_fd;
  int opt = 1;
  char buffer[1024] = {0};
  // check the command line arguments
  if (argc != 2)
  {
    usage(argv[0]);
    return -1;
  }

  // get port number from argument passed in
  port_num = atoi(argv[1]);
  // check if port number is valid
  if (port_num < 1024 || port_num > 65535)
  {
    printf("Invalid port number.\n");
    return -1;
  }

  // create a socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
  {
    printf("Socket creation failed.\n");
    return -1;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_num);

  int address_len = sizeof(address);

  if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
  {
    printf("Failed to bind socket.\n");
    return -1;
  }

  if (listen(socket_fd, 1) == -1)
  {
    printf("Failed to listen to socket.\n");
    return -1;
  }

  // while (1)
  // {
    if ((new_socket_fd = accept(socket_fd, (struct sockaddr *)&address, (socklen_t *)&address_len)) == -1)
    {
      printf("Failed to accept.\n");
      return -1;
    }
    int valread = read(new_socket_fd, buffer, 1024);
    printf("%s\n", buffer);
    printf("loop\n");
  // }

  // This is how to call the function in dir.c to get a listing of a directory.
  // It requires a file descriptor, so in your code you would pass in the file descriptor
  // returned for the ftp server's data connection

  // printf("Printed %d directory entries\n", listFiles(1, "."));
  return 0;
}
