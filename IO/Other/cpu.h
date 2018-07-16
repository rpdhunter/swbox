/*
 * thread or process set running on special core
 */

#ifndef __CPU_H__
#define __CPU_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CPU_0 (0x0001)
#define CPU_1 (0x0002)
#define CPU_2 (0x0004)
#define CPU_3 (0x0008)
#define CPU_4 (0x0010)
#define CPU_5 (0x0020)
#define CPU_6 (0x0040)
#define CPU_7 (0x0080)
#define CPU_8 (0x0100)
#define CPU_9 (0x0200)

extern unsigned int set_process_cpu(unsigned int pid, unsigned int cpu_mask);
extern unsigned int set_thread_cpu(pthread_t tid, unsigned int cpu_mask);
extern int show_process_cpu(char * task_name_p);
extern int show_process_cpu_one_time(char *task_name_p);



#ifdef __cplusplus
}
#endif

#endif /* __CPU_H__ */

