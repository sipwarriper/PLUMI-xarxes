/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer lumi.c que implementa la capa d'aplicació de LUMI, sobre la     */
/* de transport UDP (fent crides a la interfície de la capa UDP           */
/* -sockets-).                                                            */
/* Autors: Ismael El Habri, Lluís Trilla                                                           */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include "MIp2-lumi.h")                                           */

#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "MIp2-lumi.h"
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

/* Definició de constants, p.e., #define MAX_LINIA 150                    */

/* Declaració de funcions internes que es fan servir en aquest fitxer     */
/* (les seves definicions es troben més avall) per així fer-les conegudes */
/* des d'aqui fins al final de fitxer.                                    */
/* Com a mínim heu de fer les següents funcions internes:                 */

int UDP_CreaSock(const char *IPloc, int portUDPloc);
int UDP_EnviaA(int Sck, const char *IPrem, int portUDPrem, const char *SeqBytes, int LongSeqBytes);
int UDP_RepDe(int Sck, char *IPrem, int *portUDPrem, char *SeqBytes, int LongSeqBytes);
int UDP_TancaSock(int Sck);
int UDP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portUDPloc);
int UDP_DemanaConnexio(int Sck, const char *IPrem, int portUDPrem);
int UDP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes);
int UDP_Rep(int Sck, char *SeqBytes, int LongSeqBytes);
int UDP_TrobaAdrSockRem(int Sck, char *IPrem, int *portUDPrem);
int HaArribatAlgunaCosaEnTemps(const int *LlistaSck, int LongLlistaSck, int Temps);
FILE* Log_CreaFitx(const char *NomFitxLog);
int Log_Escriu(FILE* FitxLog, const char *MissLog);
int Log_TancaFitx(FILE* FitxLog);

FILE* arxiuLog;

/* Definicio de funcions EXTERNES, és a dir, d'aquelles que en altres     */
/* fitxers externs es faran servir.                                       */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI.                                  */
/* Les funcions externes les heu de dissenyar vosaltres...                */

/* Crea un socket LUMI a l’@IP “IPloc” i #port UDP “portUDPloc”            */
/* (si “IPloc” és “0.0.0.0” i/o “portUDPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port UDP, respectivament).      */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int LUMI_crearSocket(const char *IPloc, int portUDPloc){
    Log_Escriu(arxiuLog,"Creem socket\n");
    return UDP_CreaSock(IPloc, portUDPloc);
}

/* Funció que inicialitza el servidor LUMI amb la informació trobada al fitxer de configuraicó anomenat "nomFitxer"
 * i emplena "client" amb els clients trobats i "domini" amb el domini propi. A nClients fica el numero de clients que
 * ha trobat.
 * Retorna -1 si hi ha error amb el fitxer i el codi identificatiu d'aquest en cas contrari
 * */
int LUMI_iniServ(const char* nomFitxer, int *nClients, struct Client *client, char* domini){
    FILE * fid = fopen(nomFitxer,"r");
    int count=0;
    fscanf(fid, "%s", domini);
    domini[29]='\0';
    if (fid==NULL) return -1;
    if (feof(fid) || ferror(fid)) return -1;
    //fgetc(fid);
    while (!feof(fid)){
        fscanf(fid, "%s", client[count].nom);
        fscanf(fid, "%i", &client[count].estat);
        fscanf(fid, "%s", client[count].IP);
        fscanf(fid, "%i ", &client[count].port);
        printf("%s %d %s %d",client[count].nom,client[count].estat,client[count].IP,client[count].port);
        count++;
        //haig de tractar el salt de linia?
    }
    *nClients = count;
    fclose(fid);
    arxiuLog=Log_CreaFitx("log.txt");
    Log_Escriu(arxiuLog,"Servidor inicialitzat\n");
    return 0;
}
int LUMI_finiServ(){
    Log_TancaFitx(arxiuLog);
    return 0;
}


/*
 * Funció que actualitza el fitxer de configuració a fid amb l'estat actual dels clients
 * Retorna 1   */
