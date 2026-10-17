/**-----------------------------------------------------------------------*
 * src/server/main.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <log.h>
#include <misc.h>
#include <network/event.h>
#include <network/protocol.h>

#include <sys/socket.h>
void handler([[maybe_unused]] enum qevtype type, [[maybe_unused]] union qevdata data, qevent_t* ev_ctx) {
  print_logf("Received connection request");

  int client_fd;
  if ((client_fd = accept(ev_ctx->ident, NULL, NULL)) == -1)
    panic_errorf(errno, "Accepting incoming connection");
  if (write(client_fd, "Hello", 6) == -1)
    panic_errorf(errno, "Writing to socket");
  if (close(client_fd) == -1)
    panic_errorf(errno, "Closing socket");
};

const struct arg_def argl[] = {};

int main(int argc, char** argv) {
  eval_args(argv, argc, argl, sizeof(argl) / sizeof(const struct arg_def));

  int sock, queue;
  qevent_t* event;
  if ((sock = init_listen_sock(ACP_DEFAULT_PORT, 10)) == -1)
    panic_errorf(errno, "Couldn't create listen socket");
  if ((queue = init_ev_queue()) == -1)
    panic_errorf(errno, "Couldn't create queue");
  if ((event = init_sock_ev(sock, handler, EVT_READ, NULL)) == NULL)
    panic_errorf(errno, "Creating event context");
  if (reg_ev(queue, event))
    panic_errorf(errno, "Adding event to queue");
  if (handle_ev(queue, 10, NULL) == -1)
    panic_errorf(errno, "Handling events");

  print_logf("Server halted");
  return 0;
}
