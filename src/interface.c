#include "helpers.h"
#include "interface.h"

// Function takes the comman line argument, parses it, and writes it into interface
Interface * extractInterface(char * argv, Interface * interface){

  // Not to touch alter **argv
  char arg[SMALL_BUFFER];
  memset(arg, 0, SMALL_BUFFER);
  strcpy(arg, argv);
  
  // Extracting the address
  char * address = strtok(arg, ":");
  if(address == NULL) die("Bad Input (address):");

  // We need to check the validity of the provided ip
  if(isActualIP(address)){
    strncpy(interface->IP, address, IP_LENGTH);

  }else{  
    // Than its probably a domain
    struct hostent * temp;
    struct in_addr ** addr;
    temp = gethostbyname(address);

    // In case the input is simply invalid, we have to exit.
    if(temp == NULL) die("Invalid domain name:");

    // Cast
    addr = (struct in_addr **) temp->h_addr_list;
    strncpy(interface->IP, inet_ntoa(*addr[0]), IP_LENGTH);
  }

  // Extracting the port
  char * strport = strtok(NULL, ":");
  if(strport == NULL) die("Bad Input (port):");
  unsigned short port = (unsigned short) atoi(strport);
  // fprintf(stderr, "port: %d\n", port);
  // Check if conversion was succesful
  if(port == 0) die("Invalid port number:");
  // Check if the port number is in the valid range
  if(1024 < port && port <= 65535){
    interface->port = port;
  }else{
    die("Invalid port number (out of range):");
  }

  // Extracting the id
  char * strhops = strtok(NULL, ":");
  if(strhops == NULL) die("Bad Input (id):");
  int hops = atoi(strhops);
  if(hops == 0) die("Invalid id for interface:");
  interface->hops = hops;

  // Interface must be activated
  interface->active = 1;

  return interface;
}

// Prints the interface to stdout
void printInterface(Interface * inface){
  fprintf(stdout, "%s:%d:%d\n", inface->IP, inface->port, inface->hops);
  return;
}

// Print an array of interfaces
void printInterfaceArray(Interface * infaceArray, int number){
  int i = 0;
  for(i = 0; i < number; i++){
    printInterface(infaceArray + i);
  }
  return;
}

// Prints the routing table
void printRoutingTable(Interface * routingTable, char * myIP, unsigned short myPort){

  // Get the time
  time_t epoch;
  struct tm * temp;
  epoch = time(NULL);
  temp = localtime(&epoch);
  char timestamp[IP_LENGTH];
  memset(timestamp, 0, IP_LENGTH);
  sprintf(timestamp, "%.2d:%.2d:%.2d", temp->tm_hour, temp->tm_min, temp->tm_sec );

  fprintf(stdout, "Node %s:%d @ %s\n\n", myIP, myPort, timestamp);
  fprintf(stdout, "host\t\tport\tdistance\tinterface\n");
  int i = 0;
  for(i = 0; 1; i++){
    if(routingTable[i].active == 0) break;
    fprintf(stdout, "%s\t%d\t%d\t\t%d\n",
	    routingTable[i].IP,
	    routingTable[i].port,
	    routingTable[i].hops,
	    routingTable[i].id);
  }
  // free(temp);
  return;
}

// Function checks if the provide IP is a valid IP
int isActualIP(char * isIP){
  struct sockaddr_in temp;
  int isValidIP = inet_pton(AF_INET, isIP, &(temp.sin_addr));
  return (isValidIP != 0);
}

// Returns peer's Interface
Interface * infacesByIP(Interface * infaces, char * peerIP){
  int pos = 0;
  for(pos = 0; 1; pos++){
    if(infaces[pos].active == 0) break;
    if(!strcmp(infaces[pos].IP, peerIP)) return infaces + pos;
  }
  return NULL;
}

// Find the position of the entry in the table by IP
int posByIP(TableEntry * routingTable, char * IP){
  int pos = 0;
  for(pos = 0; 1; pos++){
    if(routingTable[pos].active == 0) break;
    if(!strcmp(routingTable[pos].IP, IP)) return pos;
  }
  return pos;
}

// Find the position of the entry in the table by IP
int posByIPIF(Interface * infaces, char * IP){
  int pos = 0;
  for(pos = 0; 1; pos++){
    if(infaces[pos].active == 0) return -1;;
    if(!strcmp(infaces[pos].IP, IP)) return pos;
  }
}


