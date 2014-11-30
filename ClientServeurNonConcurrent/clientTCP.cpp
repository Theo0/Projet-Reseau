//THEO CHAMBON, GROUPE C, L3 INFO, NUMERO ETUDIANT : 20100514
#include <cstring> 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include "sockdist.h"
#include "sock.h"


using namespace std;

int main(int argc, char *argv[]){

if(argc != 2)
{cout << "Nombre de paramètres incorrect, il manque l'adresse du serveur" << endl;
exit(1);}

//Creation de la boite de récéption 
Sock brCli(SOCK_STREAM, 0);
int descBrCli;

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


char tamponEnvoi[100];
char tamponOrdre[20];
char tamponChemin[200];
char tamponEnvoiFichier[100000];
char tamponReception[100000];



	while(1){
		cout << "Entrez reception pour recevoir un fichier, envoi pour en déposer un, exit pour quitter : " ;
		cin >> tamponOrdre;

		//Vérification de la demande d'arrêt ("stop" entré au clavier par l'utlisateur)
		if(!strcmp(tamponOrdre, "exit"))
		    {exit(0);}

		//On envoi l'ordre au serveur
		int envoi = send(descBrCli, tamponOrdre, strlen(tamponOrdre), 0);
		if(envoi == -1){ cout << "Echec de l'envoi de la commande" << endl; exit(1);}

		if(!strcmp(tamponOrdre, "reception")){
			cout << "On rente dans recep" << endl;
			//RECEPTION DE FICHIER PAR LE CLIENT *************************************************************
			cout << "Entrez le nom du fichier souhaité (exit pour quitter) : ";
			cin >> tamponEnvoi;
			envoi = send(descBrCli, tamponEnvoi, strlen(tamponEnvoi), 0);
			if(envoi == -1){ cout << "Echec de l'envoi" << endl; exit(1);}
			else {cout << "Demande envoyée" << endl;}


			int reception = recv(descBrCli, tamponReception, sizeof(tamponReception), 0);
			if(reception == -1){ cout << "Echec de la reception" << endl; exit(1);}

			//On vérifie la réponse du serveur
			string part(tamponReception);
			string reponse(part.substr(0, reception));
			if(reponse == "Le fichier n'existe pas")
			{
				cout << "le fichier demandé n'existe pas" << endl;
			}
			else{

				cout << "Entrer le nom sous lequel vous souhaitez sauvegarder le fichier : " ;
				char nomfichier[100];
				cin >> nomfichier;

				//Création du fichier reçu
				FILE *fichier = NULL;
				fichier =  fopen (nomfichier, "w+");
				if(fichier == NULL)
				       { printf ("Erreur a l'ouverture du fichier\n"); }


				//Remplissage du fichier reçu
				fwrite(tamponReception,1,reception, fichier);
				cout << "Fichier reçu" << endl;
				fclose(fichier);
				cout << "\n" << endl;
			}
			//FIN RECEPTION DE FICHIER PAR LE CLIENT *********************************************************
		}
		else if(!strcmp(tamponOrdre, "envoi")){
			//ENVOI DE FICHIER DU CLIENT VERS LE SERVEUR ******************************************************

			//L'utilisateur entre le chemin du fichier qu'il souahite envoyer
			cout << "Entrez le chemin du fichier que vous souhaitez envoyer : ";
			cin >> tamponChemin;

			//On transforme le chemin saisi au clavier en string
			string part(tamponChemin);
      		string file(part.substr(0, sizeof(tamponChemin)));

      		//on ouvre un pointeur sur le fichier choisi
      		FILE *fichierenvoi = NULL;
      		fichierenvoi = fopen(file.c_str(),  "r");
      		//On vérifie que le fichier existe bien
      		if (fichierenvoi == NULL){
      			cout << "Le fichier n'existe pas" << endl;
      			envoi = send(descBrCli, "error", 5, 0);
      		}
      		else{
      			//On envoi au serveur le nom sous lequel sauvegarder le fichier
      			cout << "Choisissez le nom sous lequel vous souhaitez enregistrer le fichier sur le serveur : ";
      			char nomfichier[100];
				cin >> nomfichier;
				envoi = send(descBrCli, nomfichier, strlen(nomfichier), 0);
				if(envoi == -1){
					cout << "Erreur lors de l'envoi de la demande au serveur" << endl;
				}
				else{
					//On transforme le fichier afin de pouvoir l'envoyer via send
	      			fseek(fichierenvoi, 0, SEEK_END);
	              	long fsize = ftell(fichierenvoi);
	              	fseek(fichierenvoi, 0, SEEK_SET);
	             	char *string = malloc(fsize + 1);
	      			fread(string, fsize, 1, fichierenvoi);
	      			string[fsize] = 0;

	      			//On vérifie que l'envoi s'est bien passé
	      			envoi = send(descBrCli, string, strlen(string), 0);
	      			if(envoi == -1) cout << "Echec de l'envoi du fichier" << endl;
	      			else {cout << "Fichier envoyé correctement" << endl; }
	      			//On ferme le fichier
	      			
				}
      		fclose(fichierenvoi);    
      		}
			//FIN ENVOI DE FICHIER DU CLIENT VERS LE SERVEUR **************************************************
			
		}
			else{
			cout << "Demande non reconnue" << endl;
			}
	}
close(descBrCli);
}