int LUMI_ActualitzarFitxerRegistre(const struct Client *clients, int nClients, const char *nomFitxer, const char* domini){
    Log_Escriu(arxiuLog,"Actualitzem registre \n");
    int fid = open(nomFitxer, O_CREAT|O_TRUNC|O_WRONLY);
	int writeB = write(fid, domini, strlen(domini));
    int i;
    writeB+=write(fid,"\n",strlen("\n"));
    printf("nClients: %d",nClients);
    char buffer[200];
	for (i=0; i<nClients; i++){
        int a = sprintf(buffer,"%s %d %s %d \n",clients[i].nom,clients[i].estat, clients[i].IP, clients[i].port);
        writeB = write(fid, buffer, a);
    }
	close(fid);
    return 1;
}


/*
 * Funció per demanar connexió UDP a un servidor que deixa en estat -established-
 * Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int LUMI_connexio(int Sck, const char *IPrem, int portUDPrem){
    return UDP_DemanaConnexio(Sck,IPrem, portUDPrem);
}

/*
 * Funció que demana una petició de desregistre al servidor connectat a Sck de l'usuari MI
 * Retorna -2 si el servidor està desconectat; -1 si hi ha error; 0 si el desregistre es correcte; 1 si l'usuari no existeix; 2 si format incorrecte  */
int LUMI_Desregistre(int Sck, const char * MI){
    char buffer[50];
    int b = sprintf(buffer,"D%s", MI);
    int x, i=0, rEnvio;
    int a[1];
    a[0]=Sck;
    do{
        if ((x = UDP_Envia(Sck, buffer, b)) == -1) return -1;
        rEnvio = HaArribatAlgunaCosaEnTemps(a,1,50);
        i++;
    }while(rEnvio==-2 && i<5);
    if (rEnvio==-2) return -2;
    if ((x = UDP_Rep(Sck, buffer,50))==-1)return -1;
    //printf("%d",((int)buffer[1]-48));
    return ((int)buffer[1]-48);
}

/*
 * Funció que demana una petició de registre al servidor connectat a Sck de l'usuari MI
 * Retorna -2 si el servidor està desconectat; -1 si hi ha error; 0 si el desregistre es correcte; 1 si l'usuari no existeix; 2 si format incorrecte  */
int LUMI_Registre(int Sck, const char * MI){
    char buffer[50];
    int b = sprintf(buffer,"R%s", MI);
    int x, i=0, rEnvio;
    int a[1];
    a[0]=Sck;
    do{
        if ((x = UDP_Envia(Sck, buffer, b)) == -1) return -1;
        rEnvio = HaArribatAlgunaCosaEnTemps(a,1,50);
        i++;
    }while(rEnvio==-2 && i<5);
    if (rEnvio==-2) return -2;
    x = UDP_Rep(Sck, buffer,50);
    if ((x = UDP_Rep(Sck, buffer,50))==-1)return -1;
	//printf("%d",((int)buffer[1]-48));
    return ((int)buffer[1]-48);
}

int LUMI_Localitzacio(int Sck, const char *MIloc, const char *MIrem, char * IP, int * portTCP){
    char buffer[60];
    int b = sprintf(buffer,"L%s/%s",MIrem, MIloc);
    int x, i=0, rEnvio;
    int a[1];
    a[0]=Sck;
    do{
        if ((x = UDP_Envia(Sck, buffer, b)) == -1) return -1;
        rEnvio = HaArribatAlgunaCosaEnTemps(a,1,50);
        i++;
    }while(rEnvio==-2 && i<5);
    if (rEnvio==-2) return -2;
    else if(rEnvio==-1) return -1;
    puts("HEM REBUT RESPOSTA!\n");
    if ((x = UDP_Rep(Sck, buffer,60))==-1)return -1;
    printf("PAKET:%s",buffer);
    int z=strlen(MIloc)+3; //posicio on comença el port
    char portTemp[7];
    printf("z= %d\n",z);
    int y = z;
    while (buffer[z] != '/') {
        portTemp[z - y] = buffer[z];
        z++;
    }
    portTemp[z-y]='\0';
    printf("porttemp= %s",portTemp);
    *portTCP = strtol(portTemp, (char **) NULL, 10);
    z++; //(el '/')
    int f = z;
    printf("z= %d\n",z);
    printf("x= %d\n",x);
    while(z<x){
        IP[z-f]=buffer[z];
        z++;
    }
    IP[z-f]='\0';
    printf("z= %d\n",z);
    return ((int)buffer[1]-48); //retorna el codi de resposta!


}


