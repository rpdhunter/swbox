/********************************************************************
	created:	2014/02/26
	created:	26:2:2014   11:00
	filename: 	Z:\prog\nr-switch-prog\src\comm_lib\src_lib\sync.c
	file path:	Z:\prog\nr-switch-prog\src\comm_lib\src_lib
	file base:	sync
	file ext:	c
	author:		zhouxf
	purpose:	Inter-Process Communication operations
*********************************************************************/

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/timeb.h>
#include <sys/shm.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "ipcs.h"

/********************************************************************
	purpose:	define semophore operations
*********************************************************************/

static int sem_p(int semid);
static int sem_timed_p(int semid, int timeout_s);
static int sem_v(int semid);


/*
 * supports timeout version for sem_wait
 */
int sem_timewait (sem_t * sem, const struct timespec * abs_timeout)
{
#if 1
	unsigned int usec;
	int num, cnt, rv = -1;

	if (sem == NULL) {
		return rv;
	}

	if (abs_timeout == NULL) {
		return sem_wait (sem);
	}
	else {
		usec = (abs_timeout->tv_sec * 1000000) + (abs_timeout->tv_nsec / 1000);
		if (usec < 1000) {
			num = 100;
			cnt = (usec + 99) / num;
		}
		else if (usec >= 1000 && usec < 10000) {
			num = 1000;
			cnt = (usec + 999) / num;
		}
		else {
			num = 10000;
			cnt = (usec + 9999) / num;
		}
		for (;;) {
			if (sem_trywait (sem) == 0) {
				rv = 0;
				break;
			}
			if (cnt-- > 0) {
				usleep (num);
			}
			else {
				break;
			}
		}
	}

	return rv;
#else
	extern int sem_timedwait (sem_t * , const struct timespec * );

	if (abs_timeout == NULL) {
		return sem_wait (sem);
	}
	else {
		return sem_timedwait (sem, abs_timeout);
	}
#endif
}

union semun 
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};


/********************************************************************
*功能:创建进程互斥的信号量。原来不存在，则创建，赋值；存在则直接使用，
      不重新赋值
*参数:key,信号量标识
*参数:value,信号量值.
*返回值：成功返回创建的semid值，出错返回-1.
********************************************************************/
extern int create_sem(key_t key, int value)
{
	union semun sem;
	int semid;
	sem.val = value;
	
	if((semid = semget(key, 1, 0666|IPC_CREAT|IPC_EXCL)) == -1) 
    { 
	    if(errno == EEXIST) 
	    { 
		    if((semid = semget(key, 1, 0666|IPC_CREAT)) == -1) 
		    {
		    	printf ("semget error\n");
				return -1;
		    }
		    else 
		    {
		    	return semid; /*如果已存在,不再初始化信号量*/ 
		    }
	    }
	    else
	    {
	    	printf ("semget error\n");
			return -1;
	    }
    } 
    /*设置信号量初值*/ 
    if(semctl(semid, 0, SETVAL, sem) == -1) 
    {
    	printf ("semctl SETVAL error\n");
		return -1;
    }

	return semid;
}

/********************************************************************
*功能:创建进程互斥的信号量，并指定semflg值。如果创建成功，用value值赋值
*参数:key,信号量标识
*参数:value,信号量值
*参数:semflg,信号量flag，如0666|IPC_CREAT|IPC_EXCL等
*返回值：成功返回创建的semid值，出错返回-1.
********************************************************************/
extern int create_sem_excl(key_t key, int value, int semflg)
{
	union semun sem;
	int semid;

	sem.val = value;
	
	if((semid = semget(key, 1, semflg)) == -1) 
    { 
    	printf ("semget error\n");
		return -1;
    } 
    /*设置信号量初值*/ 
    if(semctl(semid, 0, SETVAL, sem) == -1) 
    {
    	printf ("semctl SETVAL error\n");
		return -1;
    }

	return semid;
}

/********************************************************************
*功能:设置进程互斥的信号量值
*参数:semid,信号量
*参数:value,信号量值
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int setvalue_sem(int semid, int value)
{
	union semun sem;
	
	sem.val = value;
	return semctl(semid, 0, SETVAL, sem);
}

/********************************************************************
*功能:读取进程互斥的信号量值
*参数:semid,信号量
*返回值：成功返回读到的值，出错返回-1.
********************************************************************/
extern int getvalue_sem(int semid)
{
	union semun sem;
	
    return semctl(semid, 0, GETVAL, sem);
}

/********************************************************************
*功能:删除进程互斥的信号量
*参数:semid,信号量
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int destroy_sem(int semid)
{
	union semun sem;
	
	sem.val = 0;
	return semctl(semid, 0, IPC_RMID, sem);
}

static int sem_p(int semid)
{
	struct sembuf sops = {0, -1, SEM_UNDO/*IPC_NOWAIT*/};

	return (semop(semid, &sops, 1));
}

