#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define GRIMOIRE 20
#define SEM 21
#define SEG 101
#define CAI 201

#define NB_MAX 100

int temps_discussion_client_vendeur = 2;
int borne_basse_prix = 5;
int borne_haute_prix = 100;
int chance_achat = 2; //Chance de 1/n d'avoir un achat

#endif /* _CONFIG_H_ */
