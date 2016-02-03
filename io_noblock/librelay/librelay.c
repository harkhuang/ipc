#include <stdio.h>
#define RELAY_JOBMAX    4096
#define BUFSIZE         1024
#define REL_JOBMAX      100


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

enum {
	STATE_RUNNING=1,
	STATE_CANCELED,
	STATE_TERMINATED
};

struct relay_jobstat_st {
	int fd1, fd2;
	int64_t count12, count21;
	time_t start, end;
	int state;
};

int relay_addjob(int fd1, int fd2);

int relay_canceljob(int);

int relay_statjob(int, struct relay_jobstat_st*);

int relay_waitjob(int, struct relay_jobstat_st*);


//define a enum for file state
enum {
	ST_R=1,
	ST_W,
	ST_Ex,
	ST_T
};

// use for final state machine
struct fsa_relay_st {
	int state;
	int sfd, dfd;
	char buf[BUFSIZE];
	int len, pos;
	char *errstr;
	int64_t count;//use for record the event size
};

//descripte the job 
struct relay_job_st {
	int fd1, fd2;
	int fd1_saveflag, fd2_saveflag;
	struct fsa_relay_st fsa12, fsa21;
	int state;
	time_t start, end;
	pthread_mutex_t  mut;
	pthread_cond_t	cond;
};



static int a_bo;
//job  
static struct relay_job_st *job[RELAY_JOBMAX];

//init mutex job
static pthread_mutex_t	mut_job = PTHREAD_MUTEX_INITIALIZER;

//init a driver mutex
static pthread_t tid_driver;

//init a one pthread 
static pthread_once_t	init_once = PTHREAD_ONCE_INIT;


/* desc:deal the file state
   in:input the sa relay struct 
   out:null
*/   
static void fsa_driver(struct fsa_relay_st *fsa)
{
	int ret;
	//deal with   the file 
	switch (fsa->state) {

		//deal with write able
		case ST_R:
			fsa->len = read(fsa->sfd, fsa->buf, BUFSIZE);
			if (fsa->len==0) {
				fsa->state = ST_T;
			} else if (fsa->len<0) {
				if (errno==EAGAIN) {
					/* Do nothing */
				} else {
					fsa->errstr = "read(sfd)";
					fsa->state = ST_Ex;//this is a exception
				}
			} else {
				fsa->pos = 0;
				fsa->state = ST_W;
			}
			break;
		//deal with read able 
		case ST_W:
			ret = write(fsa->dfd, fsa->buf+fsa->pos, fsa->len);
			if (ret<0) {
				if (errno==EAGAIN) {
					/* Do nothing */
				} else {
					fsa->errstr = "write(dfd)";
					fsa->state = ST_Ex;
				}
			} else {
				fsa->pos += ret;
				fsa->len -= ret;
				fsa->count += ret;
				if (fsa->len>0) {
					/* Do nothing */
				} else {
					fsa->state = ST_R;
				}
			}
			break;
		//deal with error file description
		case ST_Ex:
			perror(fsa->errstr);
			fsa->state = ST_T;
			break;

		//deal with termination	
		case ST_T:
			/* Do nothing */
			break;
		default:
			raise(SIGABRT);
			break;
	}
}



//daily routine  
static void *thr_relayer(void *unused)
{
	int i;
	//loop
	while (1) {
		for (i=0;i<REL_JOBMAX;++i) {
			//lock mut job
			pthread_mutex_lock(&mut_job);
			if (job[i]) {
				//try to get the lock then do something
				if (pthread_mutex_trylock(&job[i]->mut)==0) {
					if (job[i]->state==STATE_RUNNING) {
						fsa_driver(&job[i]->fsa12);
						fsa_driver(&job[i]->fsa21);
						if (job[i]->fsa12.state==ST_T && job[i]->fsa21.state==ST_T) {
							job[i]->end = time(NULL);
							job[i]->state = STATE_TERMINATED;
						}
					}
					//broadcast to system the status is finshed
					pthread_cond_broadcast(&job[i]->cond);

					//unlock the mutex max
					pthread_mutex_unlock(&job[i]->mut);
				}
			}
			//unlock mutex job
			pthread_mutex_unlock(&mut_job);
		}
	}
}