/*timeouts-超时时间，单位秒。*/
static int sem_timed_p(int semid, int timeout_s)
{
	struct timespec timeout;
	struct sembuf sops = {0, -1, SEM_UNDO/*IPC_NOWAIT*/};
	
	timeout.tv_sec = timeout_s;
	timeout.tv_nsec = 0;
	return 0;
	//return (semtimedop(semid, &sops, 1, &timeout));
}

static int sem_v(int semid)
{
	struct sembuf sops = {0, +1, SEM_UNDO/*IPC_NOWAIT*/};
	
	return (semop(semid, &sops, 1));
}

/********************************************************************
*功能:获取进程互斥的信号量
*参数:semid,信号量id
*参数:timeout_s,超时时间,单位秒。用PRCESS_WAIT_FOREVER宏表示一直等待.
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int process_mutex_take(int semid, int timeout_s)
{
	if(PROCESS_WAIT_FOREVER == timeout_s)
	{
		return sem_p(semid);
	}
	else
	{
		return sem_timed_p(semid, timeout_s);
	}
}

/********************************************************************
*功能:释放进程互斥的信号量
*参数:semid,信号量id
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int process_mutex_give(int semid)
{
	return sem_v(semid);
}

/********************************************************************
*功能:创建线程互斥的信号量
*参数:mutex,信号量指针
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_create(pthread_mutex_t *mutex)
{
	int rv;
	pthread_mutexattr_t attr;

	if (pthread_mutexattr_init (&attr) != 0) {
		return -1;
	}
	if (pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE_NP) != 0) {
		return -1;
	}
	rv = pthread_mutex_init (mutex, &attr);
	pthread_mutexattr_destroy (&attr);

	return rv;
}

/********************************************************************
*功能:删除线程互斥的信号量
*参数:mutex,信号量指针
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_delete(pthread_mutex_t *mutex)
{
	return pthread_mutex_destroy(mutex);
}

/********************************************************************
*功能:获取线程互斥的信号量
*参数:mutex,信号量指针
*参数:timeout_s,超时时间,单位秒。用PTHREAD_WAIT_FOREVER宏表示一直等待.
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_take(pthread_mutex_t *mutex, int timeout_s)
{
	if(PTHREAD_WAIT_FOREVER == timeout_s)
	{
		return pthread_mutex_lock(mutex);
	}
	else
	{
/*
		struct timespec abs_timeout;
		
		abs_timeout.tv_sec = timeout_s;
		abs_timeout.tv_nsec = 0;
		return pthread_mutex_timedlock(mutex, &abs_timeout);//有问题，不管用
*/
		int usec, err;
		int time_wait;
		
		if(timeout_s < 1)
		{
			timeout_s = 1;
		}
		
		usec = timeout_s * 1000 * 1000;
		time_wait = 100*1000;
		for (;;) 
		{
			err = pthread_mutex_trylock(mutex);

			if (err != EBUSY) 
			{
				break;                /* Done (0), or error other than EBUSY */
			}
			usleep(time_wait);
			
			usec -= time_wait;
			
			if (usec <= 0) 
			{
				err = ETIMEDOUT;
				break;
			}
		}
		
		if(err)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
}

/********************************************************************
*功能:释放线程互斥的信号量
*参数:mutex,信号量指针
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_give(pthread_mutex_t *mutex)
{
	return pthread_mutex_unlock(mutex);
}


/********************************************************************
	purpose:	msgq related operations
*********************************************************************/

static int get_random_key(key_t *key)
{
	*key = (key_t)rand();
	return 0;
}

/*用随机key创建一个队列*/
int create_msg_q(int *q_id)
{
	key_t key;
	int queue_id;
	int i;
	int try_time;
	struct timeb tmb;

	if(ftime(&tmb) != 0)
	{
		printf ("error, ftime\n");
		return -1;
	}

	try_time = 50;/*最多创建50次*/
//	printf("srand=%u, %u, %u\n\n", (unsigned int)tmb.time, (unsigned int)tmb.millitm, (unsigned int)getpid());
	srand((unsigned int)tmb.time + (unsigned int)tmb.millitm + (unsigned)getpid());
	
	for(i = 0; i < try_time; i++)
	{
		if(get_random_key(&key) != 0)
		{
			printf("create_msg_q: get_random_key error!\n");
			return -1;
		}
		
		queue_id = msgget(key, IPC_CREAT|IPC_EXCL|0666);
		if(queue_id == -1)
		{
//			printf("create_msg_q: %s, create time =%d, key=%u\n", strerror(errno), i, (unsigned int)key);
		}
		else
		{
			break;
		}
	}

	if(i >= try_time)
	{
		printf("create_msg_q: msgget failed!\n");
		return -1;
	}

	*q_id = queue_id;
	
	return 0;
}

