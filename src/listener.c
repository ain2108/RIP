#include "helpers.h"
#include "interface.h"
#include "communications.h"
#include "listener.h"

// This thread will listen and alter the table
void * listener_thread(void * args){

  // For convenience 
  ToListenerThread * real = (ToListenerThread *) args;
  char * myIP = real->myIP;
  unsigned short myPort = real->myPort;
  TableEntry * routingTable = real->routingTable;
  pthread_rwlock_t * table_lock = real->table_lock;
  Interface * infaces = real->infaces;
  pthread_rwlock_t * infaces_lock = real->infaces_lock;
  pthread_mutex_t * dead_lock = real->dead_lock;

  // Create the listening socket
  int sock = createIPv4UDPSocket();

  // Create the sockaddr_in
  struct sockaddr_in * servAddr = createIPv4Listener(myPort, sock);

  // Enter the infinite listening loop
  Interface * peerFace;
  Packet * pack;
  char * peerIP;
  int altered = 0;
  while(1){

    // Receive the packet
    pack = receivePacket(sock, servAddr);
    // Get peer's IP address
    peerIP = inet_ntoa(servAddr->sin_addr);
    // Look up the peer in own interface table, if absent, ignore tha package
    peerFace = infacesByIP(infaces, peerIP);
    if(peerFace == NULL){
      fprintf(stderr, "node %s not peer\n", peerIP);
      free(pack);
      continue;   
    }

    // Packet processing
    //    fprintf(stderr, "packet received from %s:\n", peerIP);
    // printPacket(pack);
    
    // Check if packed is not a "DEAD" packet
    if(!strcmp(pack->buffer, "DEAD")){
      pthread_mutex_lock(dead_lock);
      fprintf(stderr, "node %s is dead\n", peerIP);
      free(pack);
      int stat = eliminatePeer(routingTable, infaces, 
			       peerIP, peerFace->id, table_lock);
      if(stat == 1) printRoutingTable(routingTable, myIP, myPort);
      pthread_mutex_unlock(dead_lock);
      continue;
    }

    // Everyting is in order. We now read the packet and apply changes
    int peerFaceID = peerFace->id;
    int disToPeer = peerFace->hops;

    // Extract the first entry
    int j = 0;
    char * entries[MAX_NODES];
    memset((char *) entries, 0, MAX_NODES * sizeof(void *));
    entries[j] = strtok(pack->buffer, "\n");
    if(entries[j] == NULL){
      free(pack);
      continue;
    }

    // Extract the rest
    for(j = 1; j < MAX_NODES; j++){
      entries[j] = strtok(NULL, "\n");
      if(entries[j] == NULL) break;
    }

    // Apply changes to the routing table
    int stat;
    int i;
    for(i = 0; i < j + 1; i++){
      stat = applyChanges(routingTable, infaces, entries[i],
			  disToPeer, peerFaceID,
			  myIP, peerFace->IP, 
			  table_lock, infaces_lock);
      if(stat == 1) altered = 1;
    }

    // If the routing table has undergone changes, print it to the user
    if(altered == 1) printRoutingTable(routingTable, myIP, myPort);

    // Cleaning
    altered = 0;
    free(pack);
  }

  fprintf(stderr, "Listener quits...\n");
  return NULL;
}
