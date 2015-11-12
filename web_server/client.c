/*
 * client.c: A very, very primitive HTTP client.
 * 
 * To run, try: 
 *      client www.cs.wisc.edu 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * CS537: For testing your server, you will want to modify this client.  
 * For example:
 * 
 * You may want to make this multi-threaded so that you can 
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs; 
 * you may want to get more URIs from the command line 
 * or read the list from a file. 
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "cs537.h"

typedef struct {
  char *host;
  char *filename;
  int port;
} req_info;

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

void *get_thread(void *ptr) {
  /* Open a single connection to the specified host and port */
  req_info *info = (req_info*)ptr;
  int clientfd = Open_clientfd(info->host, info->port);
  
  clientSend(clientfd, info->filename);
  clientPrint(clientfd);
    
  Close(clientfd);
  return NULL;
}

int main(int argc, char *argv[])
{
  req_info info;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  info.host = argv[1];
  info.port = atoi(argv[2]);
  info.filename = argv[3];
  
  int i;
  int t_num = 50;
  pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * t_num);
  for (i = 0; i < t_num; ++i) {
    if (pthread_create(&threads[i], NULL, get_thread, &info) != 0) {
      perror("Thread create failure.\n");
      exit(1);
    }
  }
  for (i = 0; i < t_num; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      perror("Thread join failure.\n");
      exit(1);
    }
  }
  /* Open a single connection to the specified host and port */
  /*clientfd = Open_clientfd(host, port);
  
  clientSend(clientfd, filename);
  clientPrint(clientfd);
    
  Close(clientfd);*/

  exit(0);
}