/*
 * return value:
 * -1: error
 *  0: success
 *  1: created
 */
int msgq_create (int key, int * msgq_id)
{
	int id, ret = 0	/* success */;

	if (key == -1 || msgq_id == NULL) {
		return -1;
	}

	if ((id = msgget (key, MSGQ_GET_FLAG | MSGQ_TEST_EXIST)) == -1) {
		if (errno == EEXIST) {
			if ((id = msgget (key, MSGQ_GET_FLAG)) == -1) {
				ret = -1;	/* error */
			}
			else {
				ret = 1;	/* created */
			}
		}
		else {
			ret = -1;		/* other errors */
		}
	}

	* msgq_id = id;

	return ret;
}

int msgq_delete (int msgq_id)
{
	if (msgq_id == -1) {
		return -1;
	}

	if (msgctl (msgq_id, MSGQ_DEL_FLAG, NULL) != 0) {
		return -1;
	}
	else {
		return 0;
	}
}

int msgq_send (int msgq_id, char * msg, int len, int msg_type)
{
	int ret;
	msgbuf_t msgbuf;

	if (msgq_id == -1 || msg == NULL) {
		return -1;
	}

	if (len > MSG_CONTEX_SIZE) {
		printf ("msgq len %d is larger than %d\n", len, MSG_CONTEX_SIZE);
		return -1;
	}

	msgbuf.msg_type = msg_type;
	memcpy (&msgbuf.msg_contex, msg, len);
	ret = msgsnd (msgq_id, &msgbuf, len, MSGQ_SND_FLAG);

	return ret;
}

int msgq_recv (int msgq_id, char * msg, int len, int msg_type)
{
	int ret = 0;
	msgbuf_t msgbuf;

	if (msgq_id == -1 || msg == NULL) {
		return -1;
	}

	if (len > MSG_CONTEX_SIZE) {
		printf ("msgq len %d is larger than %d\n", len, MSG_CONTEX_SIZE);
		return -1;
	}

	if ((ret = msgrcv (msgq_id, &msgbuf, len, msg_type, MSGQ_RCV_FLAG)) != -1) {
		memcpy (msg, &msgbuf.msg_contex, ret <= len ? ret : len);
	}

	return ret;
}

/*带超时时间的队列接受函数，超时时间为time_ms*/
int msgrcv_timeout (int msqid, void *msgp, unsigned int msgsz, long msgtyp, int time_ms)
{
	#define EACH_RECV_TIME_MS 10
	int len;
	int i;

	if(((len = msgrcv(msqid, msgp, msgsz, msgtyp, IPC_NOWAIT)) == -1)
		&& (errno == ENOMSG))
	{
		for(i = 0; i < time_ms/EACH_RECV_TIME_MS; i++)
		{
			usleep(EACH_RECV_TIME_MS * 1000);
			
			if(((len = msgrcv(msqid, msgp, msgsz, msgtyp, IPC_NOWAIT)) == -1)
				&& (errno == ENOMSG))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	if((len == -1) && (errno == ENOMSG))
	{
		printf("msgrcv timeout: qmsqid = %d, time_ms = %d\n", msqid, time_ms);
	}
	
	return len;
}


/********************************************************************
	purpose:	shared memory related operations
*********************************************************************/

void * create_shared_memory (key_t key, int size)
{
	int shm_id, first = 0;
	void * shm_p;

	shm_id = shmget (key, size, 0666 | IPC_CREAT | IPC_EXCL);
	if (shm_id == -1) {
		shm_id = shmget (key, size, 0666 | IPC_CREAT);
		if (shm_id == -1) {
			return (void *)(-1);
		}
	}
	else {
		first = 1;
	}

	shm_p = shmat (shm_id, NULL, 0);

	if (first) {
		memset (shm_p, 0, size);
	}

	return shm_p;
}

/********************************************************************
	purpose:	event mechanism 
*********************************************************************/

int events_create (events_t * ev)
{
	if (sem_init (&ev->cnt_sem, 0, 0) < 0) {
		return -1;
	}

	if (sem_init (&ev->mutex_sem, 0, 1) < 0) {
		return -1;
	}

	ev->event_mask = 0;

	return 0;
}

unsigned int events_timewait (events_t * ev, struct timespec * abs_timeout)
{
	unsigned int ev_mask;

	if (sem_timewait (&ev->cnt_sem, abs_timeout) < 0) {
		return 0;
	}

	sem_timewait (&ev->mutex_sem, NULL);

	ev_mask = ev->event_mask;
	ev->event_mask = 0;

	sem_post (&ev->mutex_sem);

	return ev_mask;
}

int events_post (events_t * ev, unsigned int ev_mask)
{
	sem_timewait (&ev->mutex_sem, NULL);

	ev->event_mask |= ev_mask;

	sem_post (&ev->mutex_sem);

	sem_post (&ev->cnt_sem);

	return 0;
}
