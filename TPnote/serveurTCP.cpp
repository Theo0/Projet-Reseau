//THEO CHAMBON, GROUPE C, L3 INFO, NUMERO ETUDIANT : 20100514

#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include "sockdist.h"
#include "sock.h"
#include <pwd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char *argv[]){

//Préparation BR publique
Sock brPub(SOCK_STREAM, (short)(10514),0);
int descBrPub;

//Verification du bind
if(brPub.good()) descBrPub = brPub.getsDesc();


int ecoute = listen(descBrPub, 5);
//Verification de l'écoute
if(ecoute == -1) cout << "Echec de listen" << endl;


struct sockaddr_in brCv;
socklen_t lgbrCv = sizeof(struct sockaddr_in);


while(1)
  {
  //Le serveur accepte un nouveau client (appel système testé)
  int descBrCv = accept(descBrPub, (struct sockaddr *)&brCv, &lgbrCv);

  if(descBrCv == -1) 
  {cout << "Echec de l'accept" << endl;}
  //Si le accept a fonctionné, le serveur concurrent fork
  else
    {
    //Si on est dans le fils
    if(fork()==0)
      {
      char tamponReception[2560000];
      char tamponEnvoi[2560000];

	//getsockname permet de récuperer l'IP et le port du client
      if(getsockname(descBrPub, (struct sockaddr *)&brCv, &lgbrCv) == -1)
	{cout << "Erreur getsockname" << endl;
	exit(1);
	}

	//Affichage de l'IP et du port du client connecté
	printf("Adresse IP du client : %s\n", inet_ntoa(brCv.sin_addr));
 	printf("Port utilisé : %d\n", (int) ntohs(brCv.sin_port));

	//Reception du message
      int reception = recv(descBrCv, tamponReception, sizeof(tamponReception), 0);
      if(reception == -1) cout << "Echec de la reception" << endl;

	//Recupération du message envoyé qui est transformé en string pour avoir le nom du fichier souhaité
      string part(tamponReception);
      string file(part.substr(0, reception));
      cout << "Nom du fichier :" << file << endl;

	//On essaie d'ouvrir le fichier souhaité par le client
      FILE *fichier = NULL;
      fichier = fopen(file.c_str(),  "r");
      
 	
	//Si fopen a échoué, le fichier n'existe pas, on envoi une réponse négative au client
      if (fichier == NULL)
         {char tamponEnvoi[]="Le fichier n'existe pas";
	  cout << "Le fichier demandé par le client n'existe pas" <<  endl;
	  int envoi = send(descBrCv, tamponEnvoi, strlen(tamponEnvoi), 0);
	  if(envoi == -1) cout << "Echec de l'envoi de la réponse négative" << endl;
	  else
	  {cout << "Réponse envoyée" << endl;}	
	 }

	//Sinon on envoi le fichier
      else
	 {
	//On transforme le fichier en char[] afin de pouvoir l'envoyer via send
	fseek(fichier, 0, SEEK_END);
        long fsize = ftell(fichier);
        fseek(fichier, 0, SEEK_SET);
       	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, fichier);
	string[fsize] = 0;
	//Envoi du fichier (string)
	int envoi = send(descBrCv, string, strlen(string), 0);
        if(envoi == -1) cout << "Echec de l'envoi du fichier" << endl;
	else
	    {cout << "Fichier envoyé correctement" << endl; }
	//On ferme le fichier
	fclose(fichier);
	
	}
      }
    }
  }
}
