/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

char usernameGlobal[100], passwordGlobal[100];
int isRestrictedGlobal = 0, typeGlobal = 0;

/* portul de conectare la server*/
int port;

int login(int sd)
{

  char username[100] = "", password[100] = "";

  printf("Introduceti username:\n");
  fgets(username, sizeof(username), stdin);

  printf("Introduceti password:\n");
  fgets(password, sizeof(password), stdin);

  if (strcmp(username, "") == 0 || strcmp(password, "") == 0)
  {
    printf("Credentialele nu pot fi nule\n");
  }

  char loginRequest[200] = "";
  char loginResponse[200] = "";

  strcpy(loginRequest, username);
  strcat(loginRequest, ";");
  strcat(loginRequest, password);

  int lungimeRequest = strlen(loginRequest);
  int lungimeResponse = 0;

  int operatie = 0;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* trimiterea mesajului la server */
  if (write(sd, &lungimeRequest, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, loginRequest, lungimeRequest) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, loginResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  // tokenizare login response (de la server primim inapoi isRestricted;type)
  // ne asteptam sa primim un raspuns de tipul 0;1 unde prima cifra reprezinta daca
  // e sau nu restrictionat, respectiv a doua cifra tipul de utilizator

  printf("\n%s\n", loginResponse);

  int lungimeRaspunsCheck = strlen(loginResponse);

  if (lungimeRaspunsCheck == 3)
  {

    char *sirTokens;
    char delim[] = ";";

    sirTokens = strtok(loginResponse, delim);

    isRestrictedGlobal = atoi(sirTokens);

    if (sirTokens != NULL)
    {
      sirTokens = strtok(NULL, delim);
    }

    typeGlobal = atoi(sirTokens);

    printf("Login ok!\n");
    strcpy(usernameGlobal, username);
    strcpy(passwordGlobal, password);

    return 1;
  }
  else
  {
    printf("Invalid credentials!\n");
    return 0;
  }
}

int signup(int sd)
{

  char nume[100] = "", username[100] = "", password[100] = "";
  int tip = 0;
  getchar();
  printf("Introduceti nume:\n");
  fgets(nume, sizeof(nume), stdin);

  printf("Introduceti username:\n");
  fgets(username, sizeof(username), stdin);

  printf("Introduceti password:\n");
  fgets(password, sizeof(password), stdin);

  // se poate crea doar utilizator normal

  if (strcmp(nume, "") == 0 || strcmp(username, "") == 0 || strcmp(password, "") == 0)
  {
    printf("Credentialele nu pot fi nule\n");
  }

  char signupRequest[200] = "";
  char signupResponse[200] = "";

  strcpy(signupRequest, nume);
  strcat(signupRequest, ";");
  strcat(signupRequest, username);
  strcat(signupRequest, ";");
  strcat(signupRequest, password);

  int lungimeRequest = strlen(signupRequest);
  int lungimeResponse = 0;

  int operatie = 1;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* trimiterea mesajului la server */
  if (write(sd, &lungimeRequest, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, signupRequest, lungimeRequest) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, &tip, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, signupResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  if (strcmp(signupResponse, "Signup successfully!") == 0)
  {
    return 1;
  }
  else
  {
    printf("Username already taken! Please chose a different one!");
    return 0;
  }
}

int adaugaMelodie(int sd)
{

  char nume[100] = "", descriere[500] = "", genuri[1000] = "", link[200] = "";
  int nrVoturi;
  getchar();
  printf("Introduceti nume:\n");
  fgets(nume, sizeof(nume), stdin);
  printf("Introduceti descriere:\n");
  fgets(descriere, sizeof(descriere), stdin);
  printf("Introduceti link:\n");
  fgets(link, sizeof(link), stdin);

  printf("Introduceti genurile muzicale cu spatiu intre ele:\n");

  fgets(genuri, sizeof(genuri), stdin);

  char *sirTokens;
  char delim[] = " ";

  int lungimeGenuri = strlen(genuri);

  char addSongResponse[200] = "";
  int lungimeResponse = 0;

  int lungimeNume = strlen(nume);
  int lungimeDescriere = strlen(descriere);
  int lungimeLink = strlen(link);

  int operatie = 2;

  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* trimiterea mesajului la server */
  if (write(sd, &lungimeNume, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, nume, lungimeNume) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, &lungimeDescriere, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, descriere, lungimeDescriere) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, &lungimeLink, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, link, lungimeLink) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, &lungimeGenuri, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, genuri, lungimeGenuri) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, addSongResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  // /* afisam mesajul primit */
  printf("Mesajul primit este: %s\n", addSongResponse);

  if (strcmp(addSongResponse, "Song added!") == 0)
  {
    printf("Song added successfully!\n");
    return 1;
  }
  else
  {
    printf("Song already exists!\n");
    return 0;
  }
}

