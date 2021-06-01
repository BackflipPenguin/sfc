#define _GNU_SOURCE
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
  int socket_desc;
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == 1) {
	fprintf(stderr, "finger: Couldn't create socket | socket: %s\n", strerror(errno));
	return 1;
  }

  if (argc < 2){
	  puts("Usage: sfc [address or url] [user](optional) [port](optional)");
	  return 0;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  if (argc == 4){
	  int port = atoi(argv[3]);
	  if (port <= 0 || port > 65535){
		  fprintf(stderr, "finger: Invalid port number\n" );
		  return 1;
	  } 
	  else{
		  server.sin_port = (unsigned short)port;
	  }
  } else {
	  server.sin_port = htons(79);
  }

  struct hostent *hostentry;
  if ( (hostentry = gethostbyname(argv[1])) == NULL){ // get address from url 
	  fprintf(stderr, "finger: Couldn't get address | gethostbyname: %s\n", strerror(errno));
	  return 1;
  }
  server.sin_addr = *(struct in_addr *)hostentry->h_addr_list[0];

  if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server))) {
	fprintf(stderr, "finger: Couldn't open socket | connect: %s\n", strerror(errno));
    return 1;
  }

  /* write name to remote socket &
	 write CRLF to indicate end of input */
  char* url;
  if (argc < 3){
	  url = "" ;
  } else {
	  url = argv[2];
  }
  puts(url);
  
  if(write(socket_desc, url, strlen(url)) == -1 || write(socket_desc, "\r\n", 2) == -1){
	  fprintf(stderr, "finger: Couldn't send username | write: %s\n", strerror(errno));
	  return 1;
  }

  register FILE *remote;
  register int c;


  remote = fdopen(socket_desc, "r"); // open remote to read data
  if (!remote){
	  fprintf(stderr, "finger: Couldn't open remote | fdopen: %s\n",strerror(errno));
	  close(socket_desc);
	  return 1;
  }

  while ((c = getc(remote)) != EOF) {
	  if(c != '\r'){ // don't write carriage return
		  putchar(c);
	  }
  }
  fclose(remote);
  close(socket_desc);

  return 0;
}
