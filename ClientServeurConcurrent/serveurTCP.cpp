#include <cstring> 
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
#include <pthread.h>
#include <mutex>

using namespace std;


pthread_t thread_id;


void *connection_handler(void *);

int envoi(int descBrCv){
  char tamponReception[100];
  char tamponEnvoi[100000];

  //Reception du message : nom du fichier
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
    if (fichier == NULL){
      char tamponEnvoi[]="Le fichier n'existe pas";
      cout << "Le fichier demandé par le client n'existe pas" <<  endl;
      int envoi = send(descBrCv, tamponEnvoi, strlen(tamponEnvoi), 0);
      if(envoi == -1) cout << "Echec de l'envoi de la réponse négative" << endl;
      else
      {cout << "Réponse envoyée" << endl;}
     }

    //Sinon on envoi le fichier
    else{
    //On transforme le fichier afin de pouvoir l'envoyer via send
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


int reception(int descBrCv){
  char tamponNomFichier[200];
  char tamponReceptionFichier[100000];
      int reception = recv(descBrCv, tamponNomFichier, sizeof(tamponNomFichier), 0);
      if(!strcmp(tamponNomFichier, "error")){
        if(reception == -1){ cout << "Echec de la reception nom du fichier" << endl;}
        else{
          int reception = recv(descBrCv, tamponReceptionFichier, sizeof(tamponReceptionFichier), 0);
          if(reception == -1){ cout << "Echec de la reception du fichier" << endl;}
          else{
            //Création du fichier de destination
            FILE *fichierRecu = NULL;
            fichierRecu =  fopen (tamponNomFichier, "w+");
            if(fichierRecu == NULL)
               { printf ("Erreur a l'ouverture du fichier\n"); }

            fwrite(tamponReceptionFichier,1,reception, fichierRecu);
            cout << "Fichier reçu" << endl;
            fclose(fichierRecu);
            cout << "\n" << endl;
          }
        }
      }
}


int main(int argc, char *argv[]){
//Préparation BR publique
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

int descBrCv;
//Le serveur accepte un nouveau client (appel système testé)
while( (descBrCv = accept(descBrPub, (struct sockaddr *)&brCv, (socklen_t*)&lgbrCv)) )
{
    puts("Connection accepted");
         int *arg = (int*)malloc(sizeof(*arg));
         *arg = descBrCv;
        if( pthread_create( &thread_id , NULL ,  connection_handler , arg) < 0)
        {
            perror("could not create thread");
            return 1;
        }
}

if(descBrCv == -1) {
  cout << "Echec de l'accept" << endl;}

} 

void *connection_handler(void * descBrCv){
    //création du tampons
  char tamponOrdre[1];
  int sock = *((int*)descBrCv);
  int ordre;


   while(1)
  { 
    int demordre = send(sock, "demande-ordre", sizeof("demande-ordre"), 0);
    if(demordre < 0){cout << "echec demande ordre" << endl;}
    else {cout << "camarche";}
    int recordre = recv(sock, tamponOrdre, sizeof(tamponOrdre),0);
    if(recordre < 0){cout << "echec reception ordre" << endl;}
    /*if(atoi(tamponOrdre) == 0){
       envoi(descBrCv);   
    }
    else if(atoi(tamponOrdre) == 1){
      reception(descBrCv);
    }
    else{
          cout << "Instruction inconnue reçue : " << tamponOrdre << endl;
        }
    memset(tamponOrdre, 0, 10);  */ 
  }

    if(ordre == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(ordre == -1)
    {
        perror("recv failed");
    }
    pthread_exit(0);     
  
  }
