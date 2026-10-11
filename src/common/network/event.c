/**-----------------------------------------------------------------------*
 * event.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <network/event.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#if defined(NET_USE_EPOLL)
#include <sys/epoll.h>
#elif defined(NET_USE_KQUEUE)
#include <sys/event.h>
#endif

const int16_t ALLOWED_EVT_FLAGS = EVT_READ | EVT_WRITE | EVT_USER | EVT_HUP;

static int _set_flags(int fd, int mask);
static qevent_t* _alloc_ev_ctx();

int init_listen_sock(int port, int max_clients) {
  int sock_fd;
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1; // Inits a TCP (ipv4) socket
  if (_set_flags(sock_fd, O_NONBLOCK) == -1) return -1;             // Set non-blocking state

  struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,                // Set type to TCP
      .sin_port = htons(port),              // Configure port
      .sin_addr.s_addr = htonl(INADDR_ANY), // Configure adress
  };
  if (bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) return -1; // Bind port to adress
  if (listen(sock_fd, max_clients) == -1) return -1;                                   // Listen on socket
  return sock_fd;
}

int init_peer_sock(int port, long address) {
  int sock_fd;
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1; // Inits a TCP (ipv4) socket
  if (_set_flags(sock_fd, O_NONBLOCK) == -1) return -1;             // Set non-blocking state

  struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,                // Set type to TCP
      .sin_port = htons(port),              // Configure port
      .sin_addr.s_addr = htonl(INADDR_ANY), // Configure adress
  };
  if (connect(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) return -1; // Connect to adress
  return sock_fd;
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
  qevent_t* sock_ctx;

  // Error if empty or unallowed event flags in filters or EVT_HUP alone
  if (filters == 0 || filters & ~(ALLOWED_EVT_FLAGS & ~EVT_USER) || !(filters & (ALLOWED_EVT_FLAGS & ~EVT_HUP & ~EVT_USER))) {
    errno = EINVAL;
    return NULL;
  }

  if ((sock_ctx = _alloc_ev_ctx()) == NULL) return NULL;
  *sock_ctx = (qevent_t){
      .ident = fd,
      .filters = filters,
      .handler = handler,
      .u_data = u_data};
  return sock_ctx;
}

inline void free_sock_ev(qevent_t* sock_ctx) {
  free(sock_ctx);
  return;
}

inline int init_ev_queue() {
#if defined(NET_USE_KQUEUE)
  // Create a kqueue instance
  return kqueue();

#elif defined(NET_USE_EPOLL)
  // Create an epoll instance
  return epoll_create1(0);
#endif
}

int reg_ev(int queue_fd, qevent_t* ev_ctx) {
  bool empty_filt = ev_ctx->filters == 0;
  bool unallowed_flags = (ev_ctx->filters & ~(ALLOWED_EVT_FLAGS)) != 0;
  bool no_primary = !(ev_ctx->filters & (ALLOWED_EVT_FLAGS & ~EVT_HUP));
  bool user_not_alone = !((ev_ctx->filters & EVT_USER) && !(ev_ctx->filters & (ALLOWED_EVT_FLAGS & ~EVT_USER)));
  if (empty_filt || unallowed_flags || no_primary || user_not_alone) { // Invalid arge if any of those conditions are verified
    errno = EINVAL;
    return -1;
  }

#if defined(NET_USE_KQUEUE)
  struct kevent events[3];
  int r, ev_n = 0;
  // Translate flags approprietly
  if (ev_ctx->filters & EVT_READ)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_READ, .flags = EV_ADD | EV_RECEIPT, .udata = ev_ctx};
  if (ev_ctx->filters & EVT_WRITE)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_WRITE, .flags = EV_ADD | EV_RECEIPT, .udata = ev_ctx};
  if (ev_ctx->filters & EVT_USER)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_USER, .flags = EV_ADD | EV_RECEIPT | EV_CLEAR, .udata = ev_ctx};

  // Add event to kqueue & return if error
  if ((r = kevent(queue_fd, events, ev_n, events, ev_n, &(struct timespec){0, 0})) == -1) return -1;

  for (int n = 0; n < r; n++)
    if ((int)events[n].data != 0) { // Check for error
      errno = (int)events[n].data;
      return -1;
    }

#elif defined(NET_USE_EPOLL)
  uint32_t filters = 0;
  // Translate flags approprietly
  if (ev_ctx->filters & EVT_READ) filters |= EPOLLIN;
  if (ev_ctx->filters & EVT_WRITE) filters |= EPOLLOUT;
  if (ev_ctx->filters & EVT_HUP) filters |= EPOLLRDHUP;
  if (ev_ctx->filters & EVT_USER) filters |= EPOLLIN;

  // Add event to epoll instance
  if (epoll_ctl(queue_fd, EPOLL_CTL_ADD, ev_ctx->ident, &(struct epoll_event){.events = filters, .data.ptr = ev_ctx}) == -1) return -1;

#endif
  return 0;
}

int unreg_ev(int queue_fd, qevent_t* ev_ctx) {
  bool empty_filt = ev_ctx->filters == 0;
  bool unallowed_flags = (ev_ctx->filters & ~(ALLOWED_EVT_FLAGS)) != 0;
  bool no_primary = !(ev_ctx->filters & (ALLOWED_EVT_FLAGS & ~EVT_HUP));
  bool user_not_alone = !((ev_ctx->filters & EVT_USER) && !(ev_ctx->filters & (ALLOWED_EVT_FLAGS & ~EVT_USER)));
  if (empty_filt || unallowed_flags || no_primary || user_not_alone) { // Invalid arge if any of those conditions are verified
    errno = EINVAL;
    return -1;
  }

#if defined(NET_USE_KQUEUE)
  struct kevent events[3];
  int r, ev_n = 0;
  // Translate flags approprietly
  if (ev_ctx->filters & EVT_READ)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_READ, .flags = EV_DELETE | EV_RECEIPT, .udata = ev_ctx};
  if (ev_ctx->filters & EVT_WRITE)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_WRITE, .flags = EV_DELETE | EV_RECEIPT, .udata = ev_ctx};
  if (ev_ctx->filters & EVT_USER)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_USER, .flags = EV_DELETE | EV_RECEIPT, .udata = ev_ctx};

  // Add event to kqueue & return if error
  if ((r = kevent(queue_fd, events, ev_n, events, ev_n, &(struct timespec){0, 0})) == -1) return -1;

  for (int n = 0; n < r; n++)
    if ((int)events[n].data != 0 && (int)events[n].data != ENOENT) { // Check for error excluding ENOENT
      errno = (int)events[n].data;
      return -1;
    }

#elif defined(NET_USE_EPOLL)
  if (epoll_ctl(queue_fd, EPOLL_CTL_DEL, ev_ctx->ident, NULL) == -1) return -1;

#endif
  return 0;
}

int handle_ev(int queue_fd, uint8_t max_events, struct timespec* timeout) {
  if (max_events == 0) {
    errno = EINVAL;
    return -1;
  }
  int r;

#if defined(NET_USE_KQUEUE)
  struct kevent events[max_events];
  if ((r = kevent(queue_fd, NULL, 0, events, max_events, timeout)) == -1) return -1;

  for (int n = 0; n < r; n++) {
    struct kevent ev = events[n];
    qevent_t* ctx = ev.udata;

    if (ev.filter == EVFILT_READ) // Read may receive 0 before HUP
      ctx->handler(EVT_READ, (union qevdata){0}, ctx);
    if (ev.filter == EVFILT_USER)
      ctx->handler(EVT_USER, (union qevdata){0}, ctx);
    if (ev.filter == EVFILT_WRITE)
      ctx->handler(EVT_WRITE, (union qevdata){0}, ctx);

    // Handle EOF if event is read (parity, on epoll write cannot report read end closed)
    if (ev.flags & EV_EOF && ev.filter == EVFILT_READ) {
      if (ev.fflags != 0) ctx->handler(EVT_ERROR, (union qevdata){.error = (int)ev.fflags}, ctx); // Notify error if socket didn't HUP properly
      if (ctx->filters & EVT_HUP) ctx->handler(EVT_HUP, (union qevdata){0}, ctx);                 // Notify HUP only if ctx subscribed
    }
  }

#elif defined(NET_USE_EPOLL)
  struct epoll_event events[max_events];
  if ((r = epoll_pwait2(queue_fd, events, max_events, timeout, NULL)) == -1) return -1;

  for (int n = 0; n < r; n++) {
    struct epoll_event ev = events[n];
    qevent_t* ctx = ev.data.ptr;

    if (ctx->filters & EVT_USER) {
      if (ev.events & EPOLLIN) { // Notify event only if there's data to be read
        uint64_t cnt;
        // Check + acknowledge event, if read successfull, call handler (equivalent as EV_CLEAR on kqueue)
        if (read(ctx->ident, &cnt, sizeof(cnt)) > 0) ctx->handler(EVT_USER, (union qevdata){0}, ctx);
      }
      continue;
    }

    if (ev.events & EPOLLIN) // Read may receive 0 before HUP
      ctx->handler(EVT_READ, (union qevdata){0}, ctx);
    if (ev.events & EPOLLOUT) // May not detect EPOLLHUP
      ctx->handler(EVT_WRITE, (union qevdata){0}, ctx);

    if (ev.events & EPOLLERR) { // Notify error if socket didn't HUP properly
      union qevdata data = {0};
      socklen_t len = sizeof(data.error);
      getsockopt(ctx->ident, SOL_SOCKET, SO_ERROR, &data.error, &len);
      ctx->handler(EVT_ERROR, data, ctx);
    }
    if (ev.events & (EPOLLRDHUP | EPOLLHUP) && ctx->filters & EVT_HUP) // Notify HUP only if ctx subscribed
      ctx->handler(EVT_HUP, (union qevdata){0}, ctx);
  }

#endif
  return r;
}

static inline int _set_flags(int fd, int mask) {
  int socket_flags;
  if ((socket_flags = fcntl(fd, F_GETFL)) == -1)
    return -1;
  if (fcntl(fd, F_SETFL, socket_flags |= mask) == -1)
    return -1;
  return 0;
}

static inline qevent_t* _alloc_ev_ctx() {
  return malloc(sizeof(qevent_t));
}
