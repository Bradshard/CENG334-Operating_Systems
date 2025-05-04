#include "hw2.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#define printl(lk, format, ...)                                                \
  pthread_mutex_lock(&lk);                                                     \
  printf(format, __VA_ARGS__);                                                 \
  pthread_mutex_unlock(&lk)

#define CWAIT 1000
#define SWAIT 1000

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
int simOver = 0;
int maxOrder;

struct Supply {
  int item;
  int n;
};

int parseStore(char *arg, int cap[3]) {
  char *p = strtok(arg, ":");
  int i = 0, r;
  while (p != NULL) {
    r = atoi(p);
    if (i < 3)
      cap[i] = r;

    p = strtok(NULL, ":");
    i++;
  }
  return r;
}

int parseSuppliers(char *arg, int sup[]) {
  char *p = strtok(arg, ":");
  int i = 0, r;
  (void)r;
  while (p != NULL) {
    sup[i++] = atoi(p);

    p = strtok(NULL, ":");
  }
  return i;
}

void *customer(void *n) {
  pthread_detach(pthread_self());
  while (!simOver) {
    int a, b, c;
    usleep(rand() % CWAIT + 10);
    while (1) {
      a = rand() % maxOrder;
      b = rand() % maxOrder;
      c = rand() % maxOrder;
      if (a + b + c > 0)
        break;
    }
    printl(mut, "buying %d,%d,%d\n", a, b, c);
    buy(a, b, c);
    printl(mut, "bought %d,%d,%d\n", a, b, c);
  }
  return NULL;
}

void *supplier(void *p) {
  Supply *s = (Supply *)p;
  pthread_detach(pthread_self());

  while (!simOver) {
    int c[3], a[3];
    printl(mut, "maysupply: %d %d\n", s->item, s->n);
    maysupply(s->item, s->n);
    printl(mut, "maysupplying: %d %d\n", s->item, s->n);
    usleep(rand() % SWAIT + 10);
    supply(s->item, s->n);
    printl(mut, "supplied: %d %d\n", s->item, s->n);
    monitorStore(c, a);
    printl(mut, "store: %d/%d %d/%d %d/%d\n", a[0], c[0], a[1], c[1], a[2],
           c[2]);
  }
  printl(mut, "sim over, supplier: %d\n", s->item);
  return NULL;
}

int main(int argc, char *argv[]) {
  int cap[3];
  int suppliers[100];
  int ncust;
  int nsup;
  int i;

  maxOrder = parseStore(argv[1], cap);
  ncust = atoi(argv[2]);
  nsup = parseSuppliers(argv[3], suppliers);

  initStore(cap[0], cap[1], cap[2], maxOrder);

  for (i = 0; i < ncust; i++) {
    pthread_t tid;
    pthread_create(&tid, NULL, customer, NULL);
  }
  for (i = 0; i < nsup; i++) {
    pthread_t tid;
    Supply *s;
    if (suppliers[i]) {
      s = new Supply;
      s->item = i % 3;
      s->n = suppliers[i];
      pthread_create(&tid, NULL, supplier, (void *)s);
    }
  }

  sleep(1);
  simOver = 1;
  sleep(2);
  return 0;
}
