//THEO CHAMBON, GROUPE C, L3 INFO, NUMERO ETUDIANT : 20100514

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
{cout << "Nombre de paramètres incorrect" << endl;
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


char tamponEnvoi[256];
char tamponReception[256];



while(1)
{
cout << "Entrez le nom du fichier souhaité (exit pour quitter) : ";
cin >> tamponEnvoi;
  
//Vérification de la demande d'arrêt ("stop" entré au clavier par l'utlisateur	
  if(!strcmp(tamponEnvoi, "exit"))
    {break;}


int envoi = send(descBrCli, tamponEnvoi, strlen(tamponEnvoi), 0);
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
}
close(descBrCli);
}
