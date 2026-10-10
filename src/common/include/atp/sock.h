/**-----------------------------------------------------------------------*
 * Sock management for atp over tcp
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#ifndef _ACP_SOCK_H
#define _ACP_SOCK_H
#if !defined(__linux__) && !defined(__APPLE__)
#error "Platform not supported"
#endif

#include <time.h>

#define MAX_QUEUED_CLIENTS 10

enum sock_type {
  T_LISTEN_SOCK = 0,
  T_PEER_SOCK
};

enum sock_event {
  SEV_READ = 0,
  SEV_RESET,
  SEV_ERR,
  SEV_UNKNOWN
};

struct sock_context {
  enum sock_type type;
  int fd;
  void* data;
};

typedef void (*event_handler)(struct sock_context* sock_ctx, enum sock_event event);

int init_queue();
int init_listen_sock(int port);
int init_peer_socket(int port, long adress);

struct sock_context* add_sock_to_queue(int queue_fd, int sock_fd, enum sock_type type);
int close_sock_ctx(struct sock_context* sock_ctx);

int handle_sock_event(int queue_fd, event_handler handler, int max_events, struct timespec* timeout);
#endif // _ACP_SOCK_H
