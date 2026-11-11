/**-----------------------------------------------------------------------*
 * src/client/main.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <log.h>
#include <misc.h>
#include <network/event.h>
#include <network/protocol.h>

#include <arpa/inet.h>
#include <curses.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// const struct arg_def argl[] = {};

void handler(enum qevtype type, union qevdata data, [[maybe_unused]] qevent_t* ev_ctx) {
  char str[64];
  switch (type) {
  case EVT_READ:;
    memcpy(str, "EVT_READ", 9);
    break;
  case EVT_ERROR:;
    sprintf(str, "EVT_ERROR: %s", error_str(data.error));
    break;
  case EVT_RDHUP:
    memcpy(str, "EVT_RDHUP", 10);
    break;
  default:
    memcpy(str, "Other", 6);
    break;
  }
  print_logf("Event type: %s, data: %ld", str, data.count);
  if (!(type == EVT_READ && data.count > 0)) return;

  char buf[64];
  if (read(ev_ctx->ident, buf, sizeof(buf)) == -1) {
    print_errorf(errno, "Reading socket");
    return;
  }
  print_logf("Read: %s", buf);
}

int main(/*int argc, char** argv*/) {
  // eval_args(argv, argc, argl, sizeof(argl) / sizeof(const struct arg_def));
  DEBUG_LOGF("Debug enabled");
  print_logf("Amaze version: %s", V_STR);
  print_logf("Reported ncurse version: %s", curses_version());

#ifdef GPU_SUPPORT
  print_logf("Reported shader ext: %s", STR(SHADER_EXT));
#endif

  int sock, queue;
  qevent_t* event;
  if ((sock = init_peer_sock(ACP_DEFAULT_PORT, INADDR_LOOPBACK)) == -1) panic_errorf(errno, "Couldn't create socket");
  if ((queue = init_ev_queue()) == -1) panic_errorf(errno, "Couldn't create queue");
  if ((event = init_sock_ev(sock, handler, EVT_READ | EVT_RDHUP, NULL)) == NULL) panic_errorf(errno, "Creating event context");
  if (reg_ev(queue, event)) panic_errorf(errno, "Adding event to queue");

  if (handle_ev(queue, 10, NULL) == -1) panic_errorf(errno, "Handling events");

  return 0;
}