// Funtion applies the changes to the routing table. Also responsible for printing.
int applyChanges(TableEntry * routingTable, Interface * infaces, 
		 char * entry,	int disToPeer, 
		 int peerFaceID, char * myIP, 
		 char * peerIP, pthread_rwlock_t * table_lock,
		 pthread_rwlock_t * face_lock){

  // Extract components
  char * IP = strtok(entry, ":");
  if(IP == NULL || !isActualIP(IP) ) return -1;
  if(!strcmp(IP, myIP)){
    //    fprintf(stderr, "self ignored\n");
    return -1;
  }

  char * portStr = strtok(NULL, ":");
  if(portStr == NULL) return -1;
  unsigned short port = atoi(portStr);

  char * distStr = strtok(NULL, ":");
  if(distStr == NULL) return -1;
  int distance = atoi(distStr);

  // We need to find this IP in the routingTable
  pthread_rwlock_rdlock(table_lock);
  int posInTable = posByIP(routingTable, IP);
  int oldDistance = routingTable[posInTable].hops;
  pthread_rwlock_unlock(table_lock);

  // To handle the node that is known to be unreachable
  if(oldDistance != -1 && distance == -1){
    pthread_rwlock_wrlock(table_lock);
    routingTable[posInTable].hops = -1;             // Distance 
    pthread_rwlock_unlock(table_lock);
    return 1;
  }

  // If a node is known to be unreachable, ignore INT_MAX from other peers
  if(oldDistance == -1 && distance == -1){
    return 0;
  }

  // If not an immediate neighbour went down, but the route is influence
  if(distance == INT_MAX / 2){
    pthread_rwlock_rdlock(face_lock);
    int posInfaces = posByIPIF(infaces, IP);
    pthread_rwlock_unlock(face_lock);
    
    fprintf(stderr, "POSITION: %d\n", posInfaces);

    pthread_rwlock_wrlock(table_lock);
    if(posInfaces < 0){
      fprintf(stderr, "unknown\n");
      routingTable[posInTable].hops = distance;             // Distance 
      routingTable[posInTable].id = peerFaceID;             // Interface ID
    }else{
      fprintf(stderr, "updated to %d\n", infaces[posInfaces].hops);
      routingTable[posInTable].hops = infaces[posInfaces].hops;         
      routingTable[posInTable].id = infaces[posInfaces].id;       
    }
    strncpy(routingTable[posInTable].IP, IP, IP_LENGTH);  // IP
    routingTable[posInTable].port = port;                 // Port
    routingTable[posInTable].active = 1;                  // Active
   
    pthread_rwlock_unlock(table_lock);
    return 1;
  }

  // Get the actual distance to the node
  distance = distance + disToPeer;

  // If evaluating the distance to immediate neighbour
  if(!strcmp(IP, peerIP)){
    if(disToPeer > oldDistance) return 0;
    pthread_rwlock_wrlock(table_lock);
    routingTable[posInTable].hops = disToPeer;           // Distance 
    strncpy(routingTable[posInTable].IP, IP, IP_LENGTH);  // IP
    routingTable[posInTable].port = port;                 // Port
    routingTable[posInTable].active = 1;                  // Active
    routingTable[posInTable].id = peerFaceID;             // Interface ID
    pthread_rwlock_unlock(table_lock);
    return 1;
  }

  // If the distance in the database is larger
  if(oldDistance != 0 && distance >= oldDistance) return 0;

  // Than the distance is either 0, meaning that we have no entry on this IP,
  // or its bigger than new distance. One way or another, we have to overwrite.
  pthread_rwlock_wrlock(table_lock);
  routingTable[posInTable].hops = distance;             // Distance 
  strncpy(routingTable[posInTable].IP, IP, IP_LENGTH);  // IP
  routingTable[posInTable].port = port;                 // Port
  routingTable[posInTable].active = 1;                  // Active
  routingTable[posInTable].id = peerFaceID;             // Interface ID
  pthread_rwlock_unlock(table_lock);        

  return 1;
}

// Function applies the changes for peer death. 
int eliminatePeer(TableEntry * routingTable, Interface * infaces,
		  char * peerIP, int peerFaceID, pthread_rwlock_t * table_lock){

  int stat = 0;

  pthread_rwlock_wrlock(table_lock);
  // We need to set the distance to the peer equal to 0 so other routers ignore it. 
  int posInTable = posByIP(routingTable, peerIP);
  if( routingTable[posInTable].hops != -1){
    routingTable[posInTable].hops = -1;
    stat = 1;
  }

  /* We now need to set the distance to all the nodes which are 
   * accessed using this peer to INT_MAX */
  int i = 0;
  int posInfaces;
  for(i = 0; 1; i++){

    if(routingTable[i].active == 0) break; // Reached the end of the table

    if(routingTable[i].hops == -1) continue; // If the node is dead, we ignore

    if(routingTable[i].id == peerFaceID &&  // Node accessed through DEAD interface
       (posInfaces = posByIPIF(infaces, routingTable[i].IP)) > 0 && // Node in question is peer
       strcmp(peerIP, routingTable[i].IP)){ // And the dead node is not the node in question
      fprintf(stderr, "%d->%d\n", routingTable[i].hops, infaces[posInfaces].hops);
      routingTable[i].hops = infaces[posInfaces].hops;
      stat = 1;
      continue;
    }

    // For all entries that match the id
    if(routingTable[i].id == peerFaceID){
      if(routingTable[i].hops == INT_MAX / 2) continue;
      routingTable[i].hops = INT_MAX / 2;
      //      routingTable[i].id = -1;
      stat = 1;
    }
  }
  pthread_rwlock_unlock(table_lock);
  return stat;
}  

