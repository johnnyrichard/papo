#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>

#define SERVER_PORT 6667

int main() {
  struct sockaddr_in server;
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd == -1) {
    perror("Unable to create a socket");
    return EXIT_FAILURE;
  }

  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);

  int binderr = bind(server_fd, (struct sockaddr *) &server, sizeof(server));

  if (binderr == -1) {
    perror("Could not bind");
    return EXIT_FAILURE;
  }

  int listenerr = listen(server_fd, SOMAXCONN);

  if (listenerr == -1) {
    perror("Could not listen");
    return EXIT_FAILURE;
  }

  printf("Server listening at %d\n", SERVER_PORT);

  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

  if (client_fd == -1) {
    perror("Could not accept connections");
    exit(1);
    return EXIT_FAILURE;
  }

  char exit[] = "EXIT\r\n";

  while (true) {
    char client_buf[1024];

    memset(client_buf, 0, 1024);

    read(client_fd, client_buf, 1024);

    printf("RECEIVED: %s\n", client_buf);

    for (int i=0; client_buf[i] != 0; i++) {
      printf("%d-", (int) client_buf[i]);
    }

    putchar(0xa);

    for (int i = 0; exit[i] != 0; i++) {
      printf("%d-", (int)exit[i]);
    }

    putchar(0xa);

    if (!strncmp(client_buf, exit, 4)) {
      puts("Bye");

      close(server_fd);
      close(client_fd);

      return EXIT_SUCCESS;
    }

    write(client_fd, client_buf, 1024);
  }

  return EXIT_SUCCESS;
}
