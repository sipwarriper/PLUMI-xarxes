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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "MIp2-lumi.h"
#include <fcntl.h>
#include <stdlib.h>

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
int ResolDNSaIP(const char *NomDNS, char *IP);
int Log_CreaFitx(const char *NomFitxLog);
int Log_Escriu(int FitxLog, const char *MissLog);
int Log_TancaFitx(int FitxLog);


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
    return UDP_CreaSock(IPloc, portUDPloc);
}

/* Funció que inicialitza el servidor LUMI amb la informació trobada al fitxer de configuraicó anomenat "nomFItxer"
 * i emplena "client" amb els clients trobats i "domini" amb el domini propi.
 * Retorna -1 si hi ha error amb el fitxer i el codi identificatiu d'aquest en cas contrari
 * */
int LUMI_iniServ(const char* nomFitxer, struct Client *client, char* domini){

    int fid = open(nomFitxer,O_CREAT|O_TRUNC);
    int readB;
    char buffer[200];
    char *next;
    char *current;
    if((readB=read(fid, buffer, 200))>0){
        strncpy(buffer, domini,readB);
        int i=0;
        while ((readB=read(fid, buffer, 200))>0){
           current=buffer;
           current[readB]='\0';
           int j=0;
           while ((next = strchr(current, ' ')) != NULL) {
               int cont=0;
               switch(j){
                   case 0: //cas del nom del client
                       while (current[cont]!=next[0]) cont++;
                       strncpy(client[i].nom,current,cont);
                       break;
                   case 1: //cas del estat del client
                       //opcio1
                       client[i].estat=strtol(current, (char**)NULL,10);
                       break;
                   case 2: //cas de la ip del client
                       strncpy(client[j].IP,current,15);
                       client[j].IP[15]='\0';
                       break;
               }
               current = next + 1;
               j++;
           }
           //tractament cas current no tractat (lultim)
           client[i].port = strtol(current, (char**)NULL, 10);
           i++;
       }
       return fid;
    }
    else return -1;

}

int LUMI_ActualitzarFitxerRegistre(const struct Client *clients, int fid){

}


int LUMI_connexio(int Sck, const char *IPrem, int portUDPrem){
    return UDP_DemanaConnexio(Sck,IPrem, portUDPrem);
}


int LUMI_Desregistre();              //nse els parametres, mentre vagi necessitant afegiré
int LUMI_Registre();                 //nse els parametres, mentre vagi necessitant afegiré
int LUMI_Localitzacio();             //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorReg();              //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorDesreg();           //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorLoc();              //nse els parametres, mentre vagi necessitant afegiré


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
    *IPrem= inet_ntoa(adrrem.sin_addr);
    *portUDPrem = ntohs(adrrem.sin_port);
}

/* S’allibera (s’esborra) el socket UDP d’identificador “Sck”.            */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TancaSock(int Sck)
{
    close(Sck);
}

/* Donat el socket UDP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc*” i “portUDPloc*” amb respectivament, la seva   */
/* @IP i #port UDP.                                                       */
/* "IPloc*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portUDPloc)
{
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
    fd_set conjunt;
    FD_ZERO(&conjunt);
    int i, descmax = 0;
    struct timeval *t;
    //FD_SET(0,&conjunt); //afegim el teclat si cal
    for (i = 0; i<LongLlistaSck; i++) {
        FD_SET(LlistaSck[i], &conjunt);
        if (LlistaSck[i] > descmax) descmax = LlistaSck[i];
    }
    if (Temps==-1) t = NULL;
    else{
        t->tv_usec=Temps*1000;
    }
    if (select(descmax + 1, &conjunt, NULL, NULL, t) == -1) return -1;
    //si sha de comprovar tb el teclat mirar primer desde aqui
    for (i = 0; i<LongLlistaSck; i++) if (FD_ISSET(LlistaSck[i], &conjunt)) break;
    if (i<LongLlistaSck) return LlistaSck[i];
    else return -2;
}

/* Donat el nom DNS "NomDNS" obté la corresponent @IP i l'escriu a "IP*"  */
/* "NomDNS" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud qualsevol, i "IP*" és un "string" de C (vector de */
/* chars imprimibles acabat en '\0') d'una longitud màxima de 16 chars    */
/* (incloent '\0').                                                       */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé     */
int ResolDNSaIP(const char *NomDNS, char *IP)
{
    struct hostent *dadesHOST; //copipaste del pdf
    struct in_addr adrHOST;    //copipaste del pdf
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
int Log_CreaFitx(const char *NomFitxLog)
{

}

/* Escriu al fitxer de "log" d'identificador "FitxLog" el missatge de     */
/* "log" "MissLog".                                                       */
/* "MissLog" és un "string" de C (vector de chars imprimibles acabat      */
/* en '\0') d'una longitud qualsevol.                                     */
/* Retorna -1 si hi ha error; el nombre de caràcters del missatge de      */
/* "log" (sense el '\0') si tot va bé                                     */
int Log_Escriu(int FitxLog, const char *MissLog)
{

}

/* Tanca el fitxer de "log" d'identificador "FitxLog".                    */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int Log_TancaFitx(int FitxLog)
{

}


/* Si ho creieu convenient, feu altres funcions...                        */