int LUMI_RLocalitzacio(int Sck, const char* IP, int portTCP, int estat){
    char MIrem1[30];
    char buffer[60];
    char missatge[60];
    int i=1, b;
    int longMissatge = UDP_Rep(Sck,missatge,60);
    printf("TINC LA PETICIO");
    int cursor=0;
    while(missatge[cursor]!='/')cursor++;
    cursor++;
    int cursorini=cursor;
    while(cursor<longMissatge){
        MIrem1[cursor-cursorini]=missatge[cursor];
        cursor++;
    }
    MIrem1[cursor-cursorini]='\0';
    b = sprintf(buffer,"B%d%s/%d/%s\0",estat, MIrem1, portTCP, IP);
    printf("misToSend: %s\n",buffer);
    printf("misRebut: %s\n",missatge);
    return UDP_Envia(Sck, buffer, b);
}

/*
 * Funció que descxifra quina sol·licitud li han donat al servidor
 * Retorna -1 si no coneix la petició; 0 si és desregistre, 1 si és Registre, 2 si es localització i 3 si es resposta a Localització */
int LUMI_ServDescxifrarRebut(const char* missatge) {
    Log_Escriu(arxiuLog,"Desxifrem la sol·licitud\n");
    char a = missatge[0];
    if(a=='D') return DESREGISTRE;
    else if(a=='R') return REGISTRE;
    else if(a=='L') return LOCALITZACIO;
    else if(a=='B') return RESPOSTALOC;
    else return -1;
}
/*
 * Funció que fa el registre d'un client al servidor, i envia la resposta adient al socket inicial, també actualitza el arxiu de clients
 * */
int LUMI_ServidorReg(struct Client *clients, int nClients,const char *Entrada,  const char *IP, int port, const char *nomFitxer,const char* domini, int socket) {
    Log_Escriu(arxiuLog,"Fem registre\n");
    if (Entrada[0] != 'R') {
        UDP_EnviaA(socket, IP, port, "A2", 2);
        return 2;
    }
    char nom[150];
    strcpy(nom,&Entrada[1]);
    Log_Escriu(arxiuLog,&Entrada[1]);
    int acabat=0, i=0;
    while (acabat==0 && i<nClients){
        if(strcmp(clients[i].nom,nom)==0){
            clients[i].estat=LLIURE;
            clients[i].port=port;
            strcpy(clients[i].IP,IP);
            acabat=1;
        }
        i++;
    }
    if(acabat==0) {
        Log_Escriu(arxiuLog,"USUARI INEXISTENT\n");
        UDP_EnviaA(socket,IP,port,"A1",2);
        return 1;
    }
    Log_Escriu(arxiuLog,"Usuari trobat\n");
    UDP_EnviaA(socket,IP,port,"A0",2);
    LUMI_ActualitzarFitxerRegistre(clients,nClients,nomFitxer,domini);
    return 0;
}

/*
 * Funció que fa el desregistre d'un client al servidor, i envia la resposta adient al socket inicial, també actualitza el arxiu de clients
 * */
