#include "config.h"

/*

Ensemble de semaphores:
Constituee du semaphore du grimoire, des semaphore des vendeurs, et des semaphores des caissiers

indice 0: semaphore pour le grimoire (init a 1)
indice (numero_vendeur*2)+1: semaphore pour la communication avec un client (init a 1)
indice (numero_vendeur*2)+1+1: semaphore pour le mutex de la zone de communication client (init a 0)
indice (nb_vendeurs*2)+1+(numero_caissier*2): semaphore pour la communication avec un client (init a 1)
indice (nb_vendeurs*2)+1+(numero_caissier*2)+1: semaphore pour le mutex de la zone de communication client (init a 0)

*/

/*

Zone de communication du vendeur:
indice 0: numero de rayon (-1 si pas de client present)
indice 1: numero de redirection du vendeur (si besoin)
indice 2: prix de l'achat (si achat)
SI indice 1 et 2 sont a -1 lors de la lecture du client


*/

/*

Zone de communication du caissier:
indice 0: numero du client
indice 1: prix de l'achat

*/

//Fonction usage appelee si mauvais nombre d'arguments
void usage(){
    printf("usage initial:\n");
    printf("./initial <int>nb_clients<int> <int>nb_vendeurs<int> <int>nb_caissiers<int>\n");
}

//Fonction qui gere la reception de SIGUSR1 et SIGUSR2
void Start(){
}

//Fonction qui affiche le grimoire (repertoire qui associe un rayon a son vendeur)
void affiche_grimoire(int* grimoire, int taille_grimoire){
    int i = 0;
    printf("Grimoire: [");
    for(i=0; i<taille_grimoire-1; i++){
        printf("%d, ", grimoire[i]);
    }
    printf("%d]\n", grimoire[i]);
}

//Fonction qui affiche le tableau des cles
void affiche_tab(int tab[], int taille_tab){
    int i = 0;
    printf("Tab: [");
    for(i=0; i<taille_tab-1; i++){
        printf("%d, ", tab[i]);
    }
    printf("%d]\n", tab[i]);
}

//Fonction qui affiche la zone de communication client/vendeur
void affiche_zones(int* tab[], int taille_tab){
    int i = 0;
    printf("Zones: [ ");
    for(i=0; i<taille_tab-1; i++){
        printf("[%d, %d, %d], ", tab[i][0], tab[i][1], tab[i][2]);
    }
    printf("[%d, %d, %d] ]\n", tab[i][0], tab[i][1], tab[i][2]);
}

//Fonction qui affiche l'ensemble de semaphore
void affiche_sem(int id_ens_sem, int nb_vendeurs, int nb_caissiers){
    int i;
    printf("Ens sem: [");
    for(i=0; i<1+(nb_vendeurs*2)+(nb_caissiers*2)-1; i++){
        printf("%d, ", semctl(id_ens_sem, i , GETVAL, NULL));
    }
    printf("%d]\n", semctl(id_ens_sem, i , GETVAL, NULL));
}