void showUsers(int sd)
{

  int lungimeResponse = 0;
  char usersResponse[5000] = "";

  int operatie = 9;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  // trimitem username-ul sa poata verifica pe server ca intr-adevar e administrator
  int lungimeUsername = strlen(usernameGlobal);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeUsername, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, usernameGlobal, lungimeUsername) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, usersResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  printf("%s\n", usersResponse);
}

void showTop(int sd)
{

  // aici trimitem pe server doar comanda
  // serverul va returna un string (continand raspunsul pe mai multe randuri) ce va fi afisat

  int lungimeResponse = 0;
  char topResponse[5000] = "";

  int operatie = 3;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, topResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  printf("%s\n", topResponse);
}

void showTopForGenre(int sd)
{

  int lungimeResponse = 0;
  char topResponse[5000] = "";

  char genMuzical[1000] = "";
  printf("Introduceti genul muzical pentru care doriti realizarea topului\n");
  getchar();
  fgets(genMuzical, sizeof(genMuzical), stdin);

  // aici trimitem pe server comanda alaturi de genul muzical dorit
  // serverul va returna un string (continand raspunsul pe mai multe randuri) ce va fi afisat

  int operatie = 4;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeGenMuzical = strlen(genMuzical);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeGenMuzical, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, genMuzical, lungimeGenMuzical) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, topResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  // daca genul introdus nu exista, serverul va returna mesajul "Genre not existing!"

  if (strcmp(topResponse, "Genre not existing!") == 0)
  {
    printf("Genul muzical introdus nu exista!\n");
  }
  else
  {
    printf("%s\n", topResponse);
  }
}

int voteSong(int sd)
{

  // fluxul logic este:
  //	- utilizatorul afiseaza topul si il vizualizeaza
  //	- dupa aceea, el hotaraste sa voteze o melodie
  //	- se verifica daca are drept de vot
  //	- va introduce numele melodiei pe care doreste sa o voteze

  // verificam daca utilizatorul are drept de vot

  int lungimeResponse = 0;
  char voteResponse[100] = "";

  if (isRestrictedGlobal == 1)
  {
    printf("User vote right is restricted!\n");
    return 0;
  }

  // vizualizare top
  showTop(sd);

  printf("\n");

  char numeMelodie[100] = "";
  printf("Introduceti numele melodiei\n");
  getchar();
  fgets(numeMelodie, sizeof(numeMelodie), stdin);

  int operatie = 5;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeMelodie = strlen(numeMelodie);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeMelodie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, numeMelodie, lungimeMelodie) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, voteResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  // verificam daca a fost totul ok

  if (strcmp(voteResponse, "Song voted!") == 0)
  {
    printf("Song voted successfully!\n");
    return 1;
  }
  else
  {
    printf("Please insert an existing song!\n");
    return 0;
  }
}

