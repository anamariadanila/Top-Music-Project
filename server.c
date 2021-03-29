/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

/*
Documentation for database manipulation:
https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
http://zetcode.com/db/sqlitec/
https://gist.github.com/jsok/2936764
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>

/* portul folosit */
#define PORT 2908

// baza de date folosita
#define DB "proiect.db"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl;		  //descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */

void raspunde(void *);

// functie pentru crearea initiala a tabelelor
void createDbTables()
{

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql;

	/* Open database */
	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stdout, "Opened database successfully\n");
	}

	// creare tabele

	// tabel user

	sql = "CREATE TABLE USER("
		  "ID INTEGER PRIMARY KEY     AUTOINCREMENT,"
		  "NAME           TEXT    NOT NULL,"
		  "USERNAME       TEXT     NOT NULL UNIQUE,"
		  "PASSWORD           TEXT    NOT NULL,"
		  "TYPE				INT NOT NULL, "
		  "ISRESTRICTED	INT NOT NULL); ";

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Table USER created successfully\n");
	}

	// table song

	sql = "CREATE TABLE SONG("
		  "ID INTEGER PRIMARY KEY     AUTOINCREMENT,"
		  "NAME           TEXT    NOT NULL UNIQUE,"
		  "DESCRIPTION       TEXT,"
		  "LINK           TEXT UNIQUE,"
		  "VOTES				INT); ";

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Table SONG created successfully\n");
	}

	// table comment

	sql = "CREATE TABLE COMMENT("
		  "ID INTEGER PRIMARY KEY     AUTOINCREMENT,"
		  "IDUSER           INT    NOT NULL,"
		  "IDSONG       INT NOT NULL,"
		  "DETAILS           TEXT,"
		  "FOREIGN KEY (IDSONG) REFERENCES SONG (ID) ON DELETE CASCADE, "
		  "FOREIGN KEY (IDUSER) REFERENCES USER (ID) ON DELETE CASCADE);";

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Table COMMENT created successfully\n");
	}

	// table genre

	sql = "CREATE TABLE GENRE("
		  "ID INTEGER PRIMARY KEY     AUTOINCREMENT,"
		  "IDSONG       INT NOT NULL,"
		  "DETAILS      TEXT NOT NULL, "
		  "FOREIGN KEY (IDSONG) REFERENCES SONG (ID) ON DELETE CASCADE);";

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Table GENRE created successfully\n");
	}
}

