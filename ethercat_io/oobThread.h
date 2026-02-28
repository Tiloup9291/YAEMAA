#ifndef _OOB_THREAD_H_
#define _OOB_THREAD_H_

#include <pthread.h>

void* oobThread(void* arg);
int initThread(void);
struct timespec timespec_add(struct timespec time1, struct timespec time2);

#endif /* _OOB_THREAD_H_ */