int addComment(int sd)
{

  // fluxul logic este:
  //	- utilizatorul afiseaza topul si il vizualizeaza
  //	- dupa aceea, el hotaraste sa comenteze la o anumita melodie
  //	- va introduce id-ul melodiei la care doreste sa adauge un comentariu

  int lungimeResponse = 0;
  char commentResponse[100] = "";

  // vizualizare top cu piese
  showTop(sd);

  char numeMelodie[1000] = "";
  char comentariu[1000] = "";
  getchar();
  printf("Introduceti numele melodiei\n");
  fgets(numeMelodie, sizeof(numeMelodie), stdin);
  printf("Introduceti comentariul\n");
  fgets(comentariu, sizeof(comentariu), stdin);

  int operatie = 6;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeUsername = strlen(usernameGlobal);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeUsername, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, usernameGlobal, lungimeUsername) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeMelodie = strlen(numeMelodie);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeMelodie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, numeMelodie, lungimeMelodie) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeComentariu = strlen(comentariu);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeComentariu, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, comentariu, lungimeComentariu) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, commentResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  // aici primim inapoi un mesaj de confirmare de genul "Comment added!"

  // verificam daca a fost totul ok

  if (strcmp(commentResponse, "Comment added!") == 0)
  {
    printf("Comment added successfully!\n");
    return 1;
  }
  else
  {
    printf("Please insert a valid song for adding a comment to it!\n");
    return 0;
  }
}

int showComments(int sd)
{

  // fluxul logic este:
  //	- utilizatorul afiseaza topul si il vizualizeaza
  //	- dupa aceea, el hotaraste sa vada comentariile la o anumita melodie

  int lungimeResponse = 0;
  char commentResponse[100] = "";

  // vizualizare top cu piese
  showTop(sd);

  char numeMelodie[1000] = "";
  getchar();
  printf("Introduceti numele melodiei\n");
  fgets(numeMelodie, sizeof(numeMelodie), stdin);

  int operatie = 10;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeMelodie = strlen(numeMelodie);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeMelodie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, numeMelodie, lungimeMelodie) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, commentResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  printf("%s\n", commentResponse);
}

int deleteSong(int sd)
{

  // fluxul logic este:
  //	- utilizatorul afiseaza topul si il vizualizeaza
  //	- dupa aceea, el hotaraste sa stearga o melodie
  //	- se verifica daca utilizatorul e administrator
  //	- va introduce id-ul melodie pe care doreste sa o stearga

  // afisare top

  int lungimeResponse = 0;
  ;
  char deleteResponse[100] = "";

  if (typeGlobal == 0)
  {
    // daca e utilizator simplu returnezi
    printf("To delete a song you have to be an administrator!\n");
    return 0;
  }

  showTop(sd);

  // verificam daca utilizatorul e administrator

  ///// sau cum e corecta verificarea aceasta

  char numeMelodie[1000] = "";
  printf("Introduceti numele melodiei\n");
  getchar();
  fgets(numeMelodie, sizeof(numeMelodie), stdin);

  // aici transmitem prin socket - doar id-ul melodiei

  int operatie = 7;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeMelodie = strlen(numeMelodie);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeMelodie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, numeMelodie, lungimeMelodie) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  // aici primim inapoi un mesaj de confirmare de genul "Song deleted!"

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, deleteResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  // verificam daca a fost totul ok

  if (strcmp(deleteResponse, "Song deleted!") == 0)
  {
    printf("Song deleted successfully!\n");
    return 1;
  }
  else
  {
    printf("Inserted song does not exist!\n");
    return 0;
  }
}

