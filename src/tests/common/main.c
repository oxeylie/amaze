/**-----------------------------------------------------------------------*
 * src/tests/common/main.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <log.h>
#include <misc.h>
#include <network/event.h>
#include <network/protocol.h>

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

int qevent_test();

int main() {
  printf("\n");
  print_logf("Info: Launching tests for common");
  unsigned int failed_tests = 0;

  failed_tests += qevent_test();

  if (failed_tests > 0) print_errorf(0, "Fail: %u tests failed", failed_tests);
  else print_logf("Success: All tests passed");
  return failed_tests > 0 ? 1 : 0;
}

// Qevent Test

volatile bool server_is_launched = false;
volatile bool client_should_close = false;
volatile bool thread_closed = false;
volatile bool qevent_failed = false;

void server_handler(enum qevtype type, union qevdata data [[maybe_unused]], qevent_t* ev_ctx) {
  DEBUG_LOGF("Listen sock: Event type: %d, data: %ld", type, data.count);
  if (type != EVT_READ) {
    qevent_failed = true;
    return;
  }
  int fd;
  if ((fd = accept(ev_ctx->ident, NULL, 0)) == -1) {
    print_errorf(errno, "Accepting connection");
    qevent_failed = true;
    return;
  }
  if (write(fd, "Hello", sizeof(char) * 6) == -1) {
    print_errorf(errno, "Writing socket");
    qevent_failed = true;
  }
  if (close(fd) == -1) {
    print_errorf(errno, "Closing socket");
    qevent_failed = true;
  }
  return;
}
void client_handler(enum qevtype type, union qevdata data [[maybe_unused]], qevent_t* ev_ctx) {
  DEBUG_LOGF("Peer sock: Event type: %d, data: %ld", type, data.count);
  if (type == EVT_READ) {
    char buf[64];
    if (read(ev_ctx->ident, buf, 64 * sizeof(char)) == -1) {
      print_errorf(errno, "Reading sock");
      qevent_failed = true;
      return;
    };
    DEBUG_LOGF("Received: \"%s\"", buf);
  } else if (type == EVT_RDHUP)
    client_should_close = true;
  else
    qevent_failed = true;

  return;
}
void* server_thread_init(void* arg [[maybe_unused]]) {
  int listen_fd, queue_fd;
  qevent_t* event;
  if ((queue_fd = init_ev_queue()) == -1) {
    print_errorf(errno, "Creating queue");
    qevent_failed = true;
    goto return_;
  }
  if ((listen_fd = init_listen_sock(ACP_DEFAULT_PORT, 10)) == -1) {
    print_errorf(errno, "Opening listen port on %d", ACP_DEFAULT_PORT);
    qevent_failed = true;
    goto close_queue;
  }
  if ((event = init_sock_ev(listen_fd, server_handler, EVT_READ, NULL)) == NULL) {
    print_errorf(errno, "Creating sock event");
    qevent_failed = true;
    goto close_event;
  }
  if (reg_ev(queue_fd, event) == -1) {
    print_errorf(errno, "Registering sock event");
    qevent_failed = true;
    goto free_event;
  }
  DEBUG_LOGF("Server arrived");
  server_is_launched = true;
  if (handle_ev(queue_fd, 10, &(struct timespec){1, 0}) <= 0) {
    print_errorf(errno, "Error handling event / No event arrived");
    qevent_failed = true;
  }
  if (unreg_ev(event) == -1) {
    print_errorf(errno, "Registering sock event");
    qevent_failed = true;
  };
free_event:
  if (free_sock_ev(event) == -1) {
    print_errorf(errno, "Freeing sock event");
    qevent_failed = true;
  };
close_event:
  close(listen_fd);
close_queue:
  close(queue_fd);
return_:
  return NULL;
};
int qevent_test() {
  int count = 0;
  pthread_t server_thread;
  pthread_create(&server_thread, NULL, server_thread_init, NULL);

  int bind_fd, queue_fd;
  qevent_t* event;
  while (!server_is_launched)
    if (qevent_failed) {
      qevent_failed = true;
      goto return_;
    }
  if ((queue_fd = init_ev_queue()) == -1) {
    print_errorf(errno, "Creating queue");
    qevent_failed = true;
    goto return_;
  }
  if ((bind_fd = init_peer_sock(ACP_DEFAULT_PORT, INADDR_LOOPBACK)) == -1) {
    print_errorf(errno, "Connecting to %#08xf", INADDR_LOOPBACK);
    qevent_failed = true;
    goto close_queue;
  }
  if ((event = init_sock_ev(bind_fd, client_handler, EVT_READ | EVT_RDHUP, NULL)) == NULL) {
    print_errorf(errno, "Creating sock event");
    qevent_failed = true;
    goto close_event_fd;
  }

  if (reg_ev(queue_fd, event) == -1) {
    print_errorf(errno, "Registering event in queue");
    qevent_failed = true;
    goto free_ev;
  }

  if (handle_ev(queue_fd, 10, &(struct timespec){1, 0}) <= 0) {
    print_errorf(errno, "Error handling event / No event arrived");
    qevent_failed = true;
  }

  if (!client_should_close)
    qevent_failed = true;

  if (unreg_ev(event) == -1) {
    print_errorf(errno, "Unregistering event in queue");
    qevent_failed = true;
    event->queue = -1;
  }
free_ev:
  if (free_sock_ev(event) == -1) {
    print_errorf(errno, "Freeing sock event");
    qevent_failed = true;
  };
close_event_fd:
  close(bind_fd);
close_queue:
  close(queue_fd);
return_:
  if (qevent_failed == true) count++;
  return count;
}
