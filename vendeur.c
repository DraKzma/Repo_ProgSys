#include "config.h"

//Fonction qui gere la reception de SIGUSR1
void Start(){
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
    fprintf(fichier_log, "[VENDEUR]: SIGUSR2 recu, je meurs bye.\n");
    fclose(fichier_log);
    printf("[VENDEUR]: SIGUSR2 recu, je meurs bye !\n");
    exit(EXIT_SUCCESS);
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

int main(int argc, char* argv[], char* envp[]){

    //Variables

    int numero_vendeur;
    int choix_rayon;
    int* grimoire;
    int taille_grimoire;
    int* zone;
    int rayon_client;
    int choix_achat;
    int prix;

    int cle, cle_grimoire, cle_zone;
    int id_ens_sem;
    int id_seg_grimoire, id_seg_zone;
    FILE* fichier_cle;
    FILE* fichier_log;
    struct sigaction sig1;
    struct sigaction sig2;

    sigset_t E;

    //Initialisation de la graine

    srand(time(NULL)+getpid());

    //Gestion des arguments

    numero_vendeur = atoi(argv[1]);
    taille_grimoire = atoi(argv[2]);

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

    //Recuperation des cle

    cle = ftok("key.txt", SEM);
    if(cle == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    cle_grimoire = ftok("key.txt", GRIMOIRE);
    if(cle_grimoire == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    cle_zone = ftok("key.txt", SEG+numero_vendeur);
    if(cle_zone == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //Recuperation des SMP

    id_seg_grimoire = shmget(cle_grimoire, sizeof(int)*taille_grimoire, 0);
    if(id_seg_grimoire == -1){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    id_seg_zone = shmget(cle_zone, sizeof(int)*3, 0);
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

    grimoire = shmat(id_seg_grimoire, NULL, 0);
    zone = shmat(id_seg_zone, NULL, 0);

    //Choix du rayon pris en charge par le vendeur

    P(0, 1, id_ens_sem);
    choix_rayon = rand()%taille_grimoire;
    while(grimoire[choix_rayon] != -1){
        choix_rayon = rand()%taille_grimoire;
    }
    grimoire[choix_rayon] = numero_vendeur;
    V(0, 1, id_ens_sem);

    //Affichage de debut

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[VENDEUR%d]: Hello world. J'attends maintenant le debut de la simulation.\n", numero_vendeur);
    fclose(fichier_log);
    printf("[VENDEUR%d]: Hello world. J'attends maintenant le debut de la simulation.\n", numero_vendeur);

    //Attente du signal de debut 

    sigsuspend(&E);

    //Communication avec les clients
    while(1){

        //On attend qu'un client se presente

        P(1+(numero_vendeur*2)+1, 1, id_ens_sem);

        //Ecriture dans le fichier de log
        fichier_log = fopen("log.txt", "a");
        if(fichier_log == NULL){
            printf("ERREUR d'ouverture du fichier_log\n");
            exit(EXIT_FAILURE);
        }
        fprintf(fichier_log, "[VENDEUR%d]: client detectee.\n", numero_vendeur);
        fclose(fichier_log);
        printf("[VENDEUR%d]: client detectee.\n", numero_vendeur);

        //Lecture du rayon du client
        rayon_client = zone[0];
        if(rayon_client == choix_rayon){
            //On decide si un achat se fait ou non
            sleep(temps_discussion_client_vendeur);
            choix_achat = (rand()+getpid())%chance_achat;
            if(choix_achat){
                //Achat effectuee

                prix = (rand()%(borne_haute_prix-borne_basse_prix+1))+borne_basse_prix;
                zone[2] = prix;

                //printf et liberation du mutex
                //Ecriture dans le fichier de log
                fichier_log = fopen("log.txt", "a");
                if(fichier_log == NULL){
                    printf("ERREUR d'ouverture du fichier_log\n");
                    exit(EXIT_FAILURE);
                }
                fprintf(fichier_log, "[VENDEUR%d]: Vente effectuee avec le client.\n", numero_vendeur);
                fclose(fichier_log);
                printf("[VENDEUR%d]: Vente effectuee avec le client.\n", numero_vendeur);
                V(1+(numero_vendeur*2)+1, 1, id_ens_sem);
                sleep(1);
            }
            else{
                //Achat non effectuee

                //printf et liberation du mutex
                //Ecriture dans le fichier de log
                fichier_log = fopen("log.txt", "a");
                if(fichier_log == NULL){
                    printf("ERREUR d'ouverture du fichier_log\n");
                    exit(EXIT_FAILURE);
                }
                fprintf(fichier_log, "[VENDEUR%d]: Vente avec le client non effectuee.\n", numero_vendeur);
                fclose(fichier_log);
                printf("[VENDEUR%d]: Vente avec le client non effectuee.\n", numero_vendeur);
                V(1+(numero_vendeur*2)+1, 1, id_ens_sem);
                sleep(1);
            }
        }
        else{
            //On redirige le client sur le bon vendeur

            zone[1] = grimoire[rayon_client];

            //printf et liberation du mutex
            //Ecriture dans le fichier de log
            fichier_log = fopen("log.txt", "a");
            if(fichier_log == NULL){
                printf("ERREUR d'ouverture du fichier_log\n");
                exit(EXIT_FAILURE);
            }
            fprintf(fichier_log, "[VENDEUR%d]: Je redirige le client sur le vendeur %d.\n", numero_vendeur, grimoire[rayon_client]);
            fclose(fichier_log);
            printf("[VENDEUR%d]: Je redirige le client sur le vendeur %d.\n", numero_vendeur, grimoire[rayon_client]);
            V(1+(numero_vendeur*2)+1, 1, id_ens_sem);
            sleep(1);   
        }
    }

    //Fin du programme

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[VENDEUR%d]: Je meurs bye.\n", numero_vendeur);
    fclose(fichier_log);
    printf("[VENDEUR%d]: Je meurs bye.\n", numero_vendeur);
    exit(EXIT_SUCCESS);
}