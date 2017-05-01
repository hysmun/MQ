#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mqueue.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

int initMQ(void);
int finishMQ(void);

int sendMsg(int mq, char *message, int size);

int fctLecture(void);
int fctTri(void);
int fctAffichage(void);

struct sigevent event;
struct mq_attr attr;
int ret_val;
int pidLec;
int pidTri;
int pidAff;

int mq_lectri;
int mq_triaff;

int main(int argc, char *argv[])
{
	initMQ();

	if((pidLec = fork()) == 0)
	{
		//lecture
		fctLecture();
	}
	if((pidLec = fork()) == 0)
	{
		//tri
		fctTri();
	}
	if((pidLec = fork()) == 0)
	{
		//affichage
		fctAffichage();
	}

	wait(NULL);
	wait(NULL);
	wait(NULL);

	finishMQ();
	return EXIT_SUCCESS;
}

int fctLecture(void)
{
	exit(0);
}

int fctTri(void)
{
	exit(0);
}

int fctAffichage(void)
{
	exit(0);
}

int sendMsg(int mq, char *message, int size)
{
	int ret;
	ret = mq_send (mq,message,size,1);
	if (ret == -1)
		perror ("\n\rSend : mq_send failed !!!");
	return ret;
}

int receiveMsg(int mq, char *message, int size)
{
	int ret;
	unsigned int priority;
	ret = mq_receive(mq,message,size,&priority);
	if (ret == -1)
		perror ("\n\rReceive : mq_receive failed !!!");
	return ret;
}






int initMQ(void)
{
	attr.mq_maxmsg = 20;
	attr.mq_msgsize = 10;
	attr.mq_flags = 0;

	mq_lectri = mq_open ("/tmp/lectri",O_CREAT|O_RDWR,0777,&attr);
	if (mq_lectri == -1)
		perror ("\n\rMAIN : mq_open failed !!!");

	mq_triaff = mq_open ("/tmp/triaff",O_CREAT|O_RDWR,0777,&attr);
	if (mq_triaff == -1)
		perror ("\n\rMAIN : mq_open failed !!!");
	return 1;
}

int finishMQ(void)
{
	ret_val = mq_close (mq_lectri);
	if (ret_val == -1)
		perror ("\n\rMAIN : mq_close failed !!!");
	ret_val = mq_unlink ("/tmp/lectri");
	if (ret_val == -1)
		perror ("\n\rMAIN : mq_unlink failed !!!");
	ret_val = mq_close (mq_triaff);
	if (ret_val == -1)
		perror ("\n\rMAIN : mq_close failed !!!");
	ret_val = mq_unlink ("/tmp/triaff");
	if (ret_val == -1)
		perror ("\n\rMAIN : mq_unlink failed !!!");
	return 1;
}







