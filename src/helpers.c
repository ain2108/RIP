#include "helpers.h"

void die(char * message){
  perror(message);
  exit(1);
}

// This snippet is modified from:
// http://stackoverflow.com/questions/17909401/linux-c-get-default-interfaces-ip-address
char * getMyIP(char * buffer){

  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);

  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;

  /* I want an IP address attached to "eth0" */
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1); // Change this for CLIC
  ioctl(fd, SIOCGIFADDR, &ifr);

  close(fd);

  /* Display result */
  sprintf(buffer, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

  return buffer;
}

int charToChar(char * dest, char * src, int numberOfBytes){
  int i = 0;
  for(i = 0; i < numberOfBytes; i++){
    dest[i] = src[i];
  }
  return i;
}

