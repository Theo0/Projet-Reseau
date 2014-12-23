Projet de Reseau de M1 Informatique AIGLE à l'UM2.
=============
Application d'échange de fichier en C++, via TCP, avec utilisation de threads. 


Utilisation
=============
1- Pour compiler, utiliser simplement "make client", et "make serveur". Les classes sockets jointes sont normalement déjà compilées, mais selon votre système il est possible que vous ayez à les recompiler. Utilisez alors "make sock.o" et "make socketdist.o".

2- Pour lancer les application, lancer d'abord le serveur, simplement "./serveur", puis lancer les clients, en leur passant en paramètre l'adresse du serveur, par exemple "./client 127.0.0.1". Le port est fixé.

3- Les dossiers pour stocker les fichiers des clients et du serveur doivent être présent dans le même répertoire que l'application. C'est à dire que le dossier FichiersServeur doit être dans le même répertoire que Serveur, idem pour FichiersClient qui doit être dans le même repertoire que "Client".

4-Pour quitter l'application, fermer d'abord les clients avant de fermer le serveur.
