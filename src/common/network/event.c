/**-----------------------------------------------------------------------*
 * event.c
 * 
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#if defined(NET_USE_EPOLL)
#include <sys/epoll.h>
#elif defined(NET_USE_KQUEUE)
#include <sys/event.h>
#endif

#include <network/event.h>

int init_listen_sock(int port) {
  return 0;
}
int init_peer_sock(int port, long address) {
  return 0;
}

qevent_t* init_user_ev(evhandler_t handler, void* u_data) {
  return NULL;
}
int trigger_user_ev(qevent_t* ev_ctx) {
  return 0;
}
int free_user_ev(qevent_t* ev_ctx) {
  return 0;
}

qevent_t* init_sock_ev(int fd, evhandler_t handler, int16_t filters, void* u_data) {
  return NULL;
}
void free_sock_ev(qevent_t* sock_ctx) {
  return;
}

int init_ev_queue() {
  return 0;
}
int reg_ev(int queue_fd, qevent_t* ev_ctx) {
  return 0;
}
int unreg_ev(int queue_fd, qevent_t* ev_ctx) {
  return 0;
}
int handle_ev(int queue_fd, int max_events, struct timespec* timeout) {
  return 0;
}
