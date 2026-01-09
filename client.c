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

//Fonction renvoyant l'indice de la file d'attente la moins occupee
int max(int id_ens_sem, int nb_vendeurs){
    int max;
    int indice_sem_max;
    int i;
    max = semctl(id_ens_sem, 1, GETVAL, NULL); // renvoie la valeur contenue dans le semaphore
    indice_sem_max = 1;
    for(i=3; i<1+(nb_vendeurs*2); i=i+2){ // on commence a 3 car l'indce 0 est reserve au grimoire
        if(semctl(id_ens_sem, i, GETVAL, NULL) > max){
            indice_sem_max = i;
            max = semctl(id_ens_sem, i , GETVAL, NULL);
        }
    }
    return indice_sem_max;
}

int main(int argc, char* argv[], char* envp[]){

    //Variables

    int numero_client;
    int nb_vendeurs;
    int nb_caissiers;
    int numero_vendeur; //numero du vendeur actuellement en communication avec le client
    int* zone;
    int* zone_caissier;
    int choix_rayon;
    int prix_achat;
    int choix_caissier;

    struct sigaction sig1;
    FILE* fichier_cle;
    FILE* fichier_log;
    int cle, cle_zone;
    int id_ens_sem;
    int id_seg_zone;
    int indice_sem_vendeur; //indice dans l'ensemble de semaphore du vendeur en communication courante avec le client
    int indice_sem_caissier; //indice dans l'ensemble de semaphore du caissier en communication courante avec le client
    int id_seg_zone_caissier;
    int cle_zone_caissier;

    sigset_t E;

    //Initialisation de la graine

    srand(time(NULL)+getpid());

    //Gestion des arguments

    numero_client = atoi(argv[1]);
    nb_vendeurs = atoi(argv[2]);
    nb_caissiers = atoi(argv[3]);

    //Gestion des masques et sigactions

    sigemptyset(&E);
    sigfillset(&E);
    sigdelset(&E, SIGUSR1);
    sigprocmask(SIG_BLOCK, &E, NULL);
    sig1.sa_handler = Start;
    sigaction(SIGUSR1, &sig1, NULL);

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

    //Recuperation du SEM

    id_ens_sem = semget(cle, 1, 0);
    if(id_ens_sem == -1){
        perror("semget");
        exit(EXIT_FAILURE);
    }

    //Affichage de debut

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CLIENT%d]: Hello world. J'attends maintenant le debut de la simulation.\n", numero_client);
    fclose(fichier_log);
    printf("[CLIENT%d]: Hello world. J'attends maintenant le debut de la simulation.\n", numero_client);

    //Attente du signal de debut

    sigsuspend(&E);

    //Separation des clients avec un sleep

    sleep(1*numero_client);

    //Recherche de l'indice dans l'ens de sem du vendeur avec la plus petite file

    indice_sem_vendeur = max(id_ens_sem, nb_vendeurs);

    //Recuperation du smp du vendeur choisi

    numero_vendeur = (indice_sem_vendeur-1)/2;
    cle_zone = ftok("key.txt", SEG+numero_vendeur);
    if(cle_zone == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    id_seg_zone = shmget(cle_zone, sizeof(int)*3, 0);
    if(id_seg_zone == -1){
        perror("shmget");
        printf("%d\n", numero_vendeur);
        exit(EXIT_FAILURE);
    }
    zone = shmat(id_seg_zone, NULL, 0);

    //Choix du rayon

    choix_rayon = rand()%nb_vendeurs; //Il y'a autant de rayons que de vendeurs

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CLIENT%d]: Je cherche le rayon %d.\n", numero_client, choix_rayon);
    fprintf(fichier_log, "[CLIENT%d]: Je me mets dans la file d'attente du vendeur %d.\n", numero_client, numero_vendeur);
    fclose(fichier_log);
    printf("[CLIENT%d]: Je cherche le rayon %d.\n", numero_client, choix_rayon);
    printf("[CLIENT%d]: Je me mets dans la file d'attente du vendeur %d.\n", numero_client, numero_vendeur);

    //Communication avec le vendeur
    P(indice_sem_vendeur, 1, id_ens_sem);
    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CLIENT%d]: Communication avec le vendeur en cours, je met mon numero de rayon recherchee dans zone[0].\n", numero_client);
    fclose(fichier_log);
    printf("[CLIENT%d]: Communication avec le vendeur en cours, je met mon numero de rayon recherchee dans zone[0].\n", numero_client);
    sleep(1);
    
    zone[0] = choix_rayon;
    V(indice_sem_vendeur+1, 1, id_ens_sem);
    sleep(1);
    P(indice_sem_vendeur+1, 1, id_ens_sem);

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CLIENT%d]: Le vendeur m'a redonne la main, je lis sa reponse.\n", numero_client);
    fclose(fichier_log);
    printf("[CLIENT%d]: Le vendeur m'a redonne la main, je lis sa reponse.\n", numero_client);
    if(zone[1] != -1){
        //Lecture de la reponse, reinitialisation de la zone du vendeur et V() sur le mutex
        numero_vendeur = zone[1];
        zone[0] = -1;
        zone[1] = -1;
        zone[2] = -1;
        V(indice_sem_vendeur, 1, id_ens_sem);

        //Recuperation du smp du vendeur choisi
        indice_sem_vendeur = (numero_vendeur*2)+1;
        cle_zone = ftok("key.txt", SEG+numero_vendeur);
        if(cle_zone == -1){
            perror("ftok");
            exit(EXIT_FAILURE);
        }
        id_seg_zone = shmget(cle_zone, sizeof(int)*3, 0);
        if(id_seg_zone == -1){
            perror("shmget");
            exit(EXIT_FAILURE);
        }
        zone = shmat(id_seg_zone, NULL, 0);
    
        //Ecriture dans le fichier de log
        fichier_log = fopen("log.txt", "a");
        if(fichier_log == NULL){
            printf("ERREUR d'ouverture du fichier_log\n");
            exit(EXIT_FAILURE);
        }
        fprintf(fichier_log, "[CLIENT%d]: J'ai trouvee le bon vendeur, c'est le vendeur %d, indice_sem %d. Je me met dans sa file d'attente\n", numero_client, numero_vendeur, indice_sem_vendeur);
        fclose(fichier_log);
        printf("[CLIENT%d]: J'ai trouvee le bon vendeur, c'est le vendeur %d, indice_sem %d. Je me met dans sa file d'attente\n", numero_client, numero_vendeur, indice_sem_vendeur);

        //Communication avec le nouveau vendeur

        P(indice_sem_vendeur, 1, id_ens_sem);
        //Ecriture dans le fichier de log
        fichier_log = fopen("log.txt", "a");
        if(fichier_log == NULL){
            printf("ERREUR d'ouverture du fichier_log\n");
            exit(EXIT_FAILURE);
        }
        fprintf(fichier_log, "[CLIENT%d]: Communication avec le vendeur en cours, je met mon numero de rayon recherchee dans zone[0].\n", numero_client);
        fclose(fichier_log);
        printf("[CLIENT%d]: Communication avec le vendeur en cours, je met mon numero de rayon recherchee dans zone[0].\n", numero_client);
        sleep(1);

        zone[0] = choix_rayon;
        V(indice_sem_vendeur+1, 1, id_ens_sem);
        sleep(1);
        P(indice_sem_vendeur+1, 1, id_ens_sem);
        //Ecriture dans le fichier de log
        fichier_log = fopen("log.txt", "a");
        if(fichier_log == NULL){
            printf("ERREUR d'ouverture du fichier_log\n");
            exit(EXIT_FAILURE);
        }
        fprintf(fichier_log, "[CLIENT%d]: Le vendeur m'a redonne la main, je lis sa reponse.\n", numero_client);
        fclose(fichier_log);
        printf("[CLIENT%d]: Le vendeur m'a redonne la main, je lis sa reponse.\n", numero_client);

        if(zone[2] != -1){
            //Lecture de la reponse, reinitialisation de la zone du vendeur et V() sur le mutex
            prix_achat = zone[2];
            zone[0] = -1;
            zone[1] = -1;
            zone[2] = -1;
            V(indice_sem_vendeur, 1, id_ens_sem);

            //Choix du caissier et mise en place des IPCS

            choix_caissier = (rand()+getpid())%nb_caissiers;
            indice_sem_caissier = 1+(nb_vendeurs*2)+(choix_caissier*2);

            //Recuperation du smp du caissier choisi

            cle_zone_caissier = ftok("key.txt", CAI+choix_caissier);
            if(cle_zone_caissier == -1){
                perror("ftok");
                exit(EXIT_FAILURE);
            }
            id_seg_zone_caissier = shmget(cle_zone_caissier, sizeof(int)*2, 0);
            if(id_seg_zone_caissier == -1){
                perror("shmget");
                exit(EXIT_FAILURE);
            }
            zone_caissier = shmat(id_seg_zone_caissier, NULL, 0);

            //Communication avec le caissier

            //Ecriture dans le fichier de log
            fichier_log = fopen("log.txt", "a");
            if(fichier_log == NULL){
                printf("ERREUR d'ouverture du fichier_log\n");
                exit(EXIT_FAILURE);
            }
            fprintf(fichier_log, "[CLIENT%d]: Je pars communiquer avec le caissier.\n", numero_client);
            fclose(fichier_log);
            printf("[CLIENT%d]: Je pars communiquer avec le caissier.\n", numero_client);
            P(indice_sem_caissier, 1, id_ens_sem);

            zone_caissier[0] = numero_client;
            zone_caissier[1] = prix_achat;

            V(indice_sem_caissier+1, 1, id_ens_sem);
            //Ecriture dans le fichier de log
            fichier_log = fopen("log.txt", "a");
            if(fichier_log == NULL){
                printf("ERREUR d'ouverture du fichier_log\n");
                exit(EXIT_FAILURE);
            }
            fprintf(fichier_log, "[CLIENT%d]: Infos envoyees au caissier num:%d prix:%d.\n", numero_client, numero_client, prix_achat);
            fclose(fichier_log);
            printf("[CLIENT%d]: Infos envoyees au caissier num:%d prix:%d.\n", numero_client, numero_client, prix_achat);
        }
        else{
            //Reinitialisation de la zone du vendeur et V() sur le mutex
            zone[0] = -1;
            zone[1] = -1;
            zone[2] = -1;
            V(indice_sem_vendeur, 1, id_ens_sem);

            //Le client sort
            //Ecriture dans le fichier de log
            fichier_log = fopen("log.txt", "a");
            if(fichier_log == NULL){
                printf("ERREUR d'ouverture du fichier_log\n");
                exit(EXIT_FAILURE);
            }
            fprintf(fichier_log, "[CLIENT%d]: Pas d'achat je sors du magasin.\n", numero_client);
            fclose(fichier_log);
            printf("[CLIENT%d]: Pas d'achat je sors du magasin.\n", numero_client);
        }
    }

    else{
        if(zone[2] != -1){
            //Lecture de la reponse, reinitialisation de la zone du vendeur et V() sur le mutex
            prix_achat = zone[2];
            zone[0] = -1;
            zone[1] = -1;
            zone[2] = -1;
            V(indice_sem_vendeur, 1, id_ens_sem);

            //Choix du caissier et mise en place des IPCS

            choix_caissier = (rand()+getpid())%nb_caissiers;
            indice_sem_caissier = 1+(nb_vendeurs*2)+(choix_caissier*2);

            //Recuperation du smp du caissier choisi

            cle_zone_caissier = ftok("key.txt", CAI+choix_caissier);
            if(cle_zone_caissier == -1){
                perror("ftok");
                exit(EXIT_FAILURE);
            }
            id_seg_zone_caissier = shmget(cle_zone_caissier, sizeof(int)*2, 0);
            if(id_seg_zone_caissier == -1){
                perror("shmget");
                exit(EXIT_FAILURE);
            }
            zone_caissier = shmat(id_seg_zone_caissier, NULL, 0);

            //Communication avec le caissier

            //Ecriture dans le fichier de log
            fichier_log = fopen("log.txt", "a");
            if(fichier_log == NULL){
                printf("ERREUR d'ouverture du fichier_log\n");
                exit(EXIT_FAILURE);
            }
            fprintf(fichier_log, "[CLIENT%d]: Je pars communiquer avec le caissier.\n", numero_client);
            fclose(fichier_log);
            printf("[CLIENT%d]: Je pars communiquer avec le caissier.\n", numero_client);
            P(indice_sem_caissier, 1, id_ens_sem);

            zone_caissier[0] = numero_client;
            zone_caissier[1] = prix_achat;

            V(indice_sem_caissier+1, 1, id_ens_sem);

            //Ecriture dans le fichier de log
            fichier_log = fopen("log.txt", "a");
            if(fichier_log == NULL){
                printf("ERREUR d'ouverture du fichier_log\n");
                exit(EXIT_FAILURE);
            }
            fprintf(fichier_log, "[CLIENT%d]: Infos envoyees au caissier num:%d prix:%d.\n", numero_client, numero_client, prix_achat);
            fclose(fichier_log);
            printf("[CLIENT%d]: Infos envoyees au caissier num:%d prix:%d.\n", numero_client, numero_client, prix_achat);
        }
        else{
            //Reinitialisation de la zone du vendeur et V() sur le mutex
            zone[0] = -1;
            zone[1] = -1;
            zone[2] = -1;
            V(indice_sem_vendeur, 1,id_ens_sem);

            //Le client sort
            //Ecriture dans le fichier de log
            fichier_log = fopen("log.txt", "a");
            if(fichier_log == NULL){
                printf("ERREUR d'ouverture du fichier_log\n");
                exit(EXIT_FAILURE);
            }
            fprintf(fichier_log, "[CLIENT%d]: Pas d'achat je sors du magasin.\n", numero_client);
            fclose(fichier_log);
            printf("[CLIENT%d]: Pas d'achat je sors du magasin.\n", numero_client);
        }
    }

    //Fin du programme

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[CLIENT%d]: Je meurs bye.\n", numero_client);
    fclose(fichier_log);
    printf("[CLIENT%d]: Je meurs bye.\n", numero_client);
    exit(EXIT_SUCCESS);
}