#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include "dir.h"
#include "usage.h"

// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.

void not_logged_in_response(int fd)
{
  send(fd, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"), 0);
}

void syntax_error_response(int fd)
{
  send(fd, "500 Syntax error, command unrecognized.\r\n",
       strlen("500 Syntax error, command unrecognized.\r\n"), 0);
}

void syntax_error_args_response(int fd)
{
  send(fd, "501 Syntax error in parameters or arguments.\r\n", strlen("501 Syntax error in parameters or arguments.\r\n"), 0);
}

int main(int argc, char *argv[])
{
  struct sockaddr_in address;
  int port_num, socket_fd, new_socket_fd;
  int opt = 1;
  // user isn't logged in initially
  int logged_in = 0;
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

  if ((new_socket_fd = accept(socket_fd, (struct sockaddr *)&address, (socklen_t *)&address_len)) == -1)
  {
    printf("Failed to accept.\n");
    return -1;
  }

  // send welcome message to client
  send(new_socket_fd, "220 Service ready for new user.\r\n", strlen("220 Service ready for new user.\r\n"), 0);
  while (1)
  {
    int data_len = recv(new_socket_fd, buffer, 1024, 0);
    printf("Data length: %d\n", data_len);
    if (data_len <= 0)
    {
      continue;
    }
    else if (data_len > 1024)
    // overflow detection
    {
      syntax_error_response(new_socket_fd);
      continue;
    }
    char *command;
    char delimit[] = " \t\r\n\v\f";

    // first string
    // TODO: null check if there's an empty line recevied from the client
    command = strtok(buffer, delimit);

    // loop through all the tokens
    // while (token != NULL)
    // {
    //   printf(" %s\n", token);
    //   token = strtok(NULL, s);
    // }

    printf("The first command: %s\n", command);
    // TODO: should probably implement a more robust way of checking for arg count
    // START PROCESSING COMMANDS FROM THE USER
    if (logged_in == 0 && strcasecmp(command, "user") != 0)
    {
      // user is not logged in - don't process commands
      not_logged_in_response(new_socket_fd);
      continue;
    }
    else
    {
      if (strcasecmp(command, "user") == 0)
      {
        char *second_arg;
        if (command != NULL)
        {
          // get the second argument
          second_arg = strtok(NULL, delimit);
          printf("The second arg: %s\n", second_arg);
          if (second_arg == NULL)
          {
            // if you enter user and there's no other arguments (NULL) 
            syntax_error_args_response(new_socket_fd);
            continue;
          }
        }
        if (strcasecmp(second_arg, "cs317") == 0)
        {
          printf("%s\n", "Login was successful. Setting logged_in = 1");
          logged_in = 1;
          send(new_socket_fd, "230 User logged in, proceed.\r\n", strlen("230 User logged in, proceed.\r\n"), 0);
        }
        else
        {
          not_logged_in_response(new_socket_fd);
          continue;
        }
      }
      else if (strcasecmp(command, "quit") == 0)
      {
        logged_in = 0;
        send(new_socket_fd, "221 Service closing control connection.\r\n", 
                strlen("221 Service closing control connection.\r\n"), 0);
        close(new_socket_fd);
        // TODO: NOT SURE IF I NEED TO RETURN A 0 HERE TO END THE PROGRAM
        return 0;
      }
      else if (strcasecmp(command, "cwd") == 0)
      {
      }
      else if (strcasecmp(command, "cdup") == 0)
      {
      }
      else if (strcasecmp(command, "type") == 0)
      {
      }
      else if (strcasecmp(command, "mode") == 0)
      {
      }
      else if (strcasecmp(command, "stru") == 0)
      {
      }
      else if (strcasecmp(command, "retr") == 0)
      {
      }
      else if (strcasecmp(command, "pasv") == 0)
      {
      }
      else if (strcasecmp(command, "nlst") == 0)
      {
      }
      else
      {
        // command isn't one of the ones supported
        syntax_error_response(new_socket_fd);
      }
      continue;
    }
  }

  // This is how to call the function in dir.c to get a listing of a directory.
  // It requires a file descriptor, so in your code you would pass in the file descriptor
  // returned for the ftp server's data connection

  // printf("Printed %d directory entries\n", listFiles(1, "."));
  return 0;
}
