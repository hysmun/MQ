#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mqueue.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define NBRROUTES 10
typedef struct ROUTES
{
	int nbrRoute;
	double duree;
}ROUTES;

int initMQ(void);
int finishMQ(void);

int sendMsg(int mq, char *message, int size);
int receiveMsg(int mq, char *message, int size);
double drand();
int swap(ROUTES *p1, ROUTES *p2);

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

/*
  	Nous allons réaliser une application de classification
  	de grandeurs mesurées sur un routeur fictif.
	Un premier processus appelé PROCESSUS LECTURE va lire
	les valeurs du temps de cheminement des paquets suivant
	plusieurs routes. Ce système sera en fait un tableau
	de deux colonnes : une colonne pour le n° de la ROUTE
	et une autre pour la DUREE du transfert.
	Ce premier processus va donc lire les données du routeur
	et va les envoyer via une message queue vers
	un deuxième processus appelé le PROCESSUS TRI.
	Celui-ci doit nous permettre de retrouver les valeurs
	des routes classées d’abord  par numéro de route
	et pour chaque route  les durées seront triées par ordre croissant.
	Finalement, le PROCESSUS TRI va envoyer ces données triées
	via messages queues vers un troisième processus appelé
	le PROCESSUS AFFICHAGE. Ce dernier lira les données
	et fera une mise en page élémentaire pour l’affichage des résultats.
 */
int main(int argc, char *argv[])
{
	initMQ();
	srand(time(NULL));

	printf("Fork lecture\n");
	if((pidLec = fork()) == 0)
	{
		//lecture
		fctLecture();
	}
	printf("Fork tri\n");
	if((pidTri = fork()) == 0)
	{
		//tri
		fctTri();
	}
	printf("Fork affichage\n");
	if((pidAff = fork()) == 0)
	{
		//affichage
		fctAffichage();
	}
	printf("MAIN : lec %d -- tri %d -- aff %d\n", pidLec, pidTri, pidAff);

	wait(NULL);
	wait(NULL);
	wait(NULL);
	printf("Fin wait\n");

	finishMQ();
	return EXIT_SUCCESS;
}

int fctLecture(void)
{
	int i;
	ROUTES tRoutes[NBRROUTES];
	printf("Debut Lecture\n");

	//generation valeur
	for(i=0; i< NBRROUTES; i++)
	{
		tRoutes[i].nbrRoute = i;
		tRoutes[i].duree = drand();
	}

	//envois des données à tri
	for(i=0; i<NBRROUTES; i++)
	{
		sendMsg(mq_lectri, (char *)&tRoutes[i],sizeof(ROUTES));
	}

	exit(0);
}

int fctTri(void)
{
	int i, j;
	ROUTES tRoutes[NBRROUTES];
	printf("Debut Tri\n");

	//reception données de lecture
	for(i=0; i<NBRROUTES; i++)
	{
		receiveMsg(mq_lectri, (char *)&tRoutes[i],sizeof(ROUTES));
	}

	//tri
	for(i=0; i<NBRROUTES; i++)
	{
		for(j = NBRROUTES-1; j>i ;j--)
		{
			if(tRoutes[j].duree < tRoutes[j-1].duree)
			{
				swap(&tRoutes[j], &tRoutes[j-1]);
			}
		}
	}

	//envois données à affichage
	for(i=0; i<NBRROUTES; i++)
	{
		sendMsg(mq_triaff, (char *)&tRoutes[i],sizeof(ROUTES));
	}

	exit(0);
}

int fctAffichage(void)
{
	int i, j;
	ROUTES tRoutes[NBRROUTES];
	printf("Debut Affichage\n");

	//reception données de lecture
	for(i=0; i<NBRROUTES; i++)
	{
		receiveMsg(mq_triaff, (char *)&tRoutes[i],sizeof(ROUTES));
	}

	//affichage
	printf("Affichage :\n\n");
	for(i=0; i<NBRROUTES; i++)
	{
		printf("%d -- routes n %d a une duree de %lf seconde\n", i, tRoutes[i].nbrRoute, tRoutes[i].duree);
		fflush(stdout);
	}
	printf("\n");
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
		perror ("\n\rReceive : mq_receive failed !!!\n");
	return ret;
}

double drand()
{
	return ((double)rand())/((double)rand());
}

int swap(ROUTES *p1, ROUTES *p2)
{
	ROUTES tmp;
	tmp.nbrRoute = p1->nbrRoute;
	tmp.duree = p1->duree;
	p1->nbrRoute = p2->nbrRoute;
	p1->duree = p2->duree;
	p2->nbrRoute = tmp.nbrRoute;
	p2->duree = tmp.duree;
	return 1;
}




int initMQ(void)
{
	printf("Debut init\n");
	attr.mq_maxmsg = NBRROUTES+1;
	attr.mq_msgsize = sizeof(ROUTES);
	attr.mq_flags = 0;

	mq_lectri = mq_open ("/tmp/lectri",O_CREAT|O_RDWR,0777,&attr);
	if (mq_lectri == -1)
		perror ("\n\rMAIN : mq_open failed !!!");

	mq_triaff = mq_open ("/tmp/triaff",O_CREAT|O_RDWR,0777,&attr);
	if (mq_triaff == -1)
		perror ("\n\rMAIN : mq_open failed !!!");
	printf("Fin init !\n");
	return 1;
}

int finishMQ(void)
{
	printf("Debut finish mq \n");
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
	printf("Fin finish mq\n");
	return 1;
}







