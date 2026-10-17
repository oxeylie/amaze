/**-----------------------------------------------------------------------*
 * event.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef NET_SOCK_H
#define NET_SOCK_H

#include <misc.h>

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#if defined(__linux__)
#define NET_USE_EPOLL
#elif defined(__APPLE__) || defined(__FreeBSD__)
#define NET_USE_KQUEUE
#else
#error "Unsupported platform (only linux/FreeBSD/macOS)"
#endif

#define CTX_ERR 0b1

// Queue event types
enum qevtype : int16_t { // F: Can be set as flag T: can be triggered
  EVT_NONE = 0b0,
  EVT_READ = 0b1,      // [F,T] Read avail
  EVT_WRITE = 0b10,    // [F,T] Write avail
  EVT_USER = 0b100,    // [F,T] Event triggered
  EVT_RDHUP = 0b1000,  // [F,T] Read end Hangup
  EVT_ERROR = 0b10000, // [F] Error happened on socket
};

// Queue event data
union qevdata {
  void* ptr;
  int error;
  size_t count;
};

// Queue event
typedef struct evcontext qevent_t;

// Handler for queue events
// - ev_type holds what type of event triggered the handle
// - ev_data holds additionnal data depending on event type
// - ev_ctx holds the additionnal context associated to the event
typedef void (*evhandler_t)(enum qevtype ev_type, union qevdata ev_data, qevent_t* ev_ctx) _nonnull(3);

// Context associated to a queue event
struct evcontext {
  evhandler_t handler; // Function that will handle the event
  uint8_t filters;     // OR'd queue events which should trigger the event
  uint8_t flags;       // OR'd context flags
  int ident;           // Event identity (generally fd)
  int queue;           // Associated queue for event
  void* u_data;        // Associated user data
};

// Inits a tcp listening sock on given port
// - port contains the listening port for the sock
// - max_clients contains the maximum queued clients
// - [return] 0 if successfull -1 if failed + errno set
int init_listen_sock(int port, size_t max_clients);
// Inits a tcp connection to given adress on given port
// - port contains the port to connect to
// - address contains the address to connect to
// - [return] 0 if successfull -1 if failed + errno set
int init_peer_sock(int port, long address);

// Inits a user event
// - handler contains the handler function associated with the event
// - u_data contains arbitrary user data associated with the event
// - [return] ptr if successfull NULL if failed + errno set
_use_res qevent_t* init_user_ev(evhandler_t handler, void* u_data) _nonnull(1);
// Triggers a user event
// - ev_ctx contains the event to trigger
// - [return] 0 if successfull -1 if failed + errno set
int trigger_user_ev(qevent_t* ev_ctx) _nonnull();
// Frees an event
// - ev_ctx contains the event to free
// - [return] 0 if successfull -1 if failed + errno set
int free_user_ev(qevent_t* ev_ctx) _nonnull();

// Inits an event associated to a socket
// - fd contains the associated file descriptor
// - handler contains the handler function associated with the event
// - filters contains the OR'd filters that should trigger the event
// - u_data contains arbitrary user data associated with the event
_use_res qevent_t* init_sock_ev(int fd, evhandler_t handler, int16_t filters, void* u_data) _nonnull(2);
// Frees an event associated to a socket
// - sock_ctx contains the event to be freed
int free_sock_ev(qevent_t* sock_ctx) _nonnull();

// Inits an event queue
// - [return] file descriptor of event queue if successfull -1 if failed + errno set
_use_res int init_ev_queue();
// Registers an event to a queue
// - queue_fd contains the file descriptor of the target queue
// - ev_ctx contains the event to be registered in the queue
// - [return] 0 if successfull -1 if failed + errno set
int reg_ev(int queue_fd, qevent_t* ev_ctx) _nonnull(2);
// Unregisters the event from its queue
// - ev_ctx contains the event to be unregistered of the queue
// - [return] 0 if successfull -1 if failed + errno set
int unreg_ev(qevent_t* ev_ctx) _nonnull();
// Handle triggered events in the queue
// - queue_fd contains the file descriptor of the target queue
// - max_events contains the maximum number of allowed events to be handled
// - timeout contains the maximum duration for which the handle will wait for events to happen
// - [return] 0 if successfull -1 if failed + errno set
int handle_ev(int queue_fd, uint8_t max_events, const struct timespec* timeout);

#endif // NET_SOCK_H
