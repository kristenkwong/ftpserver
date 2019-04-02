#include "dir.h"
#include "usage.h"
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.

void not_logged_in_response(int fd) {
  send(fd, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"), 0);
}

void syntax_error_response(int fd) {
  send(fd, "500 Syntax error, command unrecognized.\r\n",
       strlen("500 Syntax error, command unrecognized.\r\n"), 0);
}

void syntax_error_args_response(int fd) {
  send(fd, "501 Syntax error in parameters or arguments.\r\n",
       strlen("501 Syntax error in parameters or arguments.\r\n"), 0);
}

void command_okay_response(int fd) {
  send(fd, "200 Command okay.\r\n", strlen("200 Command okay.\r\n"), 0);
}

void parameter_not_supported_response(int fd) {
  send(fd, "504 Command not implemented for that parameter.\r\n",
       strlen("504 Command not implemented for that parameter.\r\n"), 0);
}

void file_action_okay_response(int fd) {
  send(fd, "250 Requested file action okay, completed.\r\n",
       strlen("250 Requested file action okay, completed.\r\n"), 0);
}

void requested_action_not_taken_response(int fd) {
  send(fd, "550 Requested action not taken.\r\n",
       strlen("550 Requested action not taken.\r\n"), 0);
}

int main(int argc, char *argv[]) {
  struct sockaddr_in address;
  // store the root directory
  char root_dir[256];
  getcwd(root_dir, sizeof(root_dir));
  printf("Root directory is: %s\n", root_dir);
  int port_num, socket_fd, new_socket_fd;
  int opt = 1;
  // user isn't logged in initially
  int logged_in = 0;
  char buffer[1024] = {0};
  // check the command line arguments
  if (argc != 2) {
    usage(argv[0]);
    return -1;
  }

  // get port number from argument passed in
  port_num = atoi(argv[1]);
  // check if port number is valid
  if (port_num < 1024 || port_num > 65535) {
    printf("Invalid port number.\n");
    return -1;
  }

  // create a socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Socket creation failed.\n");
    return -1;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_num);

  int address_len = sizeof(address);

  if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    printf("Failed to bind socket.\n");
    return -1;
  }

  if (listen(socket_fd, 1) == -1) {
    printf("Failed to listen to socket.\n");
    return -1;
  }

  if ((new_socket_fd = accept(socket_fd, (struct sockaddr *)&address,
                              (socklen_t *)&address_len)) == -1) {
    printf("Failed to accept.\n");
    return -1;
  }

  // send welcome message to client
  send(new_socket_fd, "220 Service ready for new user.\r\n",
       strlen("220 Service ready for new user.\r\n"), 0);
  while (1) {
    // clears the buffer
    memset(buffer, '\0', 1024);
    int data_len = recv(new_socket_fd, buffer, 1024, 0);
    if (data_len <= 0) {
      break;
    } else if (data_len > 1024)
    // overflow detection
    {
      syntax_error_response(new_socket_fd);
      continue;
    }

    char *token;
    int arg_count = 0;
    char delimit[] = " \t\r\n\v\f";

    // first string
    token = strtok(buffer, delimit);
    char *args[2];

    while (token != NULL && arg_count <= 1) {
      args[arg_count++] = token;
      token = strtok(NULL, delimit);
    }

    char *command = args[0];
    char *first_arg = args[1];

    printf("The first command: %s\n", command);
    printf("The first arg: %s\n", first_arg);
    printf("The number of args passed is: %d\n", arg_count);

    // START PROCESSING COMMANDS FROM THE USER
    // QUIT command
    if (strcasecmp(command, "quit") == 0) {
      if (arg_count != 1) {
        syntax_error_args_response(new_socket_fd);
        continue;
      }
      logged_in = 0;
      send(new_socket_fd, "221 Service closing control connection.\r\n",
           strlen("221 Service closing control connection.\r\n"), 0);
      close(new_socket_fd);
      // TODO: NOT SURE IF I NEED TO RETURN A 0 HERE TO END THE PROGRAM
      return 0;
    }

    if (logged_in == 0 && strcasecmp(command, "user") != 0) {
      // user is not logged in - send not logged in response
      not_logged_in_response(new_socket_fd);
      continue;
    } else {
      // USER command
      if (strcasecmp(command, "user") == 0) {
        if (arg_count != 2) {
          // if there's only 'user' and no username argument
          syntax_error_args_response(new_socket_fd);
          continue;
        }
        if (strcasecmp(first_arg, "cs317") == 0) {
          printf("%s\n", "Login was successful. Setting logged_in = 1");
          logged_in = 1;
          send(new_socket_fd, "230 User logged in, proceed.\r\n",
               strlen("230 User logged in, proceed.\r\n"), 0);
        } else {
          // if any other username is entered
          not_logged_in_response(new_socket_fd);
          continue;
        }
      } else if (strcasecmp(command, "cwd") == 0) {
        if (arg_count != 2) {
          // if there's no file path provided, send a syntax error response
          syntax_error_args_response(new_socket_fd);
          continue;
        }
        char curr_dir[256];
        getcwd(curr_dir, 256); 
        printf("The current directory is: %s\n", curr_dir);

        char *path_token;
        char *file_path;
        file_path = strdup(first_arg);

        char slash[1] = "/";
        path_token = strtok(first_arg, slash);
        while (path_token != NULL) {
          // return 550 response if file path contains ./ or ../
          if (strcmp(path_token, ".") == 0 || strcmp(path_token, "..") == 0) {
            requested_action_not_taken_response(new_socket_fd);
            goto next;
          }
          path_token = strtok(NULL, slash);
        }

        if (chdir(file_path) == -1) {
          // requested action could not be taken (invalid file path, dir doesn't exist)
          requested_action_not_taken_response(new_socket_fd);
          free(file_path);
          continue;
        } else {
          // requested action is okay - moved directories
          getcwd(curr_dir, 256); 
          printf("The current directory now is: %s\n", curr_dir);
          file_action_okay_response(new_socket_fd);
          free(file_path);
          continue;
        }
        next:
        continue;
      } else if (strcasecmp(command, "cdup") == 0) {
        // cdup command
        if (arg_count != 1) {
          // cdup should have no arguments
          syntax_error_args_response(new_socket_fd);
          continue;
        }

        char curr_dir[256];
        getcwd(curr_dir, 256); 
        printf("The current directory is: %s\n", curr_dir);
        if (strcmp(curr_dir, root_dir) == 0) {
          // send error response if CDUP is called in the root directory
          requested_action_not_taken_response(new_socket_fd);
          continue;
        } 

        if (chdir("..") == 0) {
          getcwd(curr_dir, 256); 
          printf("The current directory after is: %s\n", curr_dir);
          command_okay_response(new_socket_fd);
          continue;
        } else {
          requested_action_not_taken_response(new_socket_fd);
          continue;
        };

      } else if (strcasecmp(command, "type") == 0) {
        if (arg_count != 2) {
          // if there's only the 'type' argument
          syntax_error_args_response(new_socket_fd);
          continue;
        }
        char type = toupper(*first_arg);
        if (type == 'A' || type == 'I') {
          command_okay_response(new_socket_fd);
          continue;
        } else {
          parameter_not_supported_response(new_socket_fd);
          continue;
        }
      }
      // MODE command
      else if (strcasecmp(command, "mode") == 0) {
        if (arg_count != 2) {
          // if there's only the 'mode' argument
          syntax_error_args_response(new_socket_fd);
          continue;
        }
        char mode = toupper(*first_arg);
        if (mode == 'S')
        // only stream mode needs to be supported
        {
          command_okay_response(new_socket_fd);
          continue;
        } else {
          parameter_not_supported_response(new_socket_fd);
          continue;
        }
      }
      // STRU command
      else if (strcasecmp(command, "stru") == 0) {
        if (arg_count != 2) {
          // if there's only the 'stru' argument
          syntax_error_args_response(new_socket_fd);
          continue;
        }
        char stru = toupper(*first_arg);
        if (stru == 'F')
        // only File structure type needs to be supported
        {
          command_okay_response(new_socket_fd);
          continue;
        } else {
          parameter_not_supported_response(new_socket_fd);
          continue;
        }
      }
      // RETR command
      else if (strcasecmp(command, "retr") == 0) {
      }
      // PASV command
      else if (strcasecmp(command, "pasv") == 0) {
      }
      // NLST command
      else if (strcasecmp(command, "nlst") == 0) {
      } else {
        // command isn't one of the ones supported
        syntax_error_response(new_socket_fd);
      }
      continue;
    }
  }

  // This is how to call the function in dir.c to get a listing of a directory.
  // It requires a file descriptor, so in your code you would pass in the file
  // descriptor returned for the ftp server's data connection

  // printf("Printed %d directory entries\n", listFiles(1, "."));
  return 0;
}
