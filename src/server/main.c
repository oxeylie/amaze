/**-----------------------------------------------------------------------*
 * Amaze mini-game server
 *
 * Copyright (C) 2026 @oxeylie - GPL v3
 *-----------------------------------------------------------------------**/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/socket.h>

#include <acp/acp.h>
#include <misc/misc.h>

#define TICK_FREQ 20

// Local declarations

static int argSetPort(char** argv, int argc);
static void eventHandler(struct sock_context* sock_ctx, enum sock_event event);

uint16_t port = ACP_DEFAULT_PORT;
const struct programArgument args[] = {
    {.name = "-p", .description = "set listening port", .func = argSetPort},
    {.name = NULL, .description = NULL, .func = NULL}, // Signify end of array
};

// Global functions

int main(int argc, char** argv) {
  evalArgsContext(argv, argc, args);

  int queue_fd, server_sock_fd;
  struct sock_context* server_sock_ctx;

  if ((queue_fd = initQueue()) == -1)
    panicErrorf(errno, "Initiating queue");
  if ((server_sock_fd = initListenSocket(port)) == -1)
    panicErrorf(errno, "Seting up listen socket on port %d", port);
  if ((server_sock_ctx = addSockToQueue(queue_fd, server_sock_fd, T_LISTEN_SOCK)) == NULL)
    panicErrorf(errno, "Adding server sock to queue");

  handleSockEvents(queue_fd, eventHandler, 10, &(struct timespec){30, 0});

  closeSockCtx(server_sock_ctx);
  close(queue_fd);

  return 0;
}

// Local functions

static void eventHandler(struct sock_context* sock_ctx, enum sock_event event) { // WIP
  if (sock_ctx->type != T_LISTEN_SOCK)
    return;

  int client_sock_fd;
  while ((client_sock_fd = accept(sock_ctx->fd, NULL, NULL)) != -1) {
		printLogf("Accepting client");
    send(client_sock_fd, "Hello !", 8, 0);
    close(client_sock_fd);
  }

  if (errno != EWOULDBLOCK && errno != EAGAIN)
    printErrorf(errno, "Accepting incomming connection");
};

static int argSetPort(char** argv, int argc) {
  int val, n, pos;
  if (argc < 1) {
    errno = EINVLPRM; // Check if there's enough params
    return -1;
  }

  n = sscanf(argv[0], "%d%n", &val, &pos);
  if (n != 1 || pos != strlen(argv[0])) {
    errno = EINVLPRM; // Fail if invalid input
    return -1;
  }
  port = val & 0xffff; // Update port with mask

  return 1; // Return number of params used
};
