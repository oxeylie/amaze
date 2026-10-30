/**-----------------------------------------------------------------------*
 * src/common/network/event.c
 *
 * Todo:
 * - Make thread safe
 * - Add free_queue
 * - Remove socket creation functions
 * - Implement delayed unregistration
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <network/event.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#if defined(NET_USE_KQUEUE)
#include <sys/event.h>
#elif defined(NET_USE_EPOLL)
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

#if defined(NET_USE_KQUEUE)
#define EV_ID_TBL_SIZE 8
#define EV_ID_TBL_SLOTS (EV_ID_TBL_SIZE * 8)
static uint8_t event_id_table[EV_ID_TBL_SIZE];
#endif
static const uint8_t allowed_EVT_flags = EVT_READ | EVT_WRITE | EVT_USER | EVT_RDHUP;

static int set_sock_flags(int fd, int mask);
static qevent_t* alloc_ev_ctx();

int init_listen_sock(int port, size_t max_clients) { // Todo: change / delete implementation to allow for more modularity
  int sock_fd;
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) goto err_return;                          // Inits a TCP (ipv4) socket
  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) goto err_close; // Prevent interfering with socks in TIMEWAIT
  struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,                // Set type to TCP
      .sin_port = htons(port),              // Configure port
      .sin_addr.s_addr = htonl(INADDR_ANY), // Configure adress
  };
  if (bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) goto err_close; // Bind port to adress
  if (listen(sock_fd, max_clients) == -1) goto err_close;                                   // Listen on socket

  if (set_sock_flags(sock_fd, O_NONBLOCK) == -1) goto err_close; // Set non-blocking state
  return sock_fd;

err_close:; // Error handling
  int errno_before = errno;
  close(sock_fd);
  errno = errno_before;
err_return:
  return -1;
}

int init_peer_sock(int port, long address) { // Todo: change / delete implementation to allow for more modularity
  int sock_fd;
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) goto err_return; // Inits a TCP (ipv4) socket
  struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,             // Set type to TCP
      .sin_port = htons(port),           // Configure port
      .sin_addr.s_addr = htonl(address), // Configure adress
  };
  if (connect(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) goto err_close; // Connect to adress

  if (set_sock_flags(sock_fd, O_NONBLOCK) == -1) goto err_close; // Set non-blocking state
  return sock_fd;

err_close:; // Error handling
  int errno_before = errno;
  close(sock_fd); // We don't care about close errors but they can overwrite errno
  errno = errno_before;
err_return:
  return -1;
}

qevent_t* init_user_ev(evhandler_t handler, void* u_data) {
  qevent_t* sock_ctx = NULL;
  int ident;

#if defined(NET_USE_KQUEUE)
  for (ident = 0; ident < EV_ID_TBL_SLOTS; ident++)     // Select a suitable ident
    if (event_id_table[ident / 8] & (1 << (ident % 8))) // Check if bit is set ot know if ident is avail
      continue;
    else {
      event_id_table[ident / 8] |= (1 << (ident % 8)); // Set bit to indicate allocation
      break;
    }

  if (ident >= EV_ID_TBL_SLOTS) { // If not enough slots for user id
    errno = ENOSPC;               // Not enough space
    goto err_return;
  }

#elif defined(NET_USE_EPOLL)
  if ((ident = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) == -1) // Init with 0 in counter
    goto err_return;

#endif
  if ((sock_ctx = alloc_ev_ctx()) == NULL) goto err_close;
  *sock_ctx = (qevent_t){
      .ident = ident,
      .queue = -1,
      .filters = EVT_USER,
      .handler = handler,
      .u_data = u_data,
  };
  return sock_ctx;

err_close:
#if defined(NET_USE_KQUEUE)
  event_id_table[ident / 8] &= ~(1 << (ident % 8)); // Clear bit
#elif defined(NET_USE_EPOLL)
  close(ident);
#endif
err_return:
  return NULL;
}

int trigger_user_ev(qevent_t* ev_ctx) {
  if (ev_ctx->queue == -1) {
    errno = ENOENT; // Event queue invalid
    return -1;
  }

#if defined(NET_USE_KQUEUE)
  struct kevent* ev = &(struct kevent){
      .ident = ev_ctx->ident,
      .filter = EVFILT_USER,
      .flags = 0,
      .fflags = NOTE_TRIGGER,
      .udata = ev_ctx,
  };

  if (kevent(ev_ctx->queue, ev, 1, NULL, 0, NULL) == -1) {
    if (errno == EBADF) errno = ENOENT; // Queue is gone
    return -1;
  }

#elif defined(NET_USE_EPOLL)
  ssize_t n;
  do
    n = write(ev_ctx->ident, &(uint64_t){1}, sizeof(uint64_t));
  while ((n == -1 && errno == EINTR) || (n < sizeof(uint64_t) && errno == EINVAL)); // Retry on interrupts & partial writes
  if (n == -1) return -1;

#endif
  return 0;
};

int free_user_ev(qevent_t* ev_ctx) {
  if (ev_ctx->queue != -1) {
    errno = EBUSY;
    return -1;
  }

#if defined(NET_USE_KQUEUE)
  event_id_table[ev_ctx->ident / 8] &= ~(1 << (ev_ctx->ident % 8)); // Mark ident as free on table
  free(ev_ctx);                                                     // Free context object

#elif defined(NET_USE_EPOLL)
  close(ev_ctx->ident); // We don't care about close error since the fd is closed either way
  free(ev_ctx);         // Free context object

#endif
  return 0;
}

qevent_t* init_sock_ev(int fd, evhandler_t handler, int16_t filters, void* u_data) {
  qevent_t* sock_ctx;

  if (filters == 0 ||                                   // Check for empty filter
      filters & ~(allowed_EVT_flags & ~EVT_USER) ||     // Check for forbidden flags
      !(filters & (allowed_EVT_flags & ~EVT_USER)) ||   // Check that at least one of the main EVT_Flags is present
      (filters & EVT_RDHUP && !(filters & EVT_READ))) { // Check for EVT_RDHUP without EVT_READ
    errno = EINVAL;
    return NULL;
  }

  if ((sock_ctx = alloc_ev_ctx()) == NULL) return NULL;
  *sock_ctx = (qevent_t){
      .ident = fd,
      .queue = -1,
      .filters = filters,
      .handler = handler,
      .u_data = u_data};
  return sock_ctx;
}

int free_sock_ev(qevent_t* sock_ctx) {
  if (sock_ctx->queue != -1) {
    errno = EBUSY;
    return -1;
  }

  free(sock_ctx);
  return 0;
}

int init_ev_queue() {
#if defined(NET_USE_KQUEUE)
  // Create a kqueue instance
  return kqueue();

#elif defined(NET_USE_EPOLL)
  // Create an epoll instance
  return epoll_create1(0);
#endif
}

int reg_ev(int queue_fd, qevent_t* ev_ctx) {
  if (ev_ctx->queue != -1) {
    errno = EEXIST;
    return -1;
  }

#if defined(NET_USE_KQUEUE)
  struct kevent events[3], receipts[3];
  int ev_n = 0, rcp_n = 0, err_n = 0, prev_errno;

  // Translate flags approprietly
  if (ev_ctx->filters & EVT_READ)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_READ, .flags = EV_ADD | EV_RECEIPT, .udata = ev_ctx};
  if (ev_ctx->filters & EVT_WRITE)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_WRITE, .flags = EV_ADD | EV_RECEIPT, .udata = ev_ctx};
  if (ev_ctx->filters & EVT_USER)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_USER, .flags = EV_ADD | EV_RECEIPT | EV_CLEAR, .udata = ev_ctx};

  if (ev_n == 0) { // Verify that there are events to add
    errno = EINVAL;
    return -1;
  }

  // In case of error on the queue itself
  if ((rcp_n = kevent(queue_fd, events, ev_n, receipts, ev_n, &(struct timespec){0, 0})) == -1) {
    if (errno == EBADF) errno = ENOENT; // Queue is gone
    return -1;
  }

  ev_n = 0;                       // Reset ev_n and use as undo counter
  for (int n = 0; n < rcp_n; n++) // Check receipts for errors
    if (receipts[n].data != 0) {  // If error
      prev_errno = receipts[n].data;
      err_n++;
    } else // If socket didn't have any error, mark for potential undo
      events[ev_n++] = (struct kevent){
          .ident = receipts[n].ident,
          .filter = receipts[n].filter,
          .flags = EV_DELETE | EV_RECEIPT,
          .udata = ev_ctx,
      };

  if (err_n > 0) {
    kevent(queue_fd, events, ev_n, events, ev_n, &(struct timespec){0, 0}); // Undo all events
    errno = prev_errno;
    return -1;
  }

#elif defined(NET_USE_EPOLL)
  uint32_t filters = 0;

  // Translate flags approprietly
  if (ev_ctx->filters & EVT_READ) filters |= EPOLLIN;
  if (ev_ctx->filters & EVT_WRITE) filters |= EPOLLOUT;
  if (ev_ctx->filters & EVT_RDHUP) filters |= EPOLLRDHUP;
  if (ev_ctx->filters & EVT_USER) filters |= EPOLLIN;

  if (filters == 0) { // Verify that there are events to add
    errno = EINVAL;
    return -1;
  }

  if (epoll_ctl(queue_fd, EPOLL_CTL_ADD, ev_ctx->ident,                                         // Add event to epoll instance
                &(struct epoll_event){.events = filters, .data.ptr = ev_ctx}) == -1) return -1; // On error return -1
#endif

  ev_ctx->queue = queue_fd; // Register queue
  return 0;
}

int unreg_ev(qevent_t* ev_ctx) {
  if (ev_ctx->queue == -1) {
    errno = ENOENT; // Event queue invalid
    return -1;
  }

#if defined(NET_USE_KQUEUE)
  struct kevent events[3], receipts[3];
  int ev_n = 0, rcp_n = 0, err_n = 0, prev_errno;

  // Translate flags approprietly
  if (ev_ctx->filters & EVT_READ)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_READ, .flags = EV_DELETE | EV_RECEIPT};
  if (ev_ctx->filters & EVT_WRITE)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_WRITE, .flags = EV_DELETE | EV_RECEIPT};
  if (ev_ctx->filters & EVT_USER)
    events[ev_n++] = (struct kevent){.ident = ev_ctx->ident, .filter = EVFILT_USER, .flags = EV_DELETE | EV_RECEIPT};

  if (ev_n == 0) {  // Verify that there are events to delete
    errno = EINVAL; // Event context invalid
    return -1;
  }

  // In case of error on the queue itself
  if ((rcp_n = kevent(ev_ctx->queue, events, ev_n, receipts, ev_n, &(struct timespec){0, 0})) == -1) {
    if (errno == EBADF) errno = ENOENT; // Queue is gone
    return -1;
  }

  ev_n = 0;                                                                                 // Reset ev_n and use as undo counter
  for (int n = 0; n < rcp_n; n++)                                                           // Check receipts for errors
    if (receipts[n].data != 0 && receipts[n].data != ENOENT && receipts[n].data != EBADF) { // If error different from event not registered
      prev_errno = receipts[n].data;
      err_n++;
    } else if (receipts[n].data == 0) // If socket was deleted, mark for potential undo
      events[ev_n++] = (struct kevent){
          .ident = receipts[n].ident,
          .filter = receipts[n].filter,
          .flags = EV_ADD | EV_RECEIPT | (receipts[n].filter == EVFILT_USER ? EV_CLEAR : 0),
          .udata = ev_ctx,
      };

  if (err_n > 0) {
    kevent(ev_ctx->queue, events, ev_n, events, ev_n, &(struct timespec){0, 0}); // Undo all events
    errno = prev_errno;
    return -1;
  }

#elif defined(NET_USE_EPOLL)
  if (epoll_ctl(ev_ctx->queue, EPOLL_CTL_DEL, ev_ctx->ident, NULL) == -1 && errno != ENOENT) { // Error different from event not registered
    int prev_errno = errno;
    if (prev_errno == EBADF && fcntl(ev_ctx->queue, F_GETFD) == -1) // Verify that queue still exists
      prev_errno = ENOENT;                                          // Queue is gone
    errno = prev_errno;
    return -1;
  }

#endif
  ev_ctx->queue = -1; // Unregister queue
  return 0;
}

int handle_ev(int queue_fd, uint8_t max_events, const struct timespec* timeout) {
  if (max_events == 0) {
    errno = EINVAL;
    return -1;
  }
  int r;

#if defined(NET_USE_KQUEUE)
  struct kevent events[max_events];
  if ((r = kevent(queue_fd, NULL, 0, events, max_events, timeout)) == -1) return -1;

  for (int n = 0; n < r; n++) { // For each event that has been detected
    struct kevent ev = events[n];
    qevent_t* ctx = ev.udata;

    // If error code is set on EV_EOF or on EV_ERROR
    if ((ev.flags & EV_EOF) && ev.fflags != 0 && !(ctx->flags & CTX_ERR)) {
      ctx->flags |= CTX_ERR; // Add error flag
      ctx->handler(EVT_ERROR, (union qevdata){.error = (int)ev.fflags}, ctx);
    }
    if ((ev.flags & EV_ERROR) && ev.data != 0 && !(ctx->flags & CTX_ERR)) {
      ctx->flags |= CTX_ERR; // Add error flag
      ctx->handler(EVT_ERROR, (union qevdata){.error = (int)ev.data}, ctx);
      continue;
    }
    if (ev.filter == EVFILT_USER)
      ctx->handler(EVT_USER, (union qevdata){0}, ctx);
    // If RDHUP handling is set, don't deliver 0bytes read events
    if (ev.filter == EVFILT_READ && !((ctx->filters & EVT_RDHUP) && ev.data <= 0))
      ctx->handler(EVT_READ, (union qevdata){.count = ev.data}, ctx);
    if (ev.filter == EVFILT_WRITE)
      ctx->handler(EVT_WRITE, (union qevdata){0}, ctx);
    // Check EV_EOF flag only if EVT_RDHUP is set & ev is read
    if (ev.flags & EV_EOF && ctx->filters & EVT_RDHUP && ev.filter == EVFILT_READ)
      ctx->handler(EVT_RDHUP, (union qevdata){0}, ctx);
  }

#elif defined(NET_USE_EPOLL)
  struct epoll_event events[max_events];
  int ms_timeout = timeout != NULL ? timespec_to_ns(*timeout) / (uint64_t)1000000 : -1;
  if ((r = epoll_pwait(queue_fd, events, max_events, ms_timeout, NULL)) == -1) return -1;

  for (int n = 0; n < r; n++) { // For each event that has been detected
    struct epoll_event ev = events[n];
    qevent_t* ctx = ev.data.ptr;

    if (ev.events & EPOLLERR) { // Check for socket error
      ctx->flags |= CTX_ERR;    // Add error flag
      int error_code = 0;
      socklen_t len = sizeof(error_code);
      getsockopt(ctx->ident, SOL_SOCKET, SO_ERROR, &error_code, &len);
      ctx->handler(EVT_ERROR, (union qevdata){.error = error_code}, ctx);
    }
    if (ev.events & EPOLLIN) {       // If data avail to read
      if (ctx->filters & EVT_USER) { // Switch case if it's user event
        uint64_t cnt;
        // Check + acknowledge event, if read successfull, call handler (equivalent as EV_CLEAR on kqueue)
        if (read(ctx->ident, &cnt, sizeof(cnt)) > 0)
          ctx->handler(EVT_USER, (union qevdata){0}, ctx);
      } else {
        int count;
        if (ioctl(ctx->ident, FIONREAD, &count) == -1)
          ctx->handler(EVT_READ, (union qevdata){0}, ctx);
        else if (count != 0 || !(ctx->filters & EVT_RDHUP))
          ctx->handler(EVT_READ, (union qevdata){.count = count}, ctx);
      }
    }
    if (ev.events & EPOLLOUT)
      ctx->handler(EVT_WRITE, (union qevdata){0}, ctx);
    if (ev.events & (EPOLLRDHUP | EPOLLHUP) && ctx->filters & EVT_RDHUP) // Notify HUP only if ctx subscribed
      ctx->handler(EVT_RDHUP, (union qevdata){0}, ctx);
  }

#endif
  return r; // Return number of processed events
}

static inline int set_sock_flags(int fd, int mask) {
  int socket_flags;
  if ((socket_flags = fcntl(fd, F_GETFL)) == -1)
    return -1;
  if (fcntl(fd, F_SETFL, socket_flags |= mask) == -1)
    return -1;
  return 0;
}

static inline qevent_t* alloc_ev_ctx() {
  return malloc(sizeof(qevent_t));
}
