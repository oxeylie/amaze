/**-----------------------------------------------------------------------*
 * event.h
 * 
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef NET_SOCK_H
#define NET_SOCK_H

#if defined(__linux__)
#define NET_USE_EPOLL
#elif defined(__APPLE__) || defined(__FreeBSD__)
#define NET_USE_KQUEUE
#else
#error "Unsupported platform (only linux/FreeBSD/macOS)"
#endif

#include <unistd.h>

// Queue event types
enum qevtrigger : int16_t {
  T_EV_READ = 0x1,   // Can be both set as flag and triggered
  T_EV_WRITE = 0x2,  // Can be both set as flag and triggered
  T_EV_USER = 0x8,   // Can be set as only flag and triggered
  T_EV_HUP = 0x10,   // Can be set as flag but not alone and triggered (note: will always be delivered after T_EV_READ)
  T_EV_ERROR = 0x20, // Cannot be set as flag but triggered
};

// Queue event data
union qevdata {
  void* ptr;
  int error;
};

// Queue event
typedef struct evcontext qevent_t;

// Handler for queue events
// - ev_type holds what type of event triggered the handle
// - ev_data holds additionnal data depending on event type
// - ev_ctx holds the additionnal context associated to the event
typedef void (*evhandler_t)(enum qevtrigger ev_type, union qevdata ev_data, qevent_t* ev_ctx);

// Context associated to a queue event
struct evcontext {
  evhandler_t handler; // Function that will handle the event
  int16_t filters;     // OR'd queue events which should trigger the event
  int ident;           // Event identity (generally fd)
  void* u_data;        // Associated user data
};

// Inits a tcp listening sock on given port
// - port contains the listening port for the sock
// - [return] 0 if successfull -1 if failed + errno set
int init_listen_sock(int port);
// Inits a tcp connection to given adress on given port
// - port contains the port to connect to
// - address contains the address to connect to
// - [return] 0 if successfull -1 if failed + errno set
int init_peer_sock(int port, long address);

// Inits a user event
// - handler contains the handler function associated with the event
// - u_data contains arbitrary user data associated with the event
// - [return] ptr if successfull NULL if failed + errno set
qevent_t* init_user_ev(evhandler_t handler, void* u_data);
// Triggers an event
// - ev_ctx contains the event to trigger
// - [return] 0 if successfull -1 if failed + errno set
int trigger_user_ev(qevent_t* ev_ctx);
// Frees an event
// - ev_ctx contains the event to free
// - [return] 0 if successfull -1 if failed + errno set
int free_user_ev(qevent_t* ev_ctx);

// Inits an event associated to a socket
// - fd contains the associated file descriptor
// - handler contains the handler function associated with the event
// - filters contains the OR'd filters that should trigger the event
// - u_data contains arbitrary user data associated with the event
qevent_t* init_sock_ev(int fd, evhandler_t handler, int16_t filters, void* u_data);
// Frees an event associated to a socket
// - sock_ctx contains the event to be freed
void free_sock_ev(qevent_t* sock_ctx);

// Inits an event queue
// - [return] file descriptor of event queue if successfull -1 if failed + errno set
int init_ev_queue();
// Registers an event in the queue
// - queue_fd contains the file descriptor of the target queue
// - ev_ctx contains the event to be registered in the queue
// - [return] 0 if successfull -1 if failed + errno set
int reg_ev(int queue_fd, qevent_t* ev_ctx);
// Unregisters an event in the queue
// - queue_fd contains the file descriptor of the target queue
// - ev_ctx contains the event to be unregistered of the queue
// - [return] 0 if successfull -1 if failed + errno set
int unreg_ev(int queue_fd, qevent_t* ev_ctx);
// Handle triggered events in the queue
// - queue_fd contains the file descriptor of the target queue
// - max_events contains the maximum number of allowed events to be handled
// - timeout contains the maximum duration for which the handle will wait for events to happen
// - [return] 0 if successfull -1 if failed + errno set
int handle_ev(int queue_fd, int max_events, struct timespec* timeout);

#endif // NET_SOCK_H