void login(struct thData tdL)
{
	int lungimeResponse = 0;
	char loginResponse[100] = "";
	int lungimeRequest = 0;
	char loginRequest[1000] = "";

	char username[100] = "";
	char password[100] = "";

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	sqlite3_stmt *stmt;

	if (read(tdL.cl, &lungimeRequest, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, loginRequest, lungimeRequest) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	printf("[Thread %d]Mesajul a fost receptionat...%s\n", tdL.idThread, loginRequest);

	/*pregatim mesajul de raspuns */

	// in login response punem isRestricted;type pe care le-am luat din db

	char *sirTokens;
	char delim[] = ";";

	sirTokens = strtok(loginRequest, delim);

	strcpy(username, sirTokens);

	if (sirTokens != NULL)
	{
		sirTokens = strtok(NULL, delim);
	}

	strcpy(password, sirTokens);

	printf("\n%s\n", username);
	printf("\n%s\n", password);

	// db operations

	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	char *sql, sqlQuery[1000] = "";

	username[strlen(username) - 1] = '\0';
	password[strlen(password) - 1] = '\0';

	sprintf(sqlQuery, "select isrestricted, type from user where USERNAME='%s' and PASSWORD='%s'", username, password);

	sql = sqlQuery;

	puts(sql);

	strcpy(loginResponse, "");

	sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

	int step = sqlite3_step(stmt);

	if (step != SQLITE_ROW)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		strcpy(loginResponse, "Bad login");
	}
	else
	{
		fprintf(stdout, "Operation done successfully\n");
		sprintf(loginResponse, "%s;%s", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	lungimeResponse = strlen(loginResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, loginResponse);

	/* returnam mesajul clientului */
	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, loginResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void signup(struct thData tdL)
{
	int tip = 0;

	int lungimeRequest = 0;
	char signupRequest[1000] = "";
	int lungimeResponse = 0;
	char signupResponse[100] = "";

	if (read(tdL.cl, &lungimeRequest, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, signupRequest, lungimeRequest) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	if (read(tdL.cl, &tip, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	printf("[Thread %d]Mesajul a fost receptionat...%s %d\n", tdL.idThread, signupRequest, tip);

	/*pregatim mesajul de raspuns */

	// simulare operatii pe db

	// tokenizare
	char name[100] = "";
	char username[100] = "";
	char password[100] = "";

	char *sirTokens;
	char delim[] = ";";

	sirTokens = strtok(signupRequest, delim);

	strcpy(name, sirTokens);

	if (sirTokens != NULL)
	{
		sirTokens = strtok(NULL, delim);
	}

	strcpy(username, sirTokens);

	if (sirTokens != NULL)
	{
		sirTokens = strtok(NULL, delim);
	}

	strcpy(password, sirTokens);

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];

	/* Open database */
	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	name[strlen(name) - 1] = '\0';
	username[strlen(username) - 1] = '\0';
	password[strlen(password) - 1] = '\0';
	/* Create SQL statement */

	sprintf(sqlQuery, "INSERT INTO USER (name, username, password, type, isrestricted) values ('%s', '%s', '%s', %d, %d)", name, username, password, 0, 0);

	sql = sqlQuery;

	puts(sql);

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		strcpy(signupResponse, "Bad signup!");
	}
	else
	{
		fprintf(stdout, "Records created successfully\n");
		strcpy(signupResponse, "Signup successfully!");
	}
	sqlite3_close(db);

	//strcpy(signupResponse, "Bad signup!");
	lungimeResponse = strlen(signupResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, signupResponse);

	/* returnam mesajul clientului */
	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, signupResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void addSong(struct thData tdL)
{

	char addSongResponse[200] = "";

	int lungimeNume = 0;
	int lungimeDescriere = 0;
	int lungimeLink = 0;

	int lungimeRequest = 0;
	int lungimeResponse = 0;
	int tip = 0;
	int lungimeGenuri = 0;

	char nume[100] = "", descriere[500] = "", genuri[1000] = "", link[200] = "";

	if (read(tdL.cl, &lungimeNume, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, nume, lungimeNume) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	if (read(tdL.cl, &lungimeDescriere, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, descriere, lungimeDescriere) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	if (read(tdL.cl, &lungimeLink, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, link, lungimeLink) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	if (read(tdL.cl, &lungimeGenuri, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, genuri, lungimeGenuri) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	printf("[Thread %d]Mesajul a fost receptionat...%s\n", tdL.idThread, genuri);

	/*pregatim mesajul de raspuns */

	// operatii pe db

	int partialOk = 0;
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	sqlite3_stmt *stmt;
	char *sql, sqlQuery[1000];

	/* Open database */
	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	// first put the song without the genres
	nume[lungimeNume - 1] = '\0';
	descriere[lungimeDescriere - 1] = '\0';
	link[lungimeLink - 1] = '\0';
	genuri[lungimeGenuri - 1] = '\0';

	sprintf(sqlQuery, "insert into SONG (name, description, link, votes) values ('%s', '%s', '%s', %d)", nume, descriere, link, 0);

	sql = sqlQuery;

	puts(sql);

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Records created successfully\n");
		partialOk = 1;
	}

	if (!partialOk)
	{
		// song already exists
		sqlite3_close(db);

		strcpy(addSongResponse, "Song already exists!");
	}
	else
	{

		// get the id of the inserted song
		strcpy(sqlQuery, "");
		sprintf(sqlQuery, "select id from SONG where name='%s'", nume);
		sql = sqlQuery;

		puts(sql);

		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		int step = sqlite3_step(stmt);

		int songId = 0;
		if (step == SQLITE_ROW)
		{
			printf("\nId is: %s\n", sqlite3_column_text(stmt, 0));
			songId = atoi(sqlite3_column_text(stmt, 0));
		}

		sqlite3_finalize(stmt);

		// insert genres in genre table

		strcpy(sqlQuery, "");

		char *sirTokens;
		char delim[] = " ";

		sirTokens = strtok(genuri, delim);

		while (sirTokens != NULL)
		{
			sprintf(sqlQuery, "insert into GENRE (idsong, details) values (%d, '%s');", songId, sirTokens);

			sql = sqlQuery;

			puts(sql);

			/* Execute SQL statement */
			rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

			if (rc != SQLITE_OK)
			{
				fprintf(stderr, "SQL error: %s\n", zErrMsg);
				sqlite3_free(zErrMsg);
			}
			else
			{
				fprintf(stdout, "Records created successfully\n");
			}

			sirTokens = strtok(NULL, delim);
		}

		sqlite3_close(db);

		strcpy(addSongResponse, "Song added!");
	}

	lungimeResponse = strlen(addSongResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, addSongResponse);

	/* returnam mesajul clientului */
	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, addSongResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void addComment(struct thData tdL)
{

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];
	sqlite3_stmt *stmt;

	int lungimeUsername, lungimeMelodie, lungimeComentariu;
	char username[100], melodie[100], comentariu[100];

	int lungimeResponse;
	char commentResponse[1000];

	if (read(tdL.cl, &lungimeUsername, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, username, lungimeUsername) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	if (read(tdL.cl, &lungimeMelodie, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, melodie, lungimeMelodie) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	if (read(tdL.cl, &lungimeComentariu, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, comentariu, lungimeComentariu) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	// operatii pe db

	username[lungimeUsername - 1] = '\0';
	melodie[lungimeMelodie - 1] = '\0';
	comentariu[lungimeComentariu - 1] = '\0';

	int userId = 0;
	int songId = 0;
	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	int notExists = 0;
	int notExists2 = 0;

	sprintf(sqlQuery, "select id from USER where username='%s'", username);

	sql = sqlQuery;

	puts(sql);

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	int step = sqlite3_step(stmt);

	if (step == SQLITE_ROW)
	{

		userId = atoi(sqlite3_column_text(stmt, 0));
		printf("\nUser %d\n", userId);
	}
	else
	{
		notExists = 1;
	}

	sqlite3_finalize(stmt);

	strcpy(sqlQuery, "");
	sprintf(sqlQuery, "select id from SONG where name='%s'", melodie);

	sql = sqlQuery;

	puts(sql);

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	step = sqlite3_step(stmt);

	if (step == SQLITE_ROW)
	{

		songId = atoi(sqlite3_column_text(stmt, 0));
		printf("\nPiesa %d\n", songId);
	}
	else
	{
		notExists2 = 1;
	}

	sqlite3_finalize(stmt);

	if (!notExists && !notExists2)
	{

		strcpy(sqlQuery, "");
		sprintf(sqlQuery, "insert into COMMENT (iduser, idsong, details) values (%d, %d, '%s')", userId, songId, comentariu);

		sql = sqlQuery;

		puts(sql);

		rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		else
		{
			fprintf(stdout, "Operation done successfully\n");
			strcpy(commentResponse, "Comment added!");
		}
	}
	else
	{
		strcpy(commentResponse, "Error on adding the comment!");
	}

	sqlite3_close(db);

	lungimeResponse = strlen(commentResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, commentResponse);

	// trimitere raspuns

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, commentResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void voteSong(struct thData tdL)
{

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];
	sqlite3_stmt *stmt;

	int lungimeMelodie;
	char melodie[100];

	int lungimeResponse;
	char voteResponse[1000] = "";

	if (read(tdL.cl, &lungimeMelodie, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, melodie, lungimeMelodie) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	int nrVotes = 0;
	melodie[lungimeMelodie - 1] = '\0';

	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf(sqlQuery, "select votes from SONG where name='%s'", melodie);

	sql = sqlQuery;

	puts(sql);

	int notExists = 0;

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	int step = sqlite3_step(stmt);

	if (step == SQLITE_ROW)
	{

		nrVotes = atoi(sqlite3_column_text(stmt, 0));
		printf("\nVoturi: %d\n", nrVotes);
	}
	else
	{
		notExists = 1;
	}

	sqlite3_finalize(stmt);

	if (!notExists)
	{
		nrVotes++;

		strcpy(sqlQuery, "");
		sprintf(sqlQuery, "update SONG set votes = %d where name='%s'", nrVotes, melodie);

		sql = sqlQuery;

		puts(sql);

		rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		else
		{
			fprintf(stdout, "Operation done successfully\n");
			strcpy(voteResponse, "Song voted!");
		}
	}
	else
	{
		strcpy(voteResponse, "Error on voting song!");
	}

	sqlite3_close(db);

	lungimeResponse = strlen(voteResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, voteResponse);

	//operatii pe db
	// trimitere raspuns

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, voteResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void showUsers(struct thData tdL)
{

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];
	sqlite3_stmt *stmt;

	int lungimeUsername;
	char username[100];

	int lungimeResponse;
	char usersResponse[5000] = "";

	if (read(tdL.cl, &lungimeUsername, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, username, lungimeUsername) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	// verificam in db daca e admin
	// selectare din db

	username[lungimeUsername - 1] = '\0';

	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf(sqlQuery, "select type from USER where username='%s';", username);

	sql = sqlQuery;

	puts(sql);

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	int step = sqlite3_step(stmt);
	int type = 0;
	if (step == SQLITE_ROW)
	{
		type = atoi(sqlite3_column_text(stmt, 0));
	}

	sqlite3_finalize(stmt);

	printf("Utilizatorul este: %d", type);

	if (type == 1)
	{
		// e administrator

		sprintf(sqlQuery, "select * from USER");

		sql = sqlQuery;

		puts(sql);

		sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

		strcat(usersResponse, "id|name|username|password|type|isrestricted\n___________________________________________\n");

		while (sqlite3_step(stmt) != SQLITE_DONE)
		{

			int i;
			int num_cols = sqlite3_column_count(stmt);

			for (i = 0; i < num_cols - 1; i++)
			{

				strcat(usersResponse, sqlite3_column_text(stmt, i));
				strcat(usersResponse, "|");
			}
			strcat(usersResponse, sqlite3_column_text(stmt, i));

			strcat(usersResponse, "\n");
		}

		sqlite3_finalize(stmt);

		puts(usersResponse);
	}
	else
	{
		strcpy(usersResponse, "You are not admin!\n");
	}

	sqlite3_close(db);

	// afisam raspunsul

	lungimeResponse = strlen(usersResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, usersResponse);

	// trimitere raspuns

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, usersResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void showGeneralTopMusic(struct thData tdL)
{

	// se ia de pe db topul, order by votes DESC

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, *sql2, sqlQuery[1000], sqlQuery2[1000];
	sqlite3_stmt *stmt, *stmt2;

	int lungimeResponse = 0;
	char topResponse[5000] = "";

	// selectare din db

	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf(sqlQuery, "select id, name, description, link, votes from SONG order by votes desc");

	sql = sqlQuery;

	puts(sql);

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	strcat(topResponse, "id|name|description|link|votes|genres\n_____________________________________\n");

	while (sqlite3_step(stmt) != SQLITE_DONE)
	{

		int i;
		int num_cols = sqlite3_column_count(stmt);

		for (i = 0; i < num_cols; i++)
		{

			strcat(topResponse, sqlite3_column_text(stmt, i));
			strcat(topResponse, "|");
		}

		// for each song select genres

		int songId = atoi(sqlite3_column_text(stmt, 0));

		sprintf(sqlQuery2, "select details from GENRE where idsong=%d", songId);

		sql2 = sqlQuery2;

		puts(sql2);

		sqlite3_prepare_v2(db, sql2, -1, &stmt2, 0);

		char genuri[1000] = "";

		while (sqlite3_step(stmt2) != SQLITE_DONE)
		{
			strcat(genuri, sqlite3_column_text(stmt2, 0));
			strcat(genuri, " ");
		}

		genuri[strlen(genuri) - 1] = '\0';

		sqlite3_finalize(stmt2);

		strcat(topResponse, genuri);

		strcat(topResponse, "\n");
	}

	sqlite3_finalize(stmt);

	puts(topResponse);

	sqlite3_close(db);

	lungimeResponse = strlen(topResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, topResponse);

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, topResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void showMusicTypeTop(struct thData tdL)
{

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];
	sqlite3_stmt *stmt;

	int lungimeResponse = 0;
	char topResponse[5000] = "";

	int lungimeGenMuzical = 0;
	char genMuzical[1000] = "";

	if (read(tdL.cl, &lungimeGenMuzical, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, genMuzical, lungimeGenMuzical) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	// se ia de pe db topul, order by votesNumber DESC, where genul muzical din tabelul
	// asociat e cel furnizat de client

	genMuzical[lungimeGenMuzical - 1] = '\0';

	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf(sqlQuery, "select s.id, s.name, s.description, s.link, s.votes from SONG s join GENRE g on (s.id=g.idsong) where g.details='%s' order by s.votes desc", genMuzical);

	sql = sqlQuery;

	puts(sql);

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	strcat(topResponse, "id|name|description|link|votes\n____________________________________\n");

	while (sqlite3_step(stmt) != SQLITE_DONE)
	{
		//char raspPartial[1000]="";
		int i;
		int num_cols = sqlite3_column_count(stmt);

		for (i = 0; i < num_cols - 1; i++)
		{

			strcat(topResponse, sqlite3_column_text(stmt, i));
			strcat(topResponse, "|");
		}
		strcat(topResponse, sqlite3_column_text(stmt, i));

		strcat(topResponse, "\n");
	}

	sqlite3_finalize(stmt);

	puts(topResponse);

	sqlite3_close(db);

	lungimeResponse = strlen(topResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, topResponse);

	// trimitere raspuns

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, topResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void deleteSong(struct thData tdL)
{

	int lungimeResponse = 0;
	char deleteResponse[100] = "";

	int lungimeMelodie = 0;
	char melodie[1000] = "";

	if (read(tdL.cl, &lungimeMelodie, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, melodie, lungimeMelodie) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	// operatii pe db

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];

	/* Open database */
	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	melodie[lungimeMelodie - 1] = '\0';

	// activate foreign keys
	sql = "PRAGMA foreign_keys = ON;";

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Operation executed successfully!\n");
	}

	sprintf(sqlQuery, "delete from SONG where name='%s'", melodie);

	sql = sqlQuery;

	puts(sql);

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		strcpy(deleteResponse, "Error on deleting song!");
	}
	else
	{
		fprintf(stdout, "Operation done successfully!\n");
		strcpy(deleteResponse, "Song deleted!");
	}

	sqlite3_close(db);

	lungimeResponse = strlen(deleteResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, deleteResponse);

	// trimitere raspuns

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, deleteResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void restricUserVoteRight(struct thData tdL)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];
	sqlite3_stmt *stmt;

	int lungimeResponse = 0;
	char restrictResponse[100] = "";

	int lungimeUsername = 0;
	char username[1000] = "";

	if (read(tdL.cl, &lungimeUsername, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, username, lungimeUsername) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	// operatii pe db

	int userId = 0, userType = 0;
	username[lungimeUsername - 1] = '\0';

	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf(sqlQuery, "select id, type from USER where username='%s'", username);

	sql = sqlQuery;

	puts(sql);

	int notExists = 0;

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	int step = sqlite3_step(stmt);

	if (step == SQLITE_ROW)
	{

		userId = atoi(sqlite3_column_text(stmt, 0));
		userType = atoi(sqlite3_column_text(stmt, 1));
	}
	else
	{
		notExists = 1;
	}

	sqlite3_finalize(stmt);

	if (userType == 1)
	{
		strcpy(restrictResponse, "Cannot restrict an admin!");
	}

	else if (!notExists && !userType)
	{

		strcpy(sqlQuery, "");
		sprintf(sqlQuery, "update USER set isrestricted = %d where id=%d", 1, userId);

		sql = sqlQuery;

		puts(sql);

		rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		else
		{
			fprintf(stdout, "Operation done successfully\n");
			strcpy(restrictResponse, "User restricted!");
		}
	}
	else
	{
		strcpy(restrictResponse, "User doesn't exist!");
	}

	sqlite3_close(db);

	lungimeResponse = strlen(restrictResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, restrictResponse);

	// trimitere raspuns

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, restrictResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

void showComments(struct thData tdL)
{

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, sqlQuery[1000];
	sqlite3_stmt *stmt;

	int lungimeMelodie;
	char melodie[100];

	int lungimeResponse;
	char commentsResponse[10000] = "";

	if (read(tdL.cl, &lungimeMelodie, sizeof(int)) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}
	if (read(tdL.cl, melodie, lungimeMelodie) <= 0)
	{
		printf("[Thread %d]\n", tdL.idThread);
		perror("Eroare la read() de la client.\n");
	}

	int nrVotes = 0;
	melodie[lungimeMelodie - 1] = '\0';

	rc = sqlite3_open(DB, &db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	sprintf(sqlQuery, "select c.details from SONG s join COMMENT c on (s.id=c.idsong) where s.name='%s'", melodie);

	sql = sqlQuery;

	puts(sql);

	sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

	while (sqlite3_step(stmt) != SQLITE_DONE)
	{

		strcat(commentsResponse, sqlite3_column_text(stmt, 0));
		strcat(commentsResponse, "\n");
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	lungimeResponse = strlen(commentsResponse);
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n", tdL.idThread, commentsResponse);

	//operatii pe db
	// trimitere raspuns

	if (write(tdL.cl, &lungimeResponse, sizeof(int)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

	if (write(tdL.cl, commentsResponse, lungimeResponse) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}

int main()
{
	struct sockaddr_in server; // structura folosita de server
	struct sockaddr_in from;
	int nr; //mesajul primit de trimis la client
	int sd; //descriptorul de socket
	int pid;
	pthread_t th[100]; //Identificatorii thread-urilor care se vor crea
	int i = 0;

	// creare tabele in baza de date
	// se foloseste doar cand e nevoie in rest se comenteaza
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 //createDbTables();

	// crearea unui socket
	// configurarea optiunii pentru socket
	//pregatirea structurilor de date
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("[server]Eroare la socket().\n");
		return errno;
	}
	int on = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&server, sizeof(server));
	bzero(&from, sizeof(from));

	// stabilirea familiei de socket-uri
	// stabilirea adreselor acceptate (orice adresa)
	// utilizarea unui port utilizator
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);

	// atasarea socket-ului si verificarea posibilelor erori
	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		perror("[server]Eroare la bind().\n");
		return errno;
	}

	// serverul asculta daca vin clienti
	if (listen(sd, 2) == -1)
	{
		perror("[server]Eroare la listen().\n");
		return errno;
	}

	// servirea concurenta a clientilor
	// utilizam thread-uri
	while (1)
	{
		int client;
		thData *td; //parametru functia executata de thread
		int length = sizeof(from);

		printf("[server]Asteptam la portul %d...\n", PORT);
		fflush(stdout);

		// acceptarea clientului si verificare posibilelor erori
		if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
		{
			perror("[server]Eroare la accept().\n");
			continue;
		}

		// dupa realizarea conexiunii se astepata
		//id-ul thread-ului si descriptorul intors de accept

		td = (struct thData *)malloc(sizeof(struct thData));
		td->idThread = i++;
		td->cl = client;

		// crearea thread-ului
		pthread_create(&th[i], NULL, &treat, td);
	}
};

// functie pentru tratarea unui client
static void *treat(void *arg)
{
	struct thData tdL;
	tdL = *((struct thData *)arg);
	printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
	fflush(stdout);
	pthread_detach(pthread_self());
	raspunde((struct thData *)arg);
	/* am terminat cu acest client, inchidem conexiunea */
	close((intptr_t)arg);
	return (NULL);
};

void raspunde(void *arg)
{

	int operatiune = 0;

	struct thData tdL;
	tdL = *((struct thData *)arg);

	while (1)
	{

		// citirea operatiunii pe care clientul doreste sa o execute
		if (read(tdL.cl, &operatiune, sizeof(int)) <= 0)
		{
			printf("[Thread %d]\n", tdL.idThread);
			perror("Eroare la read() de la client.\n");
			break;
		}

		if (operatiune == 0)
		{
			//login
			login(tdL);
		}
		else if (operatiune == 1)
		{
			// signup
			signup(tdL);
		}
		else if (operatiune == 2)
		{
			// insert melodie
			addSong(tdL);
		}

		else if (operatiune == 3)
		{
			// show top general

			showGeneralTopMusic(tdL);
		}
		else if (operatiune == 4)
		{
			// show top for music genre
			showMusicTypeTop(tdL);
		}
		else if (operatiune == 5)
		{
			// vote song

			voteSong(tdL);
		}
		else if (operatiune == 6)
		{
			// comentariu

			addComment(tdL);
		}
		else if (operatiune == 7)
		{
			// deleteSong

			deleteSong(tdL);
		}
		else if (operatiune == 8)
		{
			// restrict user vote right

			restricUserVoteRight(tdL);
		}
		else if (operatiune == 9)
		{
			// show users
			showUsers(tdL);
		}
		else if (operatiune == 10)
		{
			// show comments
			showComments(tdL);
		}
	}
}
