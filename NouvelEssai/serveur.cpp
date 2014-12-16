#include <iostream>
#include <cstdio>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //struct in_addr et sockaddr_in
#include <netdb.h> // struct hostent, servent
#include <arpa/inet.h> //conversions d'adresses
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>
#include <locale>
#include <sstream>
#include <errno.h>
#include <pwd.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h> //Fonctions: open(), read(), write() and close()
#include <unistd.h>
#include "sockdist.h"
#include "sock.h"

using namespace std;

int descBrCv;

void* thread_reception(void * arg){
	char bufferReception[10000];
	char bufferNomFichier[100];
	int env;
	int socket = *((int*)arg);

	cout << "Attente des informations de reception" << endl;

	int conf = 1;
	int recep = recv(socket, bufferNomFichier, sizeof(bufferNomFichier), 0);
	if(recep == -1){
		cout << "erreur reception nom fichier" << endl;
		pthread_exit((void *)1);
	}
	cout << "Nom du fichier recu : " << bufferNomFichier << endl;
	send(socket, &conf, sizeof(int), 0);

	cout << "on attend de recevoir le fichier " << endl;
	int recep2 = recv(socket, bufferReception, sizeof(bufferReception), 0);
	if(recep2 == -1){
		cout << "Erreur reception du fichier" << endl;
		pthread_exit((void *)1);
	}

	FILE *fichierRecu = fopen(bufferNomFichier, "w+");
	if(fichierRecu == NULL){
		cout << "Erreur lors de la création du fichier" << endl;
		pthread_exit((void *)1);
	}
	fwrite(bufferReception, 1, recep2, fichierRecu);
	cout << "fichier reçu" << endl;
	fclose(fichierRecu);

	pthread_exit(0);
}

void* thread_envoi(void * arg){
	char bufferEnvoi[1000];
	char bufferReception[1000];
	int env;
	int socket = *((int*)arg);

	cout << "Attente du nom du fichier" << endl;
	int recep = recv(socket, bufferReception, sizeof(bufferReception), 0);
	if(recep < 0){
		cout << "Erreur reception nom fichier" << endl;
	}

	char nomfichier[100];
	strcpy(nomfichier, bufferReception);

	FILE *f = fopen(nomfichier, "r");

	if(f == NULL){
		cout << "le fichier n'existe pas" << endl;
		int error = -1;
		env = send(socket, &error, sizeof(int), 0);
		if(env < 0){
			cout << "erreur envoi message derreur" << endl;
 		}
	}
	else{
		int conf = 1;
		env = send(socket, &conf, sizeof(int), 0);
		if(env < 0){
			cout << "erreur envoi message confirmation" << endl;
 		}
		//On transforme le fichier afin de pouvoir l'envoyer via send
	      fseek(f, 0, SEEK_END);
	      long fsize = ftell(f);
	      fseek(f, 0, SEEK_SET);
	      char *string = malloc(fsize + 1);
	      fread(string, fsize, 1, f);
	      string[fsize] = 0;

	      //Envoi du fichier (string)
	      env = send(descBrCv, string, strlen(string), 0);
	      if(env == -1) cout << "Echec de l'envoi du fichier" << endl;
	      else{
	      	cout << "Fichier envoyé correctement" << endl;
	      }
	      //On ferme le fichier
	      fclose(f);
	}

	pthread_exit(0);
}

void* connection_handler(void * arg2){
	char bufferReception[1000];
	int socket = *((int *)arg2);
	int recep =1;

	cout << "Attente de l'ordre du client" << endl;
	recep = recv(socket, bufferReception, sizeof(bufferReception), 0);
	if(recep < 0){
		cout << "Erreur lors de la reception de l'ordre" << endl;
		pthread_exit((void *)1);
	}

	pthread_t thread_id;

	int *arg = (int*)malloc(sizeof(*arg));
	*arg = socket;

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