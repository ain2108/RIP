#ifndef _LISTENER_
#define _LISTENER_

#include "helpers.h"

// These is the argument to the thread
typedef struct ToListenerThread{

  char * myIP;
  unsigned short myPort;
  TableEntry * routingTable;
  pthread_rwlock_t * table_lock;
  Interface * infaces;
  pthread_rwlock_t * infaces_lock;
  pthread_mutex_t * dead_lock;
  
} ToListenerThread;

void * listener_thread(void * args);


#endif
