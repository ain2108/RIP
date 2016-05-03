#ifndef _INTERFACE_
#define _INTERFACE_


#include "helpers.h"

// This struct contains the information about the edge
typedef struct Interface{
  char IP[IP_LENGTH];
  unsigned short port;
  int hops;
  int id;
  int active;
  
} Interface, TableEntry;

// Function tokenizes the comman line arguments, and puts them into structure
Interface * extractInterface(char * arg, Interface * inface);
int isActualIP(char * isIP);
void printInterface(Interface * inface);
void printInterfaceArray(Interface * infaceArray, int number);
void printRoutingTable(Interface * routingTable, char * myIP, unsigned short myPort);
// int getInterfaceID(Interface * infaces);
Interface * infacesByIP(Interface * infaces, char * peerIP);
int posByIP(TableEntry * routingTable, char * IP);
int posByIPIF(Interface * infaces, char * IP);
int applyChanges(TableEntry * routingTable, Interface * infaces, 
		 char * entry,	int disToPeer, 
		 int peerFaceID, char * myIP, 
		 char * peerIP, pthread_rwlock_t * table_lock,
		 pthread_rwlock_t * face_lock);
int eliminatePeer(TableEntry * routingTable, Interface * infaces, char * peerIP, 
		  int peerFaceID, pthread_rwlock_t * table_lock);

#endif
