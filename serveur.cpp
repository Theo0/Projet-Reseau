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
#include <list>

using namespace std;

int descBrCv;

//Liste des fichiers occupés en écriture
list<string> list_ocp; 

void* thread_reception(void * arg){
	//Création des buffers pour recevoir le fichier et pour recevoir son nom
	char bufferReception[1024];
	char bufferNomFichier[100];
	//Récupération du descripteur de boite
	int socket = *((int*)arg);

	//Pour stocker la taille du fichier à recevoir
	long taille;

	cout << "Attente des informations de reception" << endl;


	//Reception du nom du fichier
	int recep = recv(socket, bufferNomFichier, sizeof(bufferNomFichier), 0);
	if(recep == -1){
		cout << "erreur reception nom fichier" << endl;
		pthread_exit(1);
	}
	if(bufferNomFichier[0] == '-1'){
		pthread_exit(1);
	}
	//Si on a reçu un buffer vide, on interromp
	if(bufferNomFichier[0] == '\0'){
		pthread_exit(1);
	}
	cout << "Nom du fichier recu : " << bufferNomFichier << endl;

	//Envoi de la confirmation de récéption du nom du fichier
	int conf = 1;
	send(socket, &conf, sizeof(int), 0);

	//Récéption de la taille du fichier
	recep = recv(socket, &taille, sizeof(long), 0);
	if(recep == -1){
		cout << "erreur reception taille fichier" << endl;
		pthread_exit(1);
	}

	cout << "on attend de recevoir le fichier " << endl;


	char nomFichier[160];
	strcpy(nomFichier, "FichiersServeur/" );
	strcat(nomFichier, bufferNomFichier);

	struct stat statFile;
	stat(nomFichier, &statFile);
	long sizevide = statFile.st_size;

	if(sizevide != 0){
		cout << "Ce fichier existe déjà, veuillez choisir un autre nom" << endl;
		int fichierExiste = -1;
		int envoiErreur = send(socket, &fichierExiste, sizeof(int), 0);
		if(envoiErreur  == -1){
			cout << "Erreur send" << endl;
			pthread_exit(1);
		}
		pthread_exit(1);
	}
	else{
		int fichierExiste = 10;
		int envoiErreur = send(socket, &fichierExiste, sizeof(int), 0);
		if(envoiErreur  == -1){
			cout << "Erreur send" << endl;
			pthread_exit(1);
		}
	}


	//On crée le fichier locale ou on va enregistrer les données reçues
	FILE *fichierRecu = fopen(nomFichier, "w+");
	if(fichierRecu == NULL){
		cout << "Erreur lors de la création du fichier" << endl;
		pthread_exit(1);
	}

	//on ajoute le fichier sur la liste des fichiers occupés
	list_ocp.push_back(nomFichier);


	//RECEPTION DU FICHIER
	long sizeRecu = 0;
	int sizeEcrit = 0;
	bool termine = false;

	//ON RECOIT LE PREMIER BLOC DU FICHIER
	int recept = recv(socket, bufferReception, sizeof(bufferReception), 0);

	//TANT QUON A PAS TOUT RECU
	while(recept > 0 && !termine){
		//ON ECRIT LE BLOC RECU DANS LE FICHIER LOCAL
		sizeEcrit = fwrite(bufferReception,sizeof(char),recept,fichierRecu);

		//ON CALCULE LE NOMBRE D'OCTETS RECUS
		sizeRecu = sizeRecu + sizeEcrit;
		//SI ON A RECU TOUT LE FICHIER, ON A TERMINE
		if (sizeRecu == taille){
			termine = true;
		}
		//SINON ON CONTINUE A RECEVOIR LES DONNES
		else{
		recept = recv(socket, bufferReception, sizeof(bufferReception), 0);
		}
	}
	cout << "fichier reçu" << endl;
	fclose(fichierRecu);
	//on supprime le fichier de la liste des fichiers occupés en ecriture
	list_ocp.remove(nomFichier);

	pthread_exit(0);
}

