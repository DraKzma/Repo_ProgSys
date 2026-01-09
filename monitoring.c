#include "config.h"

int main(int argc, char* argv[], char* envp[]){

    //Variables

    FILE* fichier_cle;
    FILE* fichier_mon;
    int cle;
    int id_ens_sem;
    int i;
    int nb_vendeurs;
    int nb_caissiers;
    int num_vendeur = 0;
    int num_caissier = 0;
    int nb_clients;

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

    //Lecture dans le fichier mon.txt

    fichier_mon = fopen("mon.txt", "r");
    if(fichier_mon == NULL){
        printf("ERREUR d'ouverture du fichier_cle\n");
        exit(EXIT_FAILURE);
    }
    if(fscanf(fichier_mon, "%d %d", &nb_vendeurs, &nb_caissiers) != 2){
        printf("ERREUR de lecture dans le fichier_mon\n");
        exit(EXIT_FAILURE);
    }
    fclose(fichier_mon);

    //Affichage de debut

    printf("[MONITORING]: Affichage des valeurs de la simulation.\n");

    //Recuperation des valeurs des semaphores

    while(1){
        sleep(1);
        for(i=1; i<1+(nb_vendeurs*2); i=i+2){
            nb_clients = semctl(id_ens_sem, i, GETNCNT);
            printf("[MONITORING]: %d client attendent au vendeur %d\n", nb_clients, num_vendeur);
            num_vendeur++;
        }
        for(i=1+(nb_vendeurs*2); i<1+(nb_vendeurs*2)+(nb_caissiers*2); i=i+2){
            nb_clients = semctl(id_ens_sem, i, GETNCNT);
            printf("[MONITORING]: %d client attendent au caissier %d\n", nb_clients, num_caissier);
            num_caissier++;
        }
        num_vendeur = 0;
        num_caissier = 0;
    }

    //Fin du programme

    printf("[MONITORING]: Je meurs bye.\n");
    exit(EXIT_SUCCESS);
}