int LUMI_ServidorDesreg(struct Client *clients, int nClients,const char *Entrada, const char *IP, int port,const char *nomFitxer , const char* domini, int socket){
    Log_Escriu(arxiuLog,"Fem desregistre\n");
    if(Entrada[0]!='D'){
        UDP_EnviaA(socket, IP, port, "A2", 2);
        return 2;
    }
    char nom[150];
    strcpy(nom,&Entrada[1]);
    int acabat =0, i=0;
    while (acabat==0 && i<nClients){
        if(strcmp(clients[i].nom,nom)==0){
            clients[i].estat=DESCONNECTAT;
            acabat=1;
        }
        i++;
    }
    if(acabat==0) {
        UDP_EnviaA(socket,IP,port,"A1",2);
        return 1;
    }
    LUMI_ActualitzarFitxerRegistre(clients,nClients,nomFitxer,domini);
    UDP_EnviaA(socket,IP,port,"A0",2);
    return 0;
}
int LUMI_ServidorLoc(int Sck, char * missatge, int longMissatge, const char* dominiloc, struct Client *clients, int nClients, const char* IPrem, int portRem){
    Log_Escriu(arxiuLog,"Localitzem\n");
    printf("Missatge: %s\n",missatge);
    puts(dominiloc);
    puts("\n");
    int i=1, j=0;
    char domini[20];
    while(missatge[i]!='@') i++;
    i++;
    while(missatge[i]!='/'){
        domini[j] = missatge[i];
        i++;
        j++;
    }
    char MIrem[30];
    int a=i+1;
    while(a!=longMissatge){
        MIrem[a-(i+1)]=missatge[a];
        a++;
    }
    domini[j]='\0';
    puts(domini);
    puts("\n");
    puts(dominiloc);
    puts("\n");
    if(strcmp(domini, dominiloc)==0){
        //domini propi, has de buscar el client i enviarli la solicitud!
        Log_Escriu(arxiuLog,"DOMINI PROPI");
        char nom[50];
        for(j=1;j<i;j++) nom[j-1]=missatge[j];
        nom[j-1]='\0';
        //buscar als clients
        printf("Client a buscar: |%s|\n",nom);
        int trobat=0, cont=0;
        while(trobat==0 && cont<nClients){
            printf("Candidat: |%s|\n",clients[cont].nom);
            if(strcmp(nom,clients[cont].nom)==0) trobat = 1;
            else cont++;
        }
        if (trobat == 0) {
            Log_Escriu(arxiuLog,"Usuari no trobat");
            char buffer[60];
            int b = sprintf(buffer,"B%d%s/%d/%s",2, MIrem, 0, "0.0.0.0");
            UDP_EnviaA(Sck,IPrem,portRem,buffer,b);
        }
        else if(clients[cont].estat==DESCONNECTAT) {
            char buffer[60];
            int b = sprintf(buffer, "B%d%s/%d/%s", 1, MIrem, 0, "0.0.0.0");
            UDP_EnviaA(Sck, IPrem, portRem, buffer, b);
        }
        else{
            Log_Escriu(arxiuLog,"Usuari trobat");
            if(UDP_EnviaA(Sck, clients[cont].IP,clients[cont].port,missatge,longMissatge)==-1) return -1;
        }
    }
    else {
        Log_Escriu(arxiuLog,"Domini extern");
        char IP[16];
        ResolDNSaIP(domini, IP);
        if (UDP_EnviaA(Sck,IP,1714,missatge,longMissatge)==-1) return -1;
    }
    return 1;
}

int LUMI_ServidorRLoc(int Sck, char * missatge, int longMissatge, const char* dominiloc, struct Client *clients, int nClients){
    Log_Escriu(arxiuLog,"Tornem resposta a localització\n");
    printf("Missatge: %s\n",missatge);
    int i=0, j=0;
    char domini[20];
    while(missatge[i]!='@') i++;
    i++;
    while(missatge[i]!='/'){
        domini[j] = missatge[i];
        i++;
        j++;
    }
    domini[j]='\0';
    printf("dom: |%s|\n",domini);
    printf("domLoc: |%s|\n",dominiloc);
    if(strcmp(domini, dominiloc)==0){
        Log_Escriu(arxiuLog,"DOMINI PROPI");
        //domini propi, has de buscar el client i enviarli la solicitud!
        char nom[50];
        for(j=2;j<i;j++) nom[j-2]=missatge[j];
        nom[j-2]='\0';
        //buscar als clients
        printf("Client a buscar: |%s|\n",nom);
        int trobat=0, cont=0;
        while(trobat==0 && cont<nClients){
            printf("Candidat: |%s|\n",clients[cont].nom);
            if(strcmp(nom,clients[cont].nom)==0) trobat = 1;
            else cont++;
        }
        printf("envio: %s \n", missatge);
        if(UDP_EnviaA(Sck, clients[cont].IP,clients[cont].port,missatge,longMissatge)==-1) return -1;
        else{
            Log_Escriu(arxiuLog,"Usuari trobat");
        }
    }
    else {
        //resoldre domini i repetir resposta
        Log_Escriu(arxiuLog,"Domini extern");
        char IP[16];
        ResolDNSaIP(domini, IP);
        printf("envio: %s \n", missatge);
        if (UDP_EnviaA(Sck,IP,1714,missatge,longMissatge)==-1) return -1;
        Log_Escriu(arxiuLog,"Enviada resposta");
    }
    return 1;
}

