#include "dir.h"
#include "usage.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
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

void send_response(int fd, int response_code) {
  char *response;
  switch (response_code) {
  case 125:
    response = "125 Data connection already open; transfer starting.\r\n";
    break;
  case 150:
    response = "150 File status okay; about to open data connection.\r\n";
    break;
  case 200:
    response = "200 Command okay.\r\n";
    break;
  case 220:
    response = "220 Service ready for new user.\r\n";
    break;
  case 226:
    response = "226 Closing data connection.\r\n";
    break;
  case 250:
    response = "250 Requested file action okay, completed.\r\n";
    break;
  case 421:
    response = "421 Service not available, closing control connection.\r\n";
    break;
  case 425:
    response = "425 Can't open data connection.\r\n";
    break;
  case 426:
    response = "426 Connection closed; transfer aborted.\r\n";
    break;
  case 450:
    response = "450 Requested file action not taken.\r\n";
    break;
  case 451:
    response = "451 Requested action aborted: local error in processing.\r\n";
    break;
  case 500:
    response = "500 Syntax error, command unrecognized.\r\n";
    break;
  case 501:
    response = "501 Syntax error in parameters or arguments.\r\n";
    break;
  case 504:
    response = "504 Command not implemented for that parameter.\r\n";
    break;
  case 530:
    response = "530 Not logged in.\r\n";
    break;
  case 550:
    response = "550 Requested action not taken.\r\n";
    break;
  }

  send(fd, response, strlen(response), 0);
}

void data_connection_already_open(int fd) {
  send(fd, "125 Data connection already open; transfer starting.\r\n",
       strlen("125 Data connection already open; transfer starting.\r\n"), 0);
}

void file_status_okay_response(int fd) {
  send(fd, "150 File status okay; about to open data connection.\r\n",
       strlen("150 File status okay; about to open data connection.\r\n"), 0);
}

void close_data_connection_response(int fd) {
  send(fd, "226 Closing data connection.\r\n",
       strlen("226 Closing data connection.\r\n"), 0);
}

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

void cant_open_data_connection_response(int fd) {
  send(fd, "425 Can't open data connection.\r\n",
       strlen("425 Can't open data connection.\r\n"), 0);
}

void service_not_available_response(int fd) {
  send(fd, "421 Service not available, closing control connection.\r\n",
       strlen("421 Service not available, closing control connection.\r\n"), 0);
}

void requested_file_action_not_taken_response(int fd) {
  send(fd, "450 Requested file action not taken.\r\n",
       strlen("450 Requested file action not taken.\r\n"), 0);
}

void requested_action_aborted_response(int fd) {
  send(fd, "451 Requested action aborted: local error in processing.\r\n",
       strlen("451 Requested action aborted: local error in processing.\r\n"),
       0);
}

// source:
// https://stackoverflow.com/questions/26951184/using-a-replace-function-in-c-programming
// function to replace periods with commas
int replace_periods(char *input) {
  for (char *p = input; *p; p++) {
    if (*p == '.')
      *p = ',';
  }
  return 1;
}

