#include "communications.h"

// Builds a packet from a routing table
Packet * buildPacket(TableEntry * routeTable, Packet * pack){

  
  // Zero out just to make sure
  memset((char *) pack, 0, sizeof(Packet));
  
  // Iterate through the table writing data to the packet buffer
  int i = 0;
  char temp[SMALL_BUFFER];
  memset(temp, 0, SMALL_BUFFER);
  for(i = 0; 1; i++){
    if(routeTable[i].active == 0) break; // breaks here
    // Put the IP in
    sprintf(temp, "%s:%d:%d\n", routeTable[i].IP,
	    routeTable[i].port, routeTable[i].hops);
    strcat(pack->buffer, temp);
    memset(temp, 0, SMALL_BUFFER);
  }

  return pack;
}

// Printer
void printPacket(Packet * pack){
  fprintf(stderr, "Packet:\n%s", pack->buffer);
  return;
}

// Send packet to peer
int sendPacketToPeer(Interface * peer, Packet * pack){

  // Decalarations
  char * IP = peer->IP;
  unsigned short port = peer->port;
  struct sockaddr_in other;
  int sock;
  int otherLen = sizeof(other);

  // Get a socket
  if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) die("socket() failed:");

  // Fill in the structure
  memset((char *) &other, 0, otherLen);
  other.sin_family = AF_INET;
  other.sin_port = htons(port);
  if(inet_aton(IP, &other.sin_addr) == 0) die("inet_aton() failed:");

  // Send the packet
  if(sendto(sock, pack->buffer, MAX_NODES * (IP_LENGTH + 32),
	    0, (const struct sockaddr *)&other, otherLen) == -1){
    die("sendto() failed");
  }

  close(sock);
  return 0;
}

// Send packet to all peer. Returns number of packets sent.
int sendToAllPeers(Interface * infaces, Packet * pack){
  int i = 0;
  for(i = 0; 1; i++){
    if(infaces[i].active == 0) break;
    sendPacketToPeer(infaces + i, pack);
  }
  return i;
}

// Creates IPv4 UDP socket
int createIPv4UDPSocket(){
  int sock;
  if ( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    die("socket() failed:");
  }
  return sock;
}

// Function fills in the serv address on the receiver side
struct sockaddr_in * createIPv4Listener(unsigned short port, int sock){

  struct sockaddr_in * servAddr =
    (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
  int servLen = sizeof(struct sockaddr_in);

  // Filling the structure
  memset((char *) servAddr, 0, servLen);
  servAddr->sin_family = AF_INET;
  servAddr->sin_port = htons(port);
  servAddr->sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sock, (struct sockaddr *)servAddr, sizeof(struct sockaddr_in)) == -1){
    die("bind() failed:");
  }

  return servAddr;
}

Packet * receivePacket(int sock, struct sockaddr_in * servAddr){
  // Creating a packet
  Packet * pack = (Packet *) malloc (sizeof(Packet));
  memset((char *) pack, 0, sizeof(Packet));
  socklen_t servLen = sizeof(struct sockaddr_in);

  ssize_t bytesReceived;
  if((bytesReceived = recvfrom(sock, pack->buffer, MAX_NODES * (IP_LENGTH + 32), 0,
			       (struct sockaddr *)servAddr, &servLen)) ==-1){
    die("recvfrom() failed:");
  }
  // int dataLength = bytesReceived - 20;
  // pack->data_size = dataLength;

  return pack;
}
 
  