/* Definicio de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer.                                  */

/* Crea un socket UDP a l’@IP “IPloc” i #port UDP “portUDPloc”            */
/* (si “IPloc” és “0.0.0.0” i/o “portUDPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port UDP, respectivament).      */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int UDP_CreaSock(const char *IPloc, int portUDPloc)
{
    int sock, i;
    struct sockaddr_in adrloc;
    if((sock=socket(AF_INET,SOCK_DGRAM,0))==-1) return -1;
    adrloc.sin_family=AF_INET;
    adrloc.sin_port=htons(portUDPloc);
    adrloc.sin_addr.s_addr=inet_addr(IPloc);    /* o bé: ...s_addr = INADDR_ANY */
    for(i=0;i<8;i++){adrloc.sin_zero[i]='\0';}
    if((bind(sock,(struct sockaddr*)&adrloc,sizeof(adrloc)))==-1) return -1;
    return sock;
}

/* Envia a través del socket UDP d’identificador “Sck” la seqüència de    */
/* bytes escrita a “SeqBytes” (de longitud “LongSeqBytes” bytes) cap al   */
/* socket remot que té @IP “IPrem” i #port UDP “portUDPrem”.              */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes       */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int UDP_EnviaA(int Sck, const char *IPrem, int portUDPrem, const char *SeqBytes, int LongSeqBytes)
{
    int i, bescrit;
    struct sockaddr_in adrrem;
    adrrem.sin_family=AF_INET;
    adrrem.sin_port=htons(portUDPrem);
    adrrem.sin_addr.s_addr= inet_addr(IPrem);
    for(i=0;i<8;i++){adrrem.sin_zero[i]='\0';}
    if((bescrit=sendto(Sck,SeqBytes,LongSeqBytes,0,(struct sockaddr*)&adrrem,sizeof(adrrem)))==-1) return -1;
    return bescrit;
}

