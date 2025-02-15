#ifndef __SCHED_H__
#define __SCHED_H__

typedef void (*task_handler)(void);

void sched_start(uint8_t prio, task_handler task);
void sched_post(uint8_t prio);
void sched_run(void);

#endif // __SCHED_H__
