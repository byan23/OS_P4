#ifndef __SERVER_H__
#define __SERVER_H_

typedef struct {
  int head;
  int sz;
  int tail;
  int *req_buf;
} queue;

void getargs(int *, int *, int *, int, char *[]);
void *produce(int);
void *consume();
void printbuf();

#endif /* __SERVER_H__ */
