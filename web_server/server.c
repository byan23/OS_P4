#include "cs537.h"
#include "request.h"
#include "server.h"
#include "assert.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)> <max_thread_num> <req_buffer_size> 
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

volatile queue q;
pthread_mutex_t m;
pthread_cond_t not_full;
pthread_cond_t empty;

// # of threads
int t_num;
// size of request buffer
int b_sz;

int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  int i;

  // thread pool
  pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * t_num); 
  getargs(&port, &t_num, &b_sz, argc, argv);

  q.head = 0;
  q.tail = -1;
  q.sz = 0;
  q.req_buf = (int*) malloc(sizeof(int) * b_sz);
  memset(q.req_buf, -1, sizeof(int) * b_sz);
  /*for (i = 0; i < b_sz; ++i)
    printf("%d, ", q.req_buf[i]);*/

  pthread_mutex_init(&m, NULL);
  pthread_cond_init(&not_full, NULL);
  pthread_cond_init(&empty, NULL);

  for (i = 0; i < t_num; ++i) {
    if (pthread_create(&threads[i], NULL, consume, NULL) != 0) {
      perror("Thread create failure.\n");
      exit(1);
    }
    //printf("Thread %d created.\n", i+1);
  }

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    produce(connfd);
    // 
    // CS537: In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. However, for SFF, you may have to do a little work
    // here (e.g., a stat() on the filename) ...
    // 
    //requestHandle(connfd);

    //Close(connfd);
  }

}

// CS537: Parse the new arguments too
void getargs(int *port, int *t_num, int* b_sz, int argc, char *argv[])
{
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <port> <threadnum> <buffersize>\n",
	argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
  *t_num = atoi(argv[2]);
  *b_sz = atoi(argv[3]);

}

void *produce(int fd) {
  pthread_mutex_lock(&m);
  q.tail = (q.tail + 1) % b_sz;
  while (q.sz == b_sz)
    pthread_cond_wait(&not_full, &m);
  q.req_buf[q.tail] = fd;
  ++q.sz;
  assert(q.sz <= b_sz);
  //printbuf();
  pthread_cond_signal(&empty);
  pthread_mutex_unlock(&m);
  return NULL;
}

void *consume() {
  int fd;
  while (1) {
    pthread_mutex_lock(&m);
    //int i = 0;
    while (q.sz == 0) {
      //if (i == 0) printf("waiting...\n");
      //++i;
      pthread_cond_wait(&empty, &m);
    }
    fd = q.req_buf[q.head];
    assert(fd != -1);
    q.req_buf[q.head] = -1;
    --q.sz;
    assert(q.sz >= 0);
    q.head = (q.head + 1) % b_sz;
    //printbuf();
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&m);
    requestHandle(fd);
    Close(fd);
  }
  return NULL;
}

void printbuf() {
  int i;
  printf("print buf:\n");
  for (i = 0; i < b_sz; ++i)
    printf("%d ", q.req_buf[i]);
  printf("\n");
}


