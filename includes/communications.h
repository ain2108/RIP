#ifndef _COMMUNICATIONS_
#define _COMMUNICATIONS_

#include "helpers.h"
#include "interface.h"

typedef struct Packet{
  char buffer[MAX_NODES * (IP_LENGTH + 32)];
} Packet;

Packet * buildPacket(TableEntry * routeTable, Packet * pack);
void printPacket(Packet * pack);
int sendPacketToPeer(Interface * peer, Packet * pack);

// Plagiarizing these function from my OWN code here. Used this code in my HW5
int sendToAllPeers(Interface * infaces, Packet * pack);
int createIPv4UDPSocket();
Packet * receivePacket(int sock, struct sockaddr_in * servAddr);

struct sockaddr_in * createIPv4Listener(unsigned short port, int sock);
#endif
