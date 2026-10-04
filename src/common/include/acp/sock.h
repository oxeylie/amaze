/**-----------------------------------------------------------------------*
 * Sock management for acp over tcp
 *
 * Copyright (C) 2026 @oxeylie - GPL v3
 *-----------------------------------------------------------------------**/

#ifndef _ACP_NET_H
#define _ACP_NET_H

#include <time.h>

#if !defined(__linux__) && !defined(__APPLE__)
#error "Platform not supported"
#endif

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

int initQueue();
int initListenSocket(int port);
int initPeerSocket(int port, long adress);

struct sock_context* addSockToQueue(int queue_fd, int sock_fd, enum sock_type type);
int closeSockCtx(struct sock_context* sock_ctx);

int handleSockEvents(int queue_fd, event_handler handler, int max_events, struct timespec* timeout);
#endif // _ACP_NET_H