void* thread_envoi(void * arg){
	//ON CREE LES BUFFERS ET ON RECUPERE LE DESCRIPTEUR
	char bufferReception[1000];
	int env;
	int socket = *((int*)arg);

	cout << "Attente du nom du fichier" << endl;
	//RECEPTION DU NOM DU FICHIER
	int recep = recv(socket, bufferReception, sizeof(bufferReception), 0);
	if(recep < 0){
		cout << "Erreur reception nom fichier" << endl;
		pthread_exit(1);
	}

	char nomfichier[100];
	strcpy(nomfichier, "FichiersServeur/" );
	strcat(nomfichier, bufferReception);
	//ON OUVRE LE FICHIER A ENVOYER
	FILE *f = fopen(nomfichier, "r");

	//test si le fichier est dans la liste des fichiers occupés
	bool occupe=false;
	list<string>::iterator list_iter;
	for(list_iter = list_ocp.begin(); list_iter != list_ocp.end(); list_iter++)
	{
		if (*list_iter==nomfichier)
		{
			occupe=true;
		}
	}

	//SI LE FICHIER N'EXISTE PAS, ON RETOURNE UNE ERREUR AU CLIENT
	if(f == NULL){
		cout << "le fichier n'existe pas" << endl;
		int error = -1;
		env = send(socket, &error, sizeof(int), 0);
		if(env < 0){
			cout << "erreur envoi message derreur" << endl;
 		}
 		pthread_exit(1);
	}
	else if (occupe){
		cout << "le fichier demandé est en cours de depot." << endl;
		int error = -2;
		env = send(socket, &error, sizeof(int), 0);
		if(env < 0){
			cout << "erreur envoi message derreur fichier en cours de dépot" << endl;
 		}
 		pthread_exit(1);
	}
	//SINON ON ENVOIE LA CONFIRMATION AU CLIENT
	else{
		int conf = 1;
		env = send(socket, &conf, sizeof(int), 0);
		if(env < 0){
			cout << "erreur envoi message confirmation" << endl;
			pthread_exit(1);
 		}

 		//CALCULE DE LA TAILLE DU FICHIER
 		struct stat statFile;
		stat(nomfichier, &statFile);
		long taille = statFile.st_size;

		//ON ENVOIE LA TAILLE DU FICHIER AU CLIENT
 		if((env = send(socket, &taille, sizeof(long), 0))<0){
 			cout << "Erreur envoi taille" << endl;
 			pthread_exit(1);
 		}


		char buffer[1024];
      	int sizeEnvoye = 0;
      	bool recu = false;

      	//ON RECOIT LE PREMIER BLOC
      	int rec = fread(buffer,sizeof(char),sizeof(buffer),f);

      		//SI ERREUR LORS DE LA LECTURE DU FICHIER
      		if(rec <= 0){
      			cout << "Erreur ouverture fichier" << endl;
      			pthread_exit(1);
      		}

      		//TANT QUON A PAS TOUT ENVORE
      		while (rec > 0 && !recu)
			{
			//ON ENVOI LE BUFFER ET ON AVANCE DANS LE FICHIER
			sizeEnvoye = send(socket, buffer, rec, 0); 
			rec = fread(buffer,sizeof(char),sizeof(buffer),f);
			}
			
	      	cout <<"fichier bien envoyé" << endl;
	      //On ferme le fichier
	      fclose(f);
	}

	pthread_exit(0);
}

void* connection_handler(void * arg2){
	char bufferReception[1000];
	int socket = *((int *)arg2);
	int recep =1;

	//ON ATTEND LA COMMANDE DU CLIENT
	cout << "Attente de l'ordre du client" << endl;
	recep = recv(socket, bufferReception, sizeof(bufferReception), 0);
	if(recep < 0){
		cout << "Erreur lors de la reception de l'ordre" << endl;
		pthread_exit(1);
	}

	pthread_t thread_id;

	int *arg = (int*)malloc(sizeof(*arg));
	*arg = socket;

	//ON CREE LE THREAD CORRESPONDANT A LA COMMANDE DU CLIENT
	if(bufferReception[0] == '1'){
		if( pthread_create( &thread_id , NULL ,  thread_reception , arg) < 0)
        {
            perror("Erreur création thread reception");
        }
	} else if(bufferReception[0] == '0'){
		if( pthread_create( &thread_id , NULL ,  thread_envoi , arg) < 0)
        {
            perror("Erreur création thread envoi");
        }
	}
	else{
		cout << "ordre non compris ou client déconnecté" << endl;
 	}

 	pthread_exit(0);

}

int main(int argc, char **arv){

	cout<<"Lancement du serveur......."<<endl;
	cout<<"===========================\n"<<endl;

	Sock brPub(SOCK_STREAM, (short)(10514),0);
	int descBrPub;

	//Verification du bind
	if(brPub.good()){
  	descBrPub = brPub.getsDesc();
	}

	int ecoute = listen(descBrPub, 5);
	//Verification de l'écoute
	if(ecoute == -1) cout << "Echec de listen" << endl;

	struct sockaddr_in brCv;
	socklen_t lgbrCv = sizeof(struct sockaddr_in);

	pthread_t thread_id;

	while( (descBrCv = accept(descBrPub, (struct sockaddr *)&brCv, (socklen_t*)&lgbrCv)) )
	{
   		 cout << "Client accepté" << endl;
         int *arg = (int*)malloc(sizeof(*arg));
         *arg = descBrCv;
        if( pthread_create( &thread_id , NULL ,  connection_handler , arg) < 0)
        {
            perror("Erreur création thread");
        }
	}	


	close(descBrPub);
}