/* Rep a través del socket UDP d’identificador “Sck” una seqüència de     */
/* bytes que prové d'un socket remot i l’escriu a “SeqBytes*” (que té     */
/* una longitud de “LongSeqBytes” bytes).                                 */
/* Omple "IPrem*" i "portUDPrem*" amb respectivament, l'@IP i el #port    */
/* UDP del socket remot.                                                  */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* "SeqBytes*" és un vector de chars qualsevol (recordeu que en C, un     */
/* char és un enter de 8 bits) d'una longitud <= LongSeqBytes bytes       */
/* Retorna -1 si hi ha error; el nombre de bytes rebuts si tot va bé.     */
int UDP_RepDe(int Sck, char *IPrem, int *portUDPrem, char *SeqBytes, int LongSeqBytes)
{
    int i, bllegit, ladrrem;
    struct sockaddr_in adrrem;
    ladrrem = sizeof(adrrem);
    if((bllegit=recvfrom(Sck,SeqBytes,LongSeqBytes,0,(struct sockaddr*)&adrrem,&ladrrem))==-1) return -1; //LongSeqBytes podria ser sizeof(SeqBytes)
    strcpy(IPrem, inet_ntoa(adrrem.sin_addr));
    *portUDPrem = ntohs(adrrem.sin_port);
    return bllegit;
}

/* S’allibera (s’esborra) el socket UDP d’identificador “Sck”.            */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TancaSock(int Sck)
{
    return close(Sck);
}

/* Donat el socket UDP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc*” i “portUDPloc*” amb respectivament, la seva   */
/* @IP i #port UDP.                                                       */
/* "IPloc*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portUDPloc)
{
    struct sockaddr_in peeraddr;
    socklen_t peeraddrlen = sizeof(peeraddr);
    getsockname(fileno(Sck), &peeraddr, &peeraddrlen);
    inet_ntop(AF_INET, &(peeraddr.sin_addr), IPloc, INET_ADDRSTRLEN);
    *portUDPloc=ntohs(peeraddr.sin_port);
    return 0;
}

/* El socket UDP d’identificador “Sck” es connecta al socket UDP d’@IP    */
/* “IPrem” i #port UDP “portUDPrem” (si tot va bé es diu que el socket    */
/* “Sck” passa a l’estat “connectat” o establert – established –).        */
/* Recordeu que a UDP no hi ha connexions com a TCP, i que això només     */
/* vol dir que es guarda localment l’adreça “remota” i així no cal        */
/* especificar-la cada cop per enviar i rebre. Llavors quan un socket     */
/* UDP està “connectat” es pot fer servir UDP_Envia() i UDP_Rep() (a més  */
/* de les anteriors UDP_EnviaA() i UDP_RepDe()) i UDP_TrobaAdrSockRem()). */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_DemanaConnexio(int Sck, const char *IPrem, int portUDPrem)
{
    //aixo te algun sentit? whaev
    int i;
    struct sockaddr_in adr;
    adr.sin_family = AF_INET;
    adr.sin_port = htons(portUDPrem);
    adr.sin_addr.s_addr = inet_addr(IPrem);
    for (i = 0; i<8; i++) { adr.sin_zero[i] = '0'; }
    return (connect(Sck, (struct sockaddr*)&adr, sizeof(adr)));
}

/* Envia a través del socket UDP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket UDP remot amb qui està connectat.                 */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int UDP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes)
{
    return send(Sck, SeqBytes, LongSeqBytes,0);
}

/* Rep a través del socket UDP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes*” (que té una longitud de “LongSeqBytes” bytes).*/
/* "SeqBytes*" és un vector de chars qualsevol (recordeu que en C, un     */
/* char és un enter de 8 bits) d'una longitud <= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; el nombre de bytes rebuts si tot va bé.     */
int UDP_Rep(int Sck, char *SeqBytes, int LongSeqBytes)
{
    return recv(Sck, SeqBytes, LongSeqBytes, 0);
}

/* Donat el socket UDP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem*” i            */
/* “portUDPrem*” amb respectivament, la seva @IP i #port UDP.             */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TrobaAdrSockRem(int Sck, char *IPrem, int *portUDPrem)
{
    struct sockaddr_in peeraddr;
    socklen_t peeraddrlen = sizeof(peeraddr);
    getpeername(fileno(Sck), &peeraddr, &peeraddrlen);
    inet_ntop(AF_INET, &(peeraddr.sin_addr), IPrem, INET_ADDRSTRLEN);
    *portUDPrem=ntohs(peeraddr.sin_port);
}

/* Examina simultàniament durant "Temps" (en [ms] els sockets (poden ser  */
/* TCP, UDP i stdin) amb identificadors en la llista “LlistaSck” (de      */
/* longitud “LongLlistaSck” sockets) per saber si hi ha arribat alguna    */
/* cosa per ser llegida. Si Temps és -1, s'espera indefinidament fins que */
/* arribi alguna cosa.                                                    */
/* "LlistaSck" és un vector d'enters d'una longitud >= LongLlistaSck      */
/* Retorna -1 si hi ha error; retorna -2 si passa "Temps" sense que       */
/* arribi res; si arriba alguna cosa per algun dels sockets, retorna      */
/* l’identificador d’aquest socket.                                       */
int HaArribatAlgunaCosaEnTemps(const int *LlistaSck, int LongLlistaSck, int Temps)
{
    int a;
    fd_set conjunt;
    FD_ZERO(&conjunt);
    int i, descmax = 0;
    struct timeval t;
    //FD_SET(0,&conjunt); //afegim el teclat si cal
    for (i = 0; i<LongLlistaSck; i++) {
        FD_SET(LlistaSck[i], &conjunt);
        if (LlistaSck[i] > descmax) descmax = LlistaSck[i];
    }
    if (Temps==-1) if (a=(select(descmax + 1, &conjunt, NULL, NULL, NULL)) == -1) return -1;
    else{
        t.tv_sec = Temps/1000;
        t.tv_usec = (Temps%1000)*1000;
        if (a=(select(descmax + 1, &conjunt, NULL, NULL, &t)) == -1) return -1;
    }
    //si sha de comprovar tb el teclat mirar primer desde aqui
    if (a==0) return -2;
    for (i = 0; i<LongLlistaSck; i++) if (FD_ISSET(LlistaSck[i], &conjunt)) break;
    return LlistaSck[i];

}

