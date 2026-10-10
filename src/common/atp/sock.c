/**-----------------------------------------------------------------------*
 * Sock management for atp over tcp
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#endif
#include <arpa/inet.h>
#include <sys/socket.h>

#include <atp/sock.h>
#include <log.h>

// Local function declarations

static int add_read_sock_queue(int queue_fd, struct sock_context* sock_ctx);
static int set_non_block(int fd);

// Global function definition

int init_peer_socket(int port, long adress) {
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0); // Create new socket
  if (sock_fd == -1) {
    print_errorf(errno, "Opening listen TCP socket");
    return -1;
  }

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr.s_addr = htonl(adress),
  };
  if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    print_errorf(errno, "Connecting to server");
    return -1;
  };

  if (set_non_block(sock_fd) == -1) {
    print_errorf(errno, "Setting flags on peer socket");
    return -1;
  }

  return sock_fd;
}

int init_listen_sock(int port) {
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0); // Initiate TCP ipv4 connection

  if (sock_fd == -1) {
    print_errorf(errno, "Opening listen TCP socket");
    return -1;
  }

  if (set_non_block(sock_fd) == -1) {
    print_errorf(errno, "Setting flags on listen socket");
    return -1;
  }

  struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,                // Set type to TCP
      .sin_port = htons(port),              // Configure port
      .sin_addr.s_addr = htonl(INADDR_ANY), // Configure adress
  };

  if (bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) {
    print_errorf(errno, "Binding listen socket to port %d", port);
    return -1;
  }

  if (listen(sock_fd, MAX_QUEUED_CLIENTS) == -1) { // Set sock to passive listening with MAX_QUEUED_CLIENTS
    print_errorf(errno, "Listening on socket");
    return -1;
  };

  return sock_fd;
}

int init_queue() {
  int queue_fd;

#ifdef __linux__
  queue_fd = epoll_create1(0); // Init epoll instace with no flags
  if (queue_fd == -1) {
    printErrorf(errno, "Creating epoll instance");
    return -1;
  }
#elif defined(__APPLE__)
  queue_fd = kqueue(); // Init a kqeue instance
  if (queue_fd == -1) {
    print_errorf(errno, "Creating kqueue instance");
    return -1;
  }
#endif

  return queue_fd;
}

struct sock_context* add_sock_to_queue(int queue_fd, int sock_fd, enum sock_type type) { // Needs to live in memory, should be allocated
  struct sock_context* sock_ctx = malloc(sizeof(struct sock_context));
  sock_ctx->type = type;
  sock_ctx->fd = sock_fd;

  switch (sock_ctx->type) {
  case T_LISTEN_SOCK:
  case T_PEER_SOCK:
    return add_read_sock_queue(queue_fd, sock_ctx) != -1 ? sock_ctx : NULL;
  default:
    return NULL;
  }
};

int close_sock_ctx(struct sock_context* sock_ctx) {
  if (close(sock_ctx->fd) == -1) return -1;
  free(sock_ctx);
  return 0;
};

int handle_sock_event(int queue_fd, event_handler handler, int max_events, struct timespec* timeout) {
#ifdef __linux__
  struct epoll_event events[max_events];
  int n = epoll_pwait2(queue_fd, events, max_events, timeout, NULL);

  if (n == -1)
    return -1;

  for (int i = 0; i < n; i++) {
    uint32_t flags = events[n].events;
    enum sock_event event_type;

    if (flags & EPOLLIN)
      event_type = SEV_READ;
    else if (flags & EPOLLHUP)
      event_type = SEV_RESET;
    else if (flags & EPOLLERR)
      event_type = SEV_ERR;
    else
      event_type = SEV_UNKNOWN;

    handler(events[n].data.ptr, event_type);
  }

#elif defined(__APPLE__)
  struct kevent events[max_events];
  int n = kevent(queue_fd, NULL, 0, events, max_events, timeout); // Fetch max number of events

  if (n == -1) // If error during poll
    return -1;

  for (int i = 0; i < n; i++) {
    uint32_t flags = events[n].flags;
    enum sock_event event_type;

    if (flags & EV_EOF)
      event_type = SEV_RESET;
    else if (flags & EV_ERROR)
      event_type = SEV_ERR;
    else
      event_type = SEV_READ;

    handler(events[n].udata, event_type);
  }
#endif
  return n; // Return number of events that happened
};

// Local function definiton

static int add_read_sock_queue(int queue_fd, struct sock_context* sock_ctx) {
#ifdef __linux__
#error "Not implemented yet"
  // epoll_ctl
#elif defined(__APPLE__)
  int result = kevent(queue_fd, &(struct kevent){
                                    // TODO: Check if returns EOF on write end close
                                    .flags = EV_ADD, // Will contain EV_EOF on sock close
                                    .ident = sock_ctx->fd,
                                    .filter = EVFILT_READ,
                                    .udata = sock_ctx,
                                },
                      1, NULL, 0, NULL);

  return result != -1 ? 0 : -1;
#endif
}

static inline int set_non_block(int fd) {
  int socket_flags;
  if ((socket_flags = fcntl(fd, F_GETFL)) == -1)
    return -1;

  socket_flags |= O_NONBLOCK; // Set socket to non-blocking

  if (fcntl(fd, F_SETFL, socket_flags) == -1)
    return -1;
  return 0;
};
