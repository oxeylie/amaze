/**-----------------------------------------------------------------------*
 * Amaze mini-game server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#include <pthread.h>

#include <log.h>
#include <misc.h>

void* network_thread_init(void* arg);
void* terrain_thread_init(void* arg);

const struct arg_def argl[] = {};
pthread_t network_thread;
pthread_t terrain_thread;

int main(int argc, char** argv) {
  eval_args(argv, argc, argl, sizeof(argl) / sizeof(const struct arg_def));

  pthread_create(&network_thread, NULL, network_thread_init, NULL);
  pthread_create(&terrain_thread, NULL, terrain_thread_init, NULL);
  print_logf("logic_thread");

  pthread_join(network_thread, NULL); // Wait for thread termination
  pthread_join(terrain_thread, NULL); // Wait for thread termination
  return 0;
}

void* network_thread_init(void* arg) {
  print_logf("network_thread");
  return NULL;
};

void* terrain_thread_init(void* arg) {
  print_logf("terrain_thread");
  return NULL;
};