int restrictUserVoteRight(int sd)
{

  // verificam daca utilizatorul actual e administrator
  // ar trebui sa fie afisati toti utilizatorii sa stie admin-ul de unde sa aleaga
  // va introduce id-ul utilizatorului pe care doreste sa il restrictioneze

  // verificam daca utilizatorul e administrator

  int lungimeResponse = 0;

  char restrictResponse[100] = "";

  ///// sau cum e corecta verificarea aceasta
  if (typeGlobal == 0)
  {
    // daca e utilizator simplu returnezi 0
    printf("You have to be administrator to restrict other users!\n");
    return 0;
  }

  //afisam toti utilizatorii sa stie de unde sa aleaga

  showUsers(sd);

  char userToRestrict[100];
  printf("Introduceti username-ul utilizatorului ce doriti a fi restrictionat\n");
  getchar();
  fgets(userToRestrict, sizeof(userToRestrict), stdin);

  // aici transmitem prin socket - doar id-ul utilizatorului ce trebuie restrictionat

  int operatie = 8;
  if (write(sd, &operatie, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  int lungimeUsername = strlen(userToRestrict);
  /* trimiterea mesajului la server */
  if (write(sd, &lungimeUsername, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  if (write(sd, userToRestrict, lungimeUsername) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    printf("%d\n", errno);
  }

  // aici primim inapoi un mesaj de confirmare de genul "Song deleted!"

  if (read(sd, &lungimeResponse, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }
  if (read(sd, restrictResponse, lungimeResponse) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    printf("%d\n", errno);
  }

  // aici primim inapoi un mesaj de confirmare de genul "User restricted!"

  // verificam daca a fost totul ok

  if (strcmp(restrictResponse, "User restricted!") == 0)
  {
    printf("User restricted successfully!\n");
    return 1;
  }
  else if (strcmp(restrictResponse, "Cannot restrict an admin!") == 0)
  {
    printf("Cannot restrict an admin!\n");
    return 0;
  }
  else
  {
    printf("Inserted user does not exist!\n");
    return 0;
  }
}

int main(int argc, char *argv[])
{

  int isLoggedIn = 0;        // 1 - inseamna logat
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
                             // mesajul trimis
  int nr = 0;
  char buf[10];

  // preluare argumente de conectare din linia de comanda
  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  //stablim portului
  port = atoi(argv[2]);

  // creare socket si configurare similar cu serverul
  // adresa este preluata din linia de comanda
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons(port);

  // conectare la server si verificarea posibilelor erori
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  // meniu

  int comanda = 0;
  int raspunsFunctie = 0;

  printf("Bun venit\n");

  while (isLoggedIn == 0)
  {
    printf("Pentru logare apasati - 1\n");
    printf("Pentru signup apasati - 2\n");

    scanf("%d", &comanda);

    if (comanda == 1)
    {
      getchar();
      raspunsFunctie = login(sd);
      if (raspunsFunctie == 1)
      {
        isLoggedIn = 1;
      }
    }
    else if (comanda == 2)
    {
      int raspunsIntermediar = signup(sd);

      printf("Please login now in order to continue!\n");

      if (raspunsIntermediar == 1)
      {

        raspunsFunctie = login(sd);
        if (raspunsFunctie == 1)
        {
          isLoggedIn = 1;
        }
      }
    }

    if (isLoggedIn == 1)
    {

      while (1)
      {
        int operatiune;
        printf("\n");
        printf("\n");
        printf("Pentru logout apasati - 0\n");
        printf("Pentru a adauga o melodie apasati - 1\n");
        printf("Pentru a vota o melodie apasati - 2\n");
        printf("Pentru a afisa topul general apasati - 3\n");
        printf("Pentru a afisa topul pentru un gen de muzica apasati - 4\n");
        printf("Pentru a adauga un comentariu la o melodie apasati - 5\n");
        printf("Pentru a vizualiza comentariile pentru o melodie apasati - 6\n");
        if (typeGlobal == 1)
        {
          printf("Pentru a sterge o melodie apasati - 7\n");
          printf("Pentru a afisa toti utilizatorii apasati - 8\n");
          printf("Pentru a restrictiona un utilizator apasati - 9\n");
        }

        scanf("%d", &operatiune);
        if (operatiune == 0)
        {
          isLoggedIn = 0;
          strcpy(usernameGlobal, "");
          strcpy(passwordGlobal, "");
          isRestrictedGlobal = 0;
          typeGlobal = 0;
          break;
        }
        else if (operatiune == 1)
        {
          adaugaMelodie(sd);
        }
        else if (operatiune == 2)
        {
          voteSong(sd);
        }
        else if (operatiune == 3)
        {
          showTop(sd);
        }
        else if (operatiune == 4)
        {
          showTopForGenre(sd);
        }
        else if (operatiune == 5)
        {
          addComment(sd);
        }
        else if (operatiune == 6)
        {
          showComments(sd);
        }
        else if (operatiune == 7)
        {
          deleteSong(sd);
        }
        else if (operatiune == 8)
        {
          showUsers(sd);
        }
        else if (operatiune == 9)
        {
          restrictUserVoteRight(sd);
        }
      }
    }
    else
    {
      continue;
    }
  }

  /* inchidem conexiunea, am terminat */
  close(sd);
}
