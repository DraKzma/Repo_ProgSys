#include "config.h"

//Fonction qui gere la reception de SIGUSR1
void Start(){
}

//Fonction P() sur les semaphores
int P(int sem, int n, int id_sem){
    struct sembuf op ={sem, -n, SEM_UNDO};

    return semop(id_sem, &op, 1);
}

//Fonction V() sur les semaphores
int V(int sem, int n, int id_sem){
    struct sembuf op ={sem, n, SEM_UNDO};

    return semop(id_sem, &op, 1);
}

//Fonction qui gere la reception de SIGUSR2
void End(){

    //Variables

    FILE* fichier_log;

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CAISSIER]: SIGUSR2 recu, je meurs bye.\n");
    fclose(fichier_log);
    printf("[CAISSIER]: SIGUSR2 recu, je meurs bye.\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[], char* envp[]){

    //Variables

    int numero_caissier;
    int* zone;
    int nb_vendeurs;

    struct sigaction sig1;
    struct sigaction sig2;
    FILE* fichier_cle;
    FILE * fichier_log;
    int cle, cle_zone;
    int id_seg_zone;
    int id_ens_sem;

    sigset_t E;

    //Gestion des arguments

    numero_caissier = atoi(argv[1]);
    nb_vendeurs = atoi(argv[2]);

    //Gestion des masques et sigactions

    sigemptyset(&E);
    sigfillset(&E);
    sigdelset(&E, SIGUSR1);
    sigdelset(&E, SIGUSR2);
    sigprocmask(SIG_BLOCK, &E, NULL);
    sig1.sa_handler = Start;
    sig2.sa_handler = End;
    sigaction(SIGUSR1, &sig1, NULL);
    sigaction(SIGUSR2, &sig2, NULL);

    //Creation du fichier cle

    fichier_cle = fopen("key.txt", "w");
    if(fichier_cle == NULL){
        printf("ERREUR d'ouverture du fichier_cle\n");
        exit(EXIT_FAILURE);
    }
    fclose(fichier_cle);

    //Recuperation des cles

    cle = ftok("key.txt", SEM);
    if(cle == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    cle_zone = ftok("key.txt", CAI+numero_caissier);
    if(cle_zone == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //Recuperation des SMP

    id_seg_zone = shmget(cle_zone, sizeof(int)*2, 0);
    if(id_seg_zone == -1){
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    //Recuperation du SEM

    id_ens_sem = semget(cle, 1, 0);
    if(id_ens_sem == -1){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    
    //Attachement du SMP

    zone = shmat(id_seg_zone, NULL, 0);

    //Affichage de debut

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CAISSIER%d]: Hello world. J'attends maintenant le debut de la simulation.\n", numero_caissier);
    fclose(fichier_log);
    printf("[CAISSIER%d]: Hello world. J'attends maintenant le debut de la simulation.\n", numero_caissier);

    //Attente du signal de debut

    sigsuspend(&E);

    //Communication avec le client

    while(1){

        //Attente de l'arrivee d'un client
        P(1+(nb_vendeurs*2)+(numero_caissier*2)+1, 1, id_ens_sem);

        //Ecriture dans le fichier de log
        fichier_log = fopen("log.txt", "a");
        if(fichier_log == NULL){
            printf("ERREUR d'ouverture du fichier_log\n");
            exit(EXIT_FAILURE);
        }
        fprintf(fichier_log, "[CAISSIER%d]: Le client %d a paye le prix %d avec succes.\n", numero_caissier, zone[0], zone[1]);
        fclose(fichier_log);

        printf("[CAISSIER%d]: Le client %d a paye le prix %d avec succes.\n", numero_caissier, zone[0], zone[1]);

        //Liberation de la ressource pour le prochain client
        V(1+(nb_vendeurs*2)+(numero_caissier*2), 1, id_ens_sem);
    }

    //Fin du programme

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CAISSIER%d]: Je meurs bye.\n", numero_caissier);
    fclose(fichier_log);
    printf("[CAISSIER%d]: Je meurs bye.\n", numero_caissier);
    exit(EXIT_SUCCESS);
}