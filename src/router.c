#include "helpers.h"
#include "interface.h"
#include "communications.h"
#include "listener.h"

// The time in seconds between sending the message to all peers
#define PERIOD 2 

// The thread that will deal with listing on the socket and modifying the table
pthread_t listener;
Interface * infacesAtSIGINT;

// At SIGINT
void sigintHandler(int dummy){
  Packet pack;
  memset((char *) &pack, 0, MAX_NODES * (IP_LENGTH + 32));
  strcpy(pack.buffer, "DEAD");
  sendToAllPeers(infacesAtSIGINT, &pack);
  exit(1);
}

int main(int argc, char ** argv){

  // Check the usage
  if(argc <= 2){
    fprintf(stderr, "Usage: <%s> <listen_port> <ip1:port1:id1> [...]\n", argv[0]);
    exit(1);
  }
  
  fprintf(stdout, "Ititializing...\n");

  // Declarations
  char myIP[IP_LENGTH];
  memset(myIP, 0, IP_LENGTH);
  getMyIP(myIP);
  int listen_port = atoi(argv[1]);
  if(listen_port == 0) die("Bad listen_port:");
  if(listen_port <= 1024 || listen_port >= 65536) die("Bad listen_port:");
  int numberOfNeighbours = argc - 2;

  // Starting Up
  Interface infaces[argc - 1];
  memset((char *) infaces, 0, sizeof(Interface) * (numberOfNeighbours + 1));
  // Populate the array of interfaces
  int i = 0;
  for(i = 0; i < numberOfNeighbours; i++){
    extractInterface(argv[i + 2], infaces + i);
    infaces[i].id = i + 1;
  }

  // Set up the actions at CTRL-C 
  infacesAtSIGINT = infaces;
  signal(SIGINT, sigintHandler);
  
  // Print to make sure everything went right
  printRoutingTable(infaces, myIP, listen_port);

  // Create the routing table
  TableEntry routingTable[MAX_NODES]; //= malloc( sizeof(TableEntry) * MAX_NODES);
  memset( (char *) routingTable, 0, MAX_NODES * sizeof(TableEntry));

  // Add the neighbours to the table
  int bytesCopied = charToChar((char *)routingTable, (char *) infaces,
	   sizeof(Interface) * numberOfNeighbours);
  if(bytesCopied != sizeof(Interface) * numberOfNeighbours) die("Problem on copy: ");
  
  // Build the first packet
  Packet pack;
  buildPacket(routingTable, &pack);
  //  printPacket(&pack);
  
  // Send the first packet to peers
  sendToAllPeers(infaces, &pack);

  // We need read/write lock for the routing table
  pthread_rwlock_t table_lock;
  pthread_rwlock_init(&table_lock, NULL);

  // We also need rw lock for the interface table, for the time when router shutsdown
  pthread_rwlock_t infaces_lock;
  pthread_rwlock_init(&infaces_lock, NULL);

  
  // Go to the thread that will deal with updating the routing table
  ToListenerThread * thread_args = (ToListenerThread *)malloc(sizeof(ToListenerThread));
  memset((char *) thread_args, 0, sizeof(ToListenerThread));

  pthread_mutex_t dead_lock;
  pthread_mutex_init(&dead_lock, NULL);

  // Fill in the arguments
  thread_args->myIP = myIP;
  thread_args->myPort = listen_port;
  thread_args->routingTable = routingTable;
  thread_args->table_lock = &table_lock;
  thread_args->infaces = infaces;
  thread_args->infaces_lock = &infaces_lock;
  thread_args->dead_lock = &dead_lock;

  // Multithread here
  int thread_err = 0;
  thread_err = pthread_create(&listener, NULL, listener_thread, (void *) thread_args);
  if(thread_err != 0) die("threading failed: ");
  
  // The rest of this thread will simply send the routing table to peers every
  // PERIOD seconds.
  // int packSent = 0;  
  while(1){

    // Sleep for some seconds
    sleep(PERIOD);

    pthread_mutex_lock(&dead_lock);
    // Lock the table and build packet 
    pthread_rwlock_rdlock(&table_lock);
    buildPacket(routingTable, &pack);
    pthread_rwlock_unlock(&table_lock);

    // Send the packet to everyone
    pthread_rwlock_rdlock(&infaces_lock);
    sendToAllPeers(infaces, &pack);
    pthread_rwlock_unlock(&infaces_lock);
    
    // On Ctrl-C something special has to be done
    //    fprintf(stderr, "%d packets sent\n", packSent);
    //   printPacket(&pack);

    pthread_mutex_unlock(&dead_lock);
    
  }
  
  fprintf(stdout, "Done.\n\n");
  
  return 0;
}