/* Donat el nom DNS "NomDNS" obté la corresponent @IP i l'escriu a "IP*"  */
/* "NomDNS" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud qualsevol, i "IP*" és un "string" de C (vector de */
/* chars imprimibles acabat en '\0') d'una longitud màxima de 16 chars    */
/* (incloent '\0').                                                       */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé     */
int ResolDNSaIP(const char *NomDNS, char *IP)
{
    struct hostent *dadesHOST;
    struct in_addr adrHOST;
    dadesHOST = gethostbyname(NomDNS);
    if (dadesHOST==NULL) return -1;
    adrHOST.s_addr = *((unsigned long *)dadesHOST->h_addr_list[0]);
    strcpy(IP,(char*)inet_ntoa(adrHOST));
    return 1;
}

/* Crea un fitxer de "log" de nom "NomFitxLog".                           */
/* "NomFitxLog" és un "string" de C (vector de chars imprimibles acabat   */
/* en '\0') d'una longitud qualsevol.                                     */
/* Retorna -1 si hi ha error; l'identificador del fitxer creat si tot va  */
/* bé.                                                                    */
FILE* Log_CreaFitx(const char *NomFitxLog)
{
    FILE* pFile= fopen(NomFitxLog,"wb");
    fputs("KIKASSO\n",pFile);
    return pFile;
}

/* Escriu al fitxer de "log" d'identificador "FitxLog" el missatge de     */
/* "log" "MissLog".                                                       */
/* "MissLog" és un "string" de C (vector de chars imprimibles acabat      */
/* en '\0') d'una longitud qualsevol.                                     */
/* Retorna -1 si hi ha error; el nombre de caràcters del missatge de      */
/* "log" (sense el '\0') si tot va bé                                     */
int Log_Escriu(FILE* FitxLog, const char *MissLog)
{
    if(FitxLog!=NULL){
        fprintf(FitxLog,MissLog);
        puts(MissLog);
        fprintf(FitxLog,"\n");
        return strlen(MissLog);
    }
    else return -1;

}

/* Tanca el fitxer de "log" d'identificador "FitxLog".                    */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int Log_TancaFitx(FILE* FitxLog)
{
    fclose(FitxLog);
}


/* Si ho creieu convenient, feu altres funcions...                        */


int LUMI_EsperaMissatge(int socket) {
    //preguntar si inclou teclat i pantalla a LlistaSck
    fd_set conjunt;
    FD_ZERO(&conjunt);
    int i, descmax = 0;
    FD_SET(socket, &conjunt);
    if (socket > descmax) descmax = socket;
    if (select(descmax + 1, &conjunt, NULL, NULL, NULL) == -1) return -1;
    //si sha de comprovar tb el teclat mirar primer desde aqui
    FD_ISSET(socket, &conjunt);
    return socket;
}
int LUMI_Rep(int Sck, char *SeqBytes, int LongSeqBytes){
    return UDP_Rep(Sck,SeqBytes,LongSeqBytes);
}
int LUMI_RepDe(int Sck, char *IPrem, int *portUDPrem, char *SeqBytes, int LongSeqBytes){
    return UDP_RepDe(Sck,IPrem,portUDPrem,SeqBytes,LongSeqBytes);
}
