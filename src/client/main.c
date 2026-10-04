/**-----------------------------------------------------------------------*
 * Amaze mini-game client
 *
 * Copyright (C) 2026 @kcraft059 - GPL v3
 *-----------------------------------------------------------------------**/

#include <unistd.h>

#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/socket.h>

#include <acp/acp.h>
#include <misc/misc.h>

static void eventHandler(struct sock_context* sock_ctx, enum sock_event type);

const struct programArgument args[] = {
    {.name = NULL, .description = NULL, .func = NULL}, // Signify end of array
};

int main(int argc, char** argv) {
  evalArgsContext(argv, argc, args);

  int client_sock_fd, queue_fd;
  struct sock_context* client_sock_ctx;
  if ((queue_fd = initQueue()) == -1)
    panicErrorf(errno, "Initiating queue");
  if ((client_sock_fd = initPeerSocket(ACP_DEFAULT_PORT, INADDR_LOOPBACK)) == -1)
    panicErrorf(errno, "Initiating peer socket");
  printLogf("Connected");
  if ((client_sock_ctx = addSockToQueue(queue_fd, client_sock_fd, T_LISTEN_SOCK)) == NULL)
    panicErrorf(errno, "Adding peer socket to queue");

  handleSockEvents(queue_fd, eventHandler, 10, &(struct timespec){30, 0});

  closeSockCtx(client_sock_ctx);
  close(queue_fd);
  return 0;
}

static void eventHandler(struct sock_context* sock_ctx, enum sock_event event) {
  char buf[32];
  recv(sock_ctx->fd, buf, sizeof(buf), 0);

  printLogf("Received \"%s\"", buf);
}
