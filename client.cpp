#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sockdist.h"
#include "sock.h"
#include <errno.h>

using namespace std;

int descBrCli;

//CREATION DE LA STRUCTURE PERMETTANT DE PASSER LES INFORMATIONS DU FICHIER AU THREAD, AINSI QUE LE DESCRIPTEUR DE SOCKET
typedef struct {
    int descBr;
    char nom[100];
    long size;
} infoFichier;



void* thread_envoi(void * args){

	//On récupère les arguments de la struct
	char nomFichier[100];
	infoFichier *arg = args;

	strcpy(nomFichier, arg->nom);
	
		//on ouvre un pointeur sur le fichier choisi
      		FILE *fichierenvoi = NULL;
      		fichierenvoi = fopen(nomFichier,  "r");

      		//On vérifie que le fichier existe bien
      		if (fichierenvoi == NULL){
      			cout << "Le fichier n'existe pas" << endl;
      			pthread_exit(1);
      		}

      		//SI ON RECOIT -1, LE SERVEUR NOUS INDIQUE QUE LE NOM DE FICHIER EST DEJA PRIS
      		int fic;
      		int fichierExiste;
      		
      		if((fic = recv(arg->descBr, &fichierExiste, sizeof(int), 0)) == -1){
      			cout << "Erreur reception" << endl;
      			pthread_exit(1);
      		}
      		if(fichierExiste == -1){
      			cout << "Ce nom de fichier existe déjà" << endl;
      			pthread_exit(1);
      		}


      		char buffer[1024];
      		int envoi = 0;

      		//ON LIT LE PREMIER BLOC DU FICHIER
      		int lit = fread(buffer,sizeof(char),sizeof(buffer),fichierenvoi);

      		//SI ERREUR DE LECTURE DU FICHIER
      		if(lit <= 0){
      			cout << "Erreur ouverture fichier" << endl;
      			pthread_exit((void *)1);
      		}

      		//TANT QUON A PAS TOUT ENVOYE
      		while (lit > 0)
			{
			//ON ENVOIE LE BLOC ET ON LIT LE SUIVANT	
			envoi = send(arg->descBr, buffer, lit, 0); 
			lit = fread(buffer,sizeof(char),sizeof(buffer),fichierenvoi);
			}
	      	cout <<"fichier bien envoyé" << endl;
			
	      	fclose(fichierenvoi);
	      	pthread_exit(0);

}

void* thread_reception(void * args){

	//On récupère les arguments de la struct
	char nomFichier[100];
	infoFichier *arg = args;
	strcpy(nomFichier, arg->nom);

	char bufferReception[1024];
				
	//Création du fichier reçu
	FILE *fichier = NULL;
	fichier =  fopen (nomFichier, "w+");
	if(fichier == NULL)
		{ printf ("Erreur a l'ouverture du fichier\n"); 
		pthread_exit(1);}

		long sizeRecu = 0;
		int sizeEcrit = 0;
		int termine = 0;		


		// ON RECOIT LE PREMIER BLOC
		int rec = read(arg->descBr, bufferReception, sizeof(bufferReception));

		//TANT QUON A PAS UN FICHIER DE LA TAILLE FINALE
		while(rec > 0 && termine==0){
			sizeEcrit = fwrite(bufferReception,sizeof(char),rec,fichier);
				sizeRecu = sizeRecu + sizeEcrit;
				if (sizeRecu == (arg->size)){
					termine = 1;
				}
				else{
					rec = read(arg->descBr, bufferReception, sizeof(bufferReception));
				}
		}

	
		cout << "Fichier reçu" << endl;
		fclose(fichier);
		cout << "\n" << endl;
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

		//ON RECUPERE L'ORDRE AU CLAVIER
		char ordre[2];
		cout << "Entrez 0 pour récuperer un fichier du serveur, 1 pour en envoyer un sur le serveur : " << endl;
		cin >> ordre;

		if(int envoi = send(descBrCli, ordre, strlen(ordre), 0) < 0){
			perror("erreur envoi de l'ordre au serveur");
			exit(1);
		}

		cout << "ordre envoyé" << endl;

		//SI ON VEUT RECUPERER UN FICHIER
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
				cout << "Erreur recv"  << endl;
				exit(1);
			}
			if(retour == -1){
				cout << "le fichier demandé n'existe pas" << endl;
			}

			long taille;

			if(recv(descBrCli, &taille, sizeof(long), 0) < 0){
				cout << "Erreur lors de la réception de la taille du fichier" << endl;
			}

			cout << "Entrer le nom sous lequel vous souhaitez sauvegarder le fichier : " ;
					char nomfichier[100];
					cin >> nomfichier;


			if(retour > 0){
			char nomfichierComplet[100];
			strcpy(nomfichierComplet, "FichiersClient/" );
			strcat(nomfichierComplet, nomfichier);	

			infoFichier *args = malloc(sizeof *args);
        	args->descBr = descBrCli;
        	args->size = taille;
        	strcpy(args->nom, nomfichierComplet);

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

			char nomfichier[100];
			strcpy(nomfichier, "FichiersClient/" );
			strcat(nomfichier, nomFichierLocal);

			//on ouvre un pointeur sur le fichier choisi
      		FILE *fichierenvoi = NULL;
      		fichierenvoi = fopen(nomfichier,  "r");
      		//On vérifie que le fichier existe bien
      		if (fichierenvoi == NULL){
      			cout << "Le fichier n'existe pas" << endl;
      			exit(1);
      		}
      		//On récupère le taille du fichier
      		struct stat statFile;
    		stat(nomfichier, &statFile);
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
			long taille = statFile.st_size;
			envoi = send(descBrCli, &taille, sizeof(long), 0);{
				if(envoi == -1){
					cout << "Erreur envoi taille fichier " << strerror(errno) <<endl;
					exit(1);
				}
			}

			infoFichier *args = malloc(sizeof *args);
        	args->descBr = descBrCli;
        	args->size = &statFile.st_size;
        	strcpy(args->nom, nomfichier);

        	if( pthread_create( &thread_id , NULL ,  thread_envoi , args) < 0)
        	{
        	free(args);	
            perror("Erreur création thread envoi");
            exit(1);
        	}

        	
		}

	}

	close(descBrCli);
}