//free some idle job code 
static int get_free_pos_unlocked(void)
{
	int i;

	for (i=0;i<REL_JOBMAX;++i) {
		if (job[i]==NULL) {
			return i;
		}
	}
	return -1;
}

//create job pthread
static void  module_load(void )
{
	int err = 0;
	err = pthread_create(&tid_driver, NULL, thr_relayer, NULL);
	if (err) {
		perror("");
		exit(1);
	}
}

//relay job   diliver job to add
//input:two file description to this pthread job 
int relay_addjob(int fd1, int fd2)
{
	int pos;

	//job diliver struct
	struct relay_job_st *self;

	//create this job just once 
	pthread_once(&init_once, module_load);

	//malloc memory
	self = malloc(sizeof(*self));

	//init job diliver struct 
	self->fd1 = fd1;
	self->fd2 = fd2;
	self->state = STATE_RUNNING;

	//get the file flags
	self->fd1_saveflag = fcntl(fd1, F_GETFL);
	fcntl(fd1, F_SETFL, self->fd1_saveflag|O_NONBLOCK);
	self->fd2_saveflag = fcntl(fd2, F_GETFL);
	fcntl(fd2, F_SETFL, self->fd2_saveflag|O_NONBLOCK);
	self->start = time(NULL);
	self->end = -1;


	//this two interface also use together
	pthread_mutex_init(&self->mut, NULL);
	pthread_cond_init(&self->cond, NULL);

	self->fsa12.sfd = fd1;
	self->fsa12.dfd = fd2;
	self->fsa12.state = ST_R;
	self->fsa21.sfd = fd2;
	self->fsa21.dfd = fd1;
	self->fsa21.state = ST_R;


	pthread_mutex_lock(&mut_job);
	pos = get_free_pos_unlocked();
	job[pos]=self;
	pthread_mutex_unlock(&mut_job);

	return pos;
}


//diliver job to cancel
int relay_canceljob(int id)
{
	pthread_once(&init_once, module_load);


	if (1) { /* do some thing*/}

	pthread_mutex_lock(&job[id]->mut);
	if (job[id]->state != STATE_RUNNING) {
		pthread_mutex_unlock(&job[id]->mut);
		return -errno;
	}
	//cancle job and broadcast signal to system
	job[id]->state = STATE_CANCELED;
	job[id]->end = time(NULL);
	pthread_cond_broadcast(&job[id]->cond);
	pthread_mutex_unlock(&job[id]->mut);
}


//get job status by job id
int relay_statjob(int id, struct relay_jobstat_st *res)
{
	pthread_once(&init_once, module_load);

	if (1) {
	}

	pthread_mutex_lock(&job[id]->mut);
	res->fd1 = job[id]->fd1;
	res->fd2 = job[id]->fd2;
	res->count12 = job[id]->fsa12.count;
	res->count21 = job[id]->fsa21.count;
	res->start = job[id]->start;
	res->end = job[id]->end;
	res->state = job[id]->state;
	pthread_mutex_unlock(&job[id]->mut);

	return 0;
}

//diliver job with job id
int relay_waitjob(int id, struct relay_jobstat_st *res)
{
	pthread_once(&init_once, module_load);

	// TODO:  do wait job
	if (1) {
		printf("do wait job...");
	}

	pthread_mutex_lock(&job[id]->mut);

	//get job id 
	while (job[id]->state == STATE_RUNNING) {
		pthread_cond_wait(&job[id]->cond, &job[id]->mut);
	}
	res->fd1 = job[id]->fd1;
	res->fd2 = job[id]->fd2;
	res->count12 = job[id]->fsa12.count;
	res->count21 = job[id]->fsa21.count;
	res->start = job[id]->start;
	res->end = job[id]->end;
	res->state = job[id]->state;
	pthread_mutex_unlock(&job[id]->mut);

	pthread_mutex_lock(&mut_job);
	pthread_mutex_destroy(&job[id]->mut);
	pthread_cond_destroy(&job[id]->cond);
	fcntl(job[id]->fd1, F_SETFL, job[id]->fd1_saveflag);
	fcntl(job[id]->fd2, F_SETFL, job[id]->fd2_saveflag);
	free(job[id]);
	job[id]=NULL;
	pthread_mutex_unlock(&mut_job);

	return 0;

}

