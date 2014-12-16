#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>
#include <cstdio>
#include <vector>
#include "sockdist.h"
#include "sock.h"

using namespace std;

int descBrCli;

typedef struct {
    //Or whatever information that you need
    int descBr;
    char nom[100];
} infoFichier;



void* thread_envoi(void * args){

	cout << "on est dans le nouveau thread" << endl;
	//On récupère les arguments de la struct
	char nomFichier[100];
	infoFichier *arg = args;

	strcpy(nomFichier, arg->nom);
	cout << nomFichier << endl;

	//on ouvre un pointeur sur le fichier choisi
      		FILE *fichierenvoi = NULL;
      		fichierenvoi = fopen(nomFichier,  "r");
      		//On vérifie que le fichier existe bien
      		if (fichierenvoi == NULL){
      			cout << "Le fichier n'existe pas" << endl;
      			exit(1);
      			pthread_exit((void *)1);
      		}
      		//On transforme le fichier afin de pouvoir l'envoyer via send
	      	fseek(fichierenvoi, 0, SEEK_END);
	        long fsize = ftell(fichierenvoi);
	        fseek(fichierenvoi, 0, SEEK_SET);
	        char *string = malloc(fsize + 1);
	      	fread(string, fsize, 1, fichierenvoi);
	      	string[fsize] = 0;

			int envoi = send(arg->descBr, string, strlen(string), 0);
	      	if(envoi == -1){
	      		cout << "echec lors de l'envoi du fichier" << endl;
	      		cout << "ERREUR : " << strerror(errno) << endl;
	      		exit(1);
	      	}
	      	cout <<"fichier bien envoyé" << endl;

	      	
			
	      	fclose(fichierenvoi);
	      	pthread_exit(0);

}

void* thread_reception(void * args){

	cout << "on est dans le nouveau thread" << endl;
	//On récupère les arguments de la struct
	char nomFichier[100];
	infoFichier *arg = args;
	strcpy(nomFichier, arg->nom);

	char tamponReception[100000];
				int reception = recv(arg->descBr, tamponReception, sizeof(tamponReception), 0);
				if(reception < 0){
					cout << "erreur reception fichier" << endl;	
					exit(1);
				}
				else{
					

					//Création du fichier reçu
					FILE *fichier = NULL;
					fichier =  fopen (nomFichier, "w+");
					if(fichier == NULL)
					       { printf ("Erreur a l'ouverture du fichier\n"); }


					//Remplissage du fichier reçu
					fwrite(tamponReception,1,reception, fichier);
					cout << "Fichier reçu" << endl;
					fclose(fichier);
					cout << "\n" << endl;
				}
				pthread_exit(0);


}

int main(int argc, char* argv[]){

	if(argc != 2){
	cout << "Nombre de paramètres incorrect, il manque l'adresse du serveur" << endl;
	exit(1);
	}

	

	while(1){
		//Creation de la boite de récéption 
		Sock brCli(SOCK_STREAM, 0);
		pthread_t thread_id;
		//Test de la boite de récéption et récupération du descripteur
		if(brCli.good())
		  {descBrCli = brCli.getsDesc();}
		else
		  {cout << "Erreur boite reception client" << endl; exit(1);}

		//Création de la BR publique
		SockDist brPub(argv[1], short(10514));
		struct sockaddr_in * adrBrPub = brPub.getAdrDist();
		int lgAdrBrPub = sizeof(struct sockaddr_in);	

		//Connexion au serveur
		int connexion = connect(descBrCli, (struct sockaddr *)adrBrPub, lgAdrBrPub);
		if(connexion == -1) {cout << "Echec de la connexion" << endl; exit(1);}

		char ordre[2];
		cout << "Entrez 0 pour récuperer un fichier du serveur, 1 pour en envoyer un sur le serveur : " << endl;
		cin >> ordre;

		if(int envoi = send(descBrCli, ordre, strlen(ordre), 0) < 0){
			perror("erreur envoi de l'ordre au serveur");
			exit(1);
		}

		cout << "ordre envoyé" << endl;

		if(ordre[0] == '0'){
			char nomFichierReception[100];
			cout << "Entrez le nom du fichier que vous voulez : " << endl;
			cin >> nomFichierReception;


			if(send(descBrCli, nomFichierReception, sizeof(nomFichierReception), 0) <0){
				cout << "echec demande fichier" << endl;
				exit(1);
			}

			int retour = 0;

			if(recv(descBrCli, &retour, sizeof(int), 0) <0){
				cout << "le serveur à rencontré un problème avec le fichier"  << endl;
				exit(1);
			}

			cout << "Entrer le nom sous lequel vous souhaitez sauvegarder le fichier : " ;
					char nomfichier[100];
					cin >> nomfichier;


			if(retour > 0){
			infoFichier *args = malloc(sizeof *args);
        	args->descBr = descBrCli;
        	strcpy(args->nom, nomfichier);

        	if( pthread_create( &thread_id , NULL ,  thread_reception , args) < 0)
        	{
        	free(args);	
            perror("Erreur création thread reception");
            exit(1);
        	}

			}



		}
		else if(ordre[0] == '1'){
			char nomFichierEnvoi[100];
			char nomFichierLocal[100];

			int retour = 0;

			cout << "Entrez le nom du fichier à envoyer : " << endl;
			cin >> nomFichierLocal;

			//on ouvre un pointeur sur le fichier choisi
      		FILE *fichierenvoi = NULL;
      		fichierenvoi = fopen(nomFichierLocal,  "r");
      		//On vérifie que le fichier existe bien
      		if (fichierenvoi == NULL){
      			cout << "Le fichier n'existe pas" << endl;
      			exit(1);
      		}

      		close(fichierenvoi);

      		cout << "Entrez le nom sous lequel l'enregistrer  : " << endl;
			cin >> nomFichierEnvoi;
			int envoi = send(descBrCli, nomFichierEnvoi, strlen(nomFichierEnvoi), 0);
			if(envoi < 0){
				cout << "erreur lors de l'envoi du nom du fichier";
				exit(1);
			}

			retour = 0;
			if(recv(descBrCli, &retour, sizeof(int), 0) <0){
				cout << "le serveur à rencontré un problème avec le fichier"  << endl;
				exit(1);
			}

			infoFichier *args = malloc(sizeof *args);
        	args->descBr = descBrCli;
        	strcpy(args->nom, nomFichierLocal);

        	if( pthread_create( &thread_id , NULL ,  thread_envoi , args) < 0)
        	{
        	free(args);	
            perror("Erreur création thread reception");
            exit(1);
        	}

        	
		}

	}

	close(descBrCli);
}