int main(int argc, char *argv[]) {
  struct sockaddr_in address;
  // store the root directory
  char root_dir[256];
  char *sock_ip_address;
  getcwd(root_dir, sizeof(root_dir));
  printf("Root directory is: %s\n", root_dir);
  int port_num, socket_fd, new_socket_fd;
  int pasv_port_num, new_pasv_fd, pasv_fd; // passive sockets
  int op = 1;

  // user isn't logged in initially
  int logged_in = 0;

  // not in passive mode initially
  int passive_mode = 0;

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

  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
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

  struct sockaddr_in sock_addr;
  socklen_t sock_addr_size = sizeof sock_addr;
  getsockname(new_socket_fd, (struct sockaddr *)&sock_addr, &sock_addr_size);
  sock_ip_address = inet_ntoa(sock_addr.sin_addr);
  printf("IPv4 Address: %s\n", sock_ip_address);

  // duplicate the IP address string, for use in PASV command
  char *dup_ip_address = strdup(sock_ip_address);
  replace_periods(dup_ip_address);

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
          // requested action could not be taken (invalid file path, dir doesn't
          // exist)
          requested_action_not_taken_response(new_socket_fd);
          free(file_path);
          continue;
        } else {
          // requested action is okay - moved directories
          getcwd(curr_dir, 256);
          file_action_okay_response(new_socket_fd);
          free(file_path);
          continue;
        }
      next:
        continue;
      }

      // CDUP COMMAND
      else if (strcasecmp(command, "cdup") == 0) {
        if (arg_count != 1) {
          // cdup should have no arguments
          syntax_error_args_response(new_socket_fd);
          continue;
        }

        char curr_dir[256];
        getcwd(curr_dir, 256);
        if (strcmp(curr_dir, root_dir) == 0) {
          // send error response if CDUP is called in the root directory
          requested_action_not_taken_response(new_socket_fd);
          continue;
        }

        if (chdir("..") == 0) {
          getcwd(curr_dir, 256);
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
        // TODO: check args
        struct sockaddr_in client_addr;

        if (passive_mode == 0) {
          send_response(new_socket_fd, 425);
          continue;
        } else if (logged_in == 0) {
          send_response(new_socket_fd, 530);
        } else {

          client_addr.sin_family = AF_INET;
          client_addr.sin_addr.s_addr = INADDR_ANY;
          client_addr.sin_port = htons(pasv_port_num);

          new_pasv_fd = accept(pasv_fd, (struct sockaddr *)&client_addr,
                               (socklen_t *)sizeof(client_addr));

          printf("%s\n", "accepted passive connection");
          int input_file;
          FILE *fp;
          long file_len;

          fp = fopen(first_arg, "rb"); // read file in bytes
          input_file = fileno(fp);     // convert to file descriptor

          FILE *testfile;
          testfile = fopen("testfile", "w+");
          int testfd;
          testfd = fileno(testfile);

          if (input_file < 0) { // no file found
            send_response(new_socket_fd, 550);
          } else {
            send_response(new_socket_fd, 125);

            while (1) {
              int bytes_read = read(input_file, buffer, sizeof(buffer));
              if (bytes_read == 0)
                break; // reached EOF
              else if (bytes_read < 0) {
                send_response(new_socket_fd, 426);
                break; // TODO HANDLE ERROR
              }

              void *p = buffer;
              while (bytes_read > 0) {
                int bytes_written = write(testfd, p, bytes_read);
                if (bytes_written <= 0) {
                  send_response(new_socket_fd, 426);
                }
                bytes_read -= bytes_written;
                p += bytes_written;
              }
            }
            send_response(new_socket_fd, 226);
          }
          close(new_pasv_fd);
          fclose(fp);
        }
      }
      // PASV command
      else if (strcasecmp(command, "pasv") == 0) {
        if (arg_count != 1) {
          // PASV should be the only argument
          syntax_error_args_response(new_socket_fd);
          continue;
        }

        // the following code is from the server.c provided on piazza with
        // slight modification source:
        // https://piazza.com/class/jq71qu0b3sj2pu?cid=582
        struct addrinfo hints, *servinfo, *p;
        struct sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size;

        int yes = 1;
        int rv;

        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_INET;       // IPv4 address
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        hints.ai_flags = AI_PASSIVE;     // fills in IP for me

        if ((rv = getaddrinfo(NULL, "0", &hints, &servinfo)) != 0) {
          // close connection if cannot get information for address
          service_not_available_response(new_socket_fd);
          exit(1);
        }

        // loop through all the results and bind to the first we can
        for (p = servinfo; p != NULL; p = p->ai_next) {
          // create socket object
          if ((pasv_fd = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
          }

          // specify that, once the program finishes, the port can be reused by
          // other processes
          if (setsockopt(pasv_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                         sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
          }

          // bind to the specified port number
          if (bind(pasv_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(pasv_fd);
            perror("server: bind");
            continue;
          }
          // if code reaches this point, the socket was properly created
          break;
        }

        freeaddrinfo(servinfo);
        // if p is null, the loop above could create a socket for any given
        // address
        if (p == NULL) {
          printf("failed to bind\n");
        }

        // setes up a queue of incoming connections to be received by the server
        if (listen(pasv_fd, 1) == -1) {
          printf("failed to listen listen\n");
          continue;
        }

        struct sockaddr_in pasv_addr;
        socklen_t pasv_addr_size = sizeof pasv_addr;
        getsockname(pasv_fd, (struct sockaddr *)&pasv_addr, &pasv_addr_size);
        printf("listening on: %d\n", pasv_fd);

        // convert to the correct byte order
        pasv_port_num = (int)ntohs(pasv_addr.sin_port);
        printf("%s%d\n", "port: ", pasv_port_num);
        int p1 = pasv_port_num / 256;
        int p2 = pasv_port_num % 256;

        char response_string[100];
        sprintf(response_string, "227 Entering Passive Mode (%s%s%d%s%d%s",
                dup_ip_address, ",", p1, ",", p2, ")\r\n");
        printf("testing again: %s\n", response_string);
        send(new_socket_fd, response_string, strlen(response_string), 0);
        passive_mode = 1;
      }
      // NLST command
      else if (strcasecmp(command, "nlst") == 0) {
        struct sockaddr_storage client_addr;

        if (passive_mode == 0) {
          cant_open_data_connection_response(new_socket_fd);
        } else if (logged_in == 0) {
          not_logged_in_response(new_socket_fd);
        } else if (arg_count > 1) { // param given
          syntax_error_args_response(new_socket_fd);
        } else {
          socklen_t sin_size;
          sin_size = sizeof(client_addr);
          new_pasv_fd = accept(pasv_fd, (struct sockaddr *)&client_addr,
                               &sin_size);
          
          printf("help: %d\n", new_pasv_fd);

          file_status_okay_response(new_socket_fd);
          int files = listFiles(new_pasv_fd, ".");
          if (files == -1) { // dir doesn't exist or no access
            requested_file_action_not_taken_response(new_socket_fd);
          } else if (files == -2) { // insufficient resources
            requested_action_aborted_response(new_socket_fd);
          } else {
            close_data_connection_response(new_socket_fd);
            close(new_pasv_fd);
            close(pasv_fd);

            passive_mode = 0;
          }
        }
      }

      else {
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