int main(int argc, char* argv[], char* envp[]){

    //Variables 

    int nb_clients, nb_vendeurs, nb_caissiers;
    int i;
    int* grimoire;
    int taille_grimoire;
    int retour_semctl = 0;
    pid_t pid;

    struct sigaction sig1;
    struct sigaction sig2;
    FILE * fichier_cle;
    FILE * fichier_log;
    FILE * fichier_mon;
    key_t cle;
    key_t cle_grimoire;
    int id_seg_grimoire;
    int id_ens_sem;
    int tab_smp_vendeurs[NB_MAX];
    int tab_cles_vendeurs[NB_MAX];
    int tab_smp_caissiers[NB_MAX];
    int tab_cles_caissiers[NB_MAX];
    int n_vendeur = SEG;
    int n_caissier = CAI;
    int* zones_communication[NB_MAX]; //Comunnications client/vendeur
    int* zones_caissiers[NB_MAX]; //Comunnications client/caissier

    sigset_t E;

    //Gestion des arguments

    if(argc != 4){
        usage();
        exit(EXIT_FAILURE);
    }
    nb_clients = atoi(argv[1]);
    nb_vendeurs = atoi(argv[2]);
    nb_caissiers = atoi(argv[3]);
    
    //Initialisation de la taille du grimoire

    taille_grimoire = nb_vendeurs;

    //Gestion des masques et sigactions

    sigemptyset(&E);
    sigfillset(&E);
    sigdelset(&E, SIGUSR1);
    sigdelset(&E, SIGUSR2);
    sigprocmask(SIG_BLOCK, &E, NULL);
    sig1.sa_handler = Start;
    sig2.sa_handler = Start;
    sigaction(SIGUSR1, &sig1, NULL);
    sigaction(SIGUSR2, &sig2, NULL);

    //Creation du fichier cle

    fichier_cle = fopen("key.txt", "w");
    if(fichier_cle == NULL){
        printf("ERREUR d'ouverture du fichier_cle\n");
        exit(EXIT_FAILURE);
    }
    fclose(fichier_cle);

    //Creation du fichier de log

    fichier_log = fopen("log.txt", "w");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fclose(fichier_log);

    //Creation des cles

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
    for(i=0; i<nb_vendeurs; i++){
        tab_cles_vendeurs[i] = ftok("key.txt", n_vendeur);
        if(tab_cles_vendeurs[i] == -1){
            perror("ftok");
            exit(EXIT_FAILURE);
        }
        n_vendeur++;
    }
    for(i=0; i<nb_caissiers; i++){
        tab_cles_caissiers[i] = ftok("key.txt", n_caissier);
        if(tab_cles_caissiers[i] == -1){
            perror("ftok");
            exit(EXIT_FAILURE);
        }
        n_caissier++;
    }
    //affiche_tab(tab_cles_vendeurs, nb_vendeurs);
    //affiche_tab(tab_cles_caissiers, nb_caissiers);

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[INIT]: cles cree.\n");
    fclose(fichier_log);
    printf("[INIT]: cles cree.\n");

    //Creation des SMP

    id_seg_grimoire = shmget(cle_grimoire, sizeof(int)*taille_grimoire, IPC_CREAT | IPC_EXCL | 0660);
    if(id_seg_grimoire == -1){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    for(i=0; i<nb_vendeurs; i++){
        tab_smp_vendeurs[i] = shmget(tab_cles_vendeurs[i], sizeof(int)*3, IPC_CREAT | IPC_EXCL | 0660);
        if(tab_smp_vendeurs[i] == -1){
            perror("shmget");
            exit(EXIT_FAILURE);
        }
    }
    for(i=0; i<nb_caissiers; i++){
        tab_smp_caissiers[i] = shmget(tab_cles_caissiers[i], sizeof(int)*2, IPC_CREAT | IPC_EXCL | 0660);
        if(tab_smp_caissiers[i] == -1){
            perror("shmget");
            exit(EXIT_FAILURE);
        }
    }

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[INIT]: Segments de memoire partages cree.\n");
    fclose(fichier_log);
    printf("[INIT]: Segments de memoire partages cree.\n");

    //Creation du SEM

    id_ens_sem = semget(cle, 1+(nb_vendeurs*2)+(nb_caissiers*2), IPC_CREAT | IPC_EXCL | 0660);
    if(id_ens_sem == -1){
        perror("shmget");

        //Destruction des ipcs deja cree si l'on a un probleme de creation
        shmctl(id_seg_grimoire, IPC_RMID, NULL);
        for(i=0; i<nb_vendeurs; i++){
            shmctl(tab_smp_vendeurs[i], IPC_RMID, NULL);
        }
        for(i=0; i<nb_caissiers; i++){
            shmctl(tab_smp_caissiers[i], IPC_RMID, NULL);
        }

        exit(EXIT_FAILURE);
    }

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[INIT]: Semaphores cree.\n");
    fclose(fichier_log);
    printf("[INIT]: Semaphores cree.\n");

    //Initialisation du SEM

    for(i=0; i<1+(nb_vendeurs*2)+(nb_caissiers*2); i++){
        retour_semctl = semctl(id_ens_sem, i, SETVAL, 1); //Les semaphores initialisee a 1
    }
    for(i=2; i<1+(nb_vendeurs*2); i=i+2){
        retour_semctl = semctl(id_ens_sem, i, SETVAL, 0); //Sauf les mutex des vendeurs sont initialisee a 0
    }
    for(i=1+(nb_vendeurs*2)+1; i<1+(nb_vendeurs*2)+(nb_caissiers*2); i=i+2){
        retour_semctl = semctl(id_ens_sem, i, SETVAL, 0); //Sauf les mutex des caissiers sont initialisee a 0
    }
    if(retour_semctl == -1){
        perror("semctl");

        //Destruction des ipcs deja cree l'on a un probleme de creation
        shmctl(id_seg_grimoire, IPC_RMID, NULL);
        for(i=0; i<nb_vendeurs; i++){
            shmctl(tab_smp_vendeurs[i], IPC_RMID, NULL);
        }
        for(i=0; i<nb_caissiers; i++){
            shmctl(tab_smp_caissiers[i], IPC_RMID, NULL);
        }
        for(i=0; i<1+(nb_vendeurs*2)+(nb_caissiers*2); i++){
            semctl(id_ens_sem, i, IPC_RMID, NULL);
        }

        exit(EXIT_FAILURE);
    }
    //affiche_sem(id_ens_sem, nb_vendeurs, nb_caissiers);

    //Attachement des SMP

    grimoire = shmat(id_seg_grimoire, NULL, 0);
    for(i=0; i<nb_vendeurs; i++){
        zones_communication[i] = shmat(tab_smp_vendeurs[i], NULL, 0);
    }
    for(i=0; i<nb_caissiers; i++){
        zones_caissiers[i] = shmat(tab_smp_caissiers[i], NULL, 0);
    }

    //Initialisation du grimoire

    for(i=0; i<taille_grimoire; i++){
        grimoire[i] = -1;
    }

    //Initialisation de la zone de communication des vendeurs

    for(i=0; i<nb_vendeurs; i++){
        zones_communication[i][0] = -1;
        zones_communication[i][1] = -1;
        zones_communication[i][2] = -1;
    }

    //Initialisation de la zone de communication des caissiers

    for(i=0; i<nb_caissiers; i++){
        zones_caissiers[i][0] = -1;
        zones_caissiers[i][1] = -1;
    }

    //Creation des clients

    for(i=0; i<nb_clients; i++){

        //Gestion des arguments

        sprintf(argv[1], "%d", i);
        sprintf(argv[2], "%d", nb_vendeurs);
        sprintf(argv[3], "%d", nb_caissiers);

        //Creation du client

        pid = fork();
        switch (pid){
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
                break;
            case 0:
                //Processus FILS

                execve("client", argv, envp);
                break;
            default:
                //Processus PERE
                
                //Ecriture dans le fichier de log
                fichier_log = fopen("log.txt", "a");
                if(fichier_log == NULL){
                    printf("ERREUR d'ouverture du fichier_log\n");
                    exit(EXIT_FAILURE);
                }
                fprintf(fichier_log, "[INIT]: client numero %d cree.\n", i);
                fclose(fichier_log);
                printf("[INIT]: client numero %d cree.\n", i);
                break;
        }
    }

    //Creation des vendeurs

    for(i=0; i<nb_vendeurs; i++){

        //Gestion des arguments 

        sprintf(argv[1],"%d",i);
        sprintf(argv[2], "%d", taille_grimoire);

        //Creation du vendeur

        pid = fork();
        switch (pid){
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
                break;
            case 0:
                //Processus FILS

                execve("vendeur", argv, envp);
                break;
            default:
                //Processus PERE
                
                //Ecriture dans le fichier de log
                fichier_log = fopen("log.txt", "a");
                if(fichier_log == NULL){
                    printf("ERREUR d'ouverture du fichier_log\n");
                    exit(EXIT_FAILURE);
                }
                fprintf(fichier_log, "[INIT]: vendeur numero %d cree.\n", i);
                fclose(fichier_log);
                printf("[INIT]: vendeur numero %d cree.\n", i);
                break;
        }
    }

    //Creation des caissiers

    for(i=0; i<nb_caissiers; i++){

        //Gestion des arguments 

        sprintf(argv[1],"%d",i);
        sprintf(argv[2],"%d",nb_vendeurs);

        //Creation du caissier

        pid = fork();
        switch (pid){
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
                break;
            case 0:
                //Processus FILS

                execve("caissier", argv, envp);
                break;
            default:
                //Processus PERE
                
                //Ecriture dans le fichier de log
                fichier_log = fopen("log.txt", "a");
                if(fichier_log == NULL){
                    printf("ERREUR d'ouverture du fichier_log\n");
                    exit(EXIT_FAILURE);
                }
                fprintf(fichier_log, "[INIT]: caissier numero %d cree.\n", i);
                fclose(fichier_log);
                printf("[INIT]: caissier numero %d cree.\n", i);
                break;
        }
    }

    //Ecriture dans le fichier de mon
    fichier_mon = fopen("mon.txt", "w");
    if(fichier_mon == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_mon, "%d %d", nb_vendeurs, nb_caissiers);
    fclose(fichier_mon);

    //Attente pour separer les affichages

    sleep(1);

    //Affichage de debut de simulation et sleep (pour laisser le temps aux autres de se setup)

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[INIT]: La simulation va commencer..\n");
    fprintf(fichier_log, "[INIT]: PID pour l'envoie du signal de fin: %d\n", getpid());
    fclose(fichier_log);
    printf("[INIT]: La simulation va commencer..\n");
    printf("[INIT]: PID pour l'envoie du signal de fin: %d\n", getpid());

    //Affichage du grimoire (utile pour savoir si ca marche)

    affiche_grimoire(grimoire, taille_grimoire);
    //affiche_zones(zones_communication, nb_vendeurs);
    
    sleep(2);
    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "\n");
    fclose(fichier_log);
    printf("\n");

    //Envoi du signal de debut a tous les fils
    //kill arrete le programme donc on doit gerer avec un sigaction

    kill(0,SIGUSR1);
    sleep(1);

    //Attente de reception de SIGUSR1 pour arreter la simulation

    sigaction(SIGUSR1, &sig1, NULL);
    sigsuspend(&E);
    sleep(1);
    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "\n");
    fclose(fichier_log);
    printf("\n");

    //Envoi de SIGUSR2 aux vendeurs et caissiers

    kill(0,SIGUSR2);

    //Detachement des SMP

    shmdt(grimoire);
    for(i=0; i<nb_vendeurs; i++){
        shmdt(zones_communication[i]);
    }
    for(i=0; i<nb_caissiers; i++){
        shmdt(zones_caissiers[i]);
    }

    //Destruction des SMP

    shmctl(id_seg_grimoire, IPC_RMID, NULL);
    for(i=0; i<nb_vendeurs; i++){
        shmctl(tab_smp_vendeurs[i], IPC_RMID, NULL);
    }
    for(i=0; i<nb_caissiers; i++){
        shmctl(tab_smp_caissiers[i], IPC_RMID, NULL);
    }

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[INIT]: Segments de memoire partages detruit.\n");
    fclose(fichier_log);
    printf("[INIT]: Segments de memoire partages detruit.\n");

    //Destruction du SEM

    for(i=0; i<1+(nb_vendeurs*2)+(nb_caissiers*2); i++){
        semctl(id_ens_sem, i, IPC_RMID, NULL);
    }

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[INIT]: Semaphores detruit.\n");
    fclose(fichier_log);
    printf("[INIT]: Semaphores detruit.\n");

    //Fin du programme

    sleep(1);

    //Ecriture dans le fichier de log
    fichier_log = fopen("log.txt", "a");
    if(fichier_log == NULL){
        printf("ERREUR d'ouverture du fichier_log\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier_log, "[INIT]: SIGUSR1 recu. Fin de la simulation.\n");
    fclose(fichier_log);
    printf("[INIT]: SIGUSR1 recu. Fin de la simulation.\n");
    exit(EXIT_SUCCESS);
}