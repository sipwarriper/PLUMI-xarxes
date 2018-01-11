/**************************************************************************/
/*                                                                        */
/* P1 - MI amb sockets TCP/IP - Part I                                    */
/* Fitxer mi.c que implementa la capa d'aplicació de MI, sobre la capa de */
/* transport TCP (fent crides a la interfície de la capa TCP -sockets-).  */
/* Autors: Lluís Trilla, Ismael El Habri 		                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include "MIp1v4-mi.h")                                           */
#include "MIp2-mi.h"
#include <string.h>
#include <stdio.h> 
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h> 
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stdio.h>

/* Definició de constants, p.e., #define MAX_LINIA 150                    */

/* Declaració de funcions internes que es fan servir en aquest fitxer     */
/* (les seves definicions es troben més avall) per així fer-les conegudes */
/* des d'aqui fins al final de fitxer.                                    */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc);
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc);
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem);
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem);
int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes);
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes);
int TCP_TancaSock(int Sck);
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc);
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem);
int HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck);
void MostraError(const char *text);
int AplicarProtocol(char tipus, int longMiss, char *info, char *missatgeFinal);
void InterpretarProtocol(char *tipus, int* longMiss, char *info, char *missatgeFinal);


/* Definicio de funcions EXTERNES, és a dir, d'aquelles que en altres     */
/* fitxers externs es faran servir.                                       */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa MI.                                    */

/* Inicia l’escolta de peticions remotes de conversa a través d’un nou    */
/* socket TCP en el #port “portTCPloc” i una @IP local qualsevol (és a    */
/* dir, crea un socket “servidor” o en estat d’escolta – listen –).       */
/* Retorna -1 si hi ha error; l’identificador del socket d’escolta de MI  */
/* creat si tot va bé.                                                    */
int MI_IniciaEscPetiRemConv(int portTCPloc,int* portFinal, char* ip)
{
	int sesc;
	char iploc[15];
	strcpy(iploc, "0.0.0.0");
	if ((sesc = TCP_CreaSockServidor(iploc, portTCPloc)) == -1) {
		perror("socket\n");
		return -1;
	}
	
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;
	getifaddrs(&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_INET) {
			sa = (struct sockaddr_in *) ifa->ifa_addr;
			if(strcmp(inet_ntoa(sa->sin_addr),"127.0.0.1"))addr = inet_ntoa(sa->sin_addr);
		}
	}
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(sesc, (struct sockaddr *)&sin, &len) == -1)
    perror("getsockname");
	else
	*portFinal=ntohs(sin.sin_port);
	strcpy(ip,addr);
	return sesc;
}

/* Escolta indefinidament fins que arriba una petició local de conversa   */
/* a través del teclat o bé una petició remota de conversa a través del   */
/* socket d’escolta de MI d’identificador “SckEscMI” (un socket           */
/* “servidor”).                                                           */
/* Retorna -1 si hi ha error; 0 si arriba una petició local; SckEscMI si  */
/* arriba una petició remota.                                             */
int MI_HaArribatPetiConv(int SckEscMI)
{
		int fds[2];
		fds[0] = 0; fds[1] = SckEscMI;
		return HaArribatAlgunaCosa(fds, 2);

}

/* Crea una conversa iniciada per una petició local que arriba a través   */
/* del teclat: crea un socket TCP “client” (en un #port i @IP local       */
/* qualsevol), a través del qual fa una petició de conversa a un procés   */
/* remot, el qual les escolta a través del socket TCP ("servidor") d'@IP  */
/* “IPrem” i #port “portTCPrem” (és a dir, crea un socket “connectat” o   */
/* en estat establert – established –). Aquest socket serà el que es farà */
/* servir durant la conversa.                                             */
/* Omple “IPloc*” i “portTCPloc*” amb, respectivament, l’@IP i el #port   */
/* TCP del socket del procés local.                                       */
/* El nickname local “NicLoc” i el nickname remot són intercanviats amb   */
/* el procés remot, i s’omple “NickRem*” amb el nickname remot. El procés */
/* local és qui inicia aquest intercanvi (és a dir, primer s’envia el     */
/* nickname local i després es rep el nickname remot).                    */
/* "IPrem" i "IPloc*" són "strings" de C (vectors de chars imprimibles    */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/* "NicLoc" i "NicRem*" són "strings" de C (vectors de chars imprimibles  */
/* acabats en '\0') d'una longitud màxima de 300 chars (incloent '\0').   */
/* Retorna -1 si hi ha error; l’identificador del socket de conversa de   */
/* MI creat si tot va bé.                                                 */
int MI_DemanaConv(const char *IPrem, int portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem)
{
	char buffer[304]; char buffer2[304];
	int scon, bytes_llegits, bytes_escrits, longMiss;
	char tipus;
	if ((scon = TCP_CreaSockClient(IPloc, 0)) == -1){
			perror("socket 2\n");
			return -1;
	}
	if (TCP_DemanaConnexio(scon, IPrem, portTCPrem) == -1) {
		perror("connect\n");
		TCP_TancaSock(scon);
		return -1;
	}
	TCP_TrobaAdrSockLoc(scon, IPloc, portTCPloc);
	bytes_llegits = AplicarProtocol('N',strlen(NicLoc),NicLoc,buffer);
	bytes_escrits = TCP_Envia(scon, buffer, bytes_llegits); 
	
	
	bytes_llegits = TCP_Rep(scon, buffer2, sizeof(buffer2));
	InterpretarProtocol(&tipus, &longMiss, NicRem, buffer2);
	NicRem[longMiss]='\0'; //convertim a string;
	return scon;
}

/* Crea una conversa iniciada per una petició remota que arriba a través  */
/* del socket d’escolta de MI d’identificador “SckEscMI” (un socket       */
/* “servidor”): accepta la petició i crea un socket (un socket            */
/* “connectat” o en estat establert – established –), que serà el que es  */
/* farà servir durant la conversa.                                        */
/* Omple “IPrem*”, “portTCPrem*”, “IPloc*” i “portTCPloc*” amb,           */
/* respectivament, l’@IP i el #port TCP del socket del procés remot i del */
/* socket del procés local.                                               */
/* El nickname local “NicLoc” i el nickname remot són intercanviats amb   */
/* el procés remot, i s’omple “NickRem*” amb el nickname remot. El procés */
/* remot és qui inicia aquest intercanvi (és a dir, primer es rep el      */
/* nickname remot i després s’envia el nickname local).                   */
/* "IPrem*" i "IPloc*" són "strings" de C (vectors de chars imprimibles   */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/* "NicLoc" i "NicRem*" són "strings" de C (vectors de chars imprimibles  */
/* acabats en '\0') d'una longitud màxima de 300 chars (incloent '\0').   */
/* Retorna -1 si hi ha error; l’identificador del socket de conversa      */
/* de MI creat si tot va bé.                                              */
int MI_AcceptaConv(int SckEscMI, char *IPrem, int *portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem)
{
	char buffer[304]; char buffer2[304];
	int scon, bytes_llegits, bytes_escrits, longMiss;
	char tipus;
	if ((scon = TCP_AcceptaConnexio(SckEscMI, IPrem, portTCPrem)) == -1) {
			perror("connect\n");
			TCP_TancaSock(scon);
			return -1;
	}
	
	bytes_llegits = TCP_Rep(scon, buffer2, sizeof(buffer2));
	InterpretarProtocol(&tipus, &longMiss, NicRem, buffer2);
	NicRem[longMiss]='\0'; //convertim a string;
	
	bytes_llegits = AplicarProtocol('N',strlen(NicLoc),NicLoc,buffer);
	bytes_escrits = TCP_Envia(scon, buffer, bytes_llegits); 
	
	return scon;
}

/* Escolta indefinidament fins que arriba una línia local de conversa a   */
/* través del teclat o bé una línia remota de conversa a través del       */
/* socket de conversa de MI d’identificador “SckConvMI” (un socket        */
/* "connectat”).                                                          */
/* Retorna -1 si hi ha error; 0 si arriba una línia local; SckConvMI si   */
/* arriba una línia remota.                                               */
int MI_HaArribatLinia(int SckConvMI)
{
	int fds[2];
	fds[0] = 0; fds[1] = SckConvMI;
	return HaArribatAlgunaCosa(fds, 2);
}

/* Envia a través del socket de conversa de MI d’identificador            */
/* “SckConvMI” (un socket “connectat”) la línia “Linia” escrita per       */
/* l’usuari local.                                                        */
/* "Linia" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0'), no conté el caràcter fi de línia ('\n') i té una longitud       */
/* màxima de 300 chars (incloent '\0').                                   */
/* Retorna -1 si hi ha error; el nombre de caràcters n de la línia        */
/* enviada (sense el ‘\0’) si tot va bé (0 <= n <= 299).                  */
int MI_EnviaLinia(int SckConvMI, const char *Linia)
{
	char buffer[304];
	int bytes_llegits = AplicarProtocol('L',strlen(Linia),Linia,buffer);
	int bytes_escrits = TCP_Envia(SckConvMI, buffer, bytes_llegits); //mirarse loffset aquest de merda!
	if (bytes_escrits == -1) return -1;
	else return strlen(Linia); //Després miralri el sentit, NO ES CULPA MEVA!
}

/* Rep a través del socket de conversa de MI d’identificador “SckConvMI”  */
/* (un socket “connectat”) una línia escrita per l’usuari remot, amb la   */
/* qual omple “Linia”, o bé detecta l’acabament de la conversa per part   */
/* de l’usuari remot.                                                     */
/* "Linia*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0'), no conté el caràcter fi de línia ('\n') i té una longitud       */
/* màxima de 300 chars (incloent '\0').                                   */
/* Retorna -1 si hi ha error; -2 si l’usuari remot acaba la conversa; el  */
/* nombre de caràcters n de la línia rebuda (sense el ‘\0’) si tot va bé  */
/* (0 <= n <= 299).                                                       */
int MI_RepLinia(int SckConvMI, char *Linia)
{
	char buffer[304];
	int longMiss;
	char tipus;
	int bytes_llegits = TCP_Rep(SckConvMI, buffer, sizeof(buffer));
	InterpretarProtocol(&tipus, &longMiss, Linia, buffer);
	Linia[longMiss]='\0'; //convertim a string;
	if (bytes_llegits == -1) return -1;
	else if(bytes_llegits == 0) return -2;
	else return longMiss;
}

/* Acaba la conversa associada al socket de conversa de MI                */
/* d’identificador “SckConvMI” (un socket “connectat”).                   */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int MI_AcabaConv(int SckConvMI)
{
	return close(SckConvMI);
}

/* Acaba l’escolta de peticions remotes de conversa que arriben a través  */
/* del socket d’escolta de MI d’identificador “SckEscMI” (un socket       */
/* “servidor”).                                                           */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int MI_AcabaEscPetiRemConv(int SckEscMI)
{
	return close(SckEscMI);
}

/* Definicio de funcions                                                  */

/* Crea un socket TCP “client” a l’@IP “IPloc” i #port TCP “portTCPloc”   */
/* (si “IPloc” és “0.0.0.0” i/o “portTCPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port TCP, respectivament).      */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc) {
	if (IPloc == NULL && portTCPloc == 0) return socket(AF_INET, SOCK_STREAM, 0); //per no cambiar codi del us al main (lol)
	else {
		int sesc, i;
		struct sockaddr_in adr;
		if ((sesc = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
		adr.sin_family = AF_INET;
		adr.sin_port = htons(portTCPloc);
		adr.sin_addr.s_addr = inet_addr(IPloc);
		for (i = 0; i<8; i++) { adr.sin_zero[i] = '0'; }
		if ((bind(sesc, (struct sockaddr*)&adr, sizeof(adr))) == -1) return -1;
		return sesc;

	}
}

/* Crea un socket TCP “servidor” (o en estat d’escolta – listen –) a      */
/* l’@IP “IPloc” i #port TCP “portTCPloc” (si “IPloc” és “0.0.0.0” i/o    */
/* “portTCPloc” és 0 es fa una assignació implícita de l’@IP i/o del      */
/* #port TCP, respectivament).                                            */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc) {
	int sesc, i;
	struct sockaddr_in adrloc;
	if ((sesc = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
	adrloc.sin_family = AF_INET;
	adrloc.sin_port = htons(portTCPloc);
	adrloc.sin_addr.s_addr = inet_addr(IPloc);
	for (i = 0; i<8; i++) { adrloc.sin_zero[i] = '0'; }
	if ((bind(sesc, (struct sockaddr*)&adrloc, sizeof(adrloc))) == -1) return -1;
	if ((listen(sesc, 3)) == -1) return -1;
	return sesc;
}

/* El socket TCP “client” d’identificador “Sck” demana una connexió al    */
/* socket TCP “servidor” d’@IP “IPrem” i #port TCP “portTCPrem” (si tot   */
/* va bé es diu que el socket “Sck” passa a l’estat “connectat” o         */
/* establert – established –).                                            */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem) {
	int i;
	struct sockaddr_in adr;
	adr.sin_family = AF_INET;
	adr.sin_port = htons(portTCPrem);
	adr.sin_addr.s_addr = inet_addr(IPrem);
	for (i = 0; i<8; i++) { adr.sin_zero[i] = '0'; }
	return (connect(Sck, (struct sockaddr*)&adr, sizeof(adr)));
}

/* El socket TCP “servidor” d’identificador “Sck” accepta fer una         */
/* connexió amb un socket TCP “client” remot, i crea un “nou” socket,     */
/* que és el que es farà servir per enviar i rebre dades a través de la   */
/* connexió (es diu que aquest nou socket es troba en l’estat “connectat” */
/* o establert – established –; el nou socket té la mateixa adreça que    */
/* “Sck”).                                                                */
/* Omple “IPrem*” i “portTCPrem*” amb respectivament, l’@IP i el #port    */
/* TCP del socket remot amb qui s’ha establert la connexió.               */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; l’identificador del socket connectat creat  */
/* si tot va bé.                                                          */
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem) {
	struct sockaddr_in adr;
	int scon, long_adr = sizeof(adr);
	scon = accept(Sck, (struct sockaddr*)&adr, &long_adr);
	//*IPrem = inet_ntoa(adr.sin_addr);
	//*portTCPrem = ntohs(adr.sin_port);
	TCP_TrobaAdrSockRem(scon, IPrem, portTCPrem);
	return scon;
}

/* Envia a través del socket TCP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket TCP remot amb qui està connectat.                 */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes) {
	return write(Sck, SeqBytes, LongSeqBytes);
}

/* Rep a través del socket TCP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes*” (que té una longitud de “LongSeqBytes” bytes),*/
/* o bé detecta que la connexió amb el socket remot ha estat tancada.     */
/* "SeqBytes*" és un vector de chars qualsevol (recordeu que en C, un     */
/* char és un enter de 8 bits) d'una longitud <= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; 0 si la connexió està tancada; el nombre de */
/* bytes rebuts si tot va bé.                                             */
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes) {
	return read(Sck, SeqBytes, LongSeqBytes);
}

/* S’allibera (s’esborra) el socket TCP d’identificador “Sck”; si “Sck”   */
/* està connectat es tanca la connexió TCP que té establerta.             */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TancaSock(int Sck) {
	return close(Sck);
}

/* Donat el socket TCP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc*” i “portTCPloc*” amb respectivament, la seva   */
/* @IP i #port TCP.                                                       */
/* "IPloc*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc) {
	struct sockaddr_in adr;
	int long_adr = sizeof(adr);
	if (getsockname(Sck, (struct sockaddr*)&adr, &long_adr) == -1) return -1;
	*IPloc = inet_ntoa(adr.sin_addr);
	*portTCPloc = ntohs(adr.sin_port);
	return 1;
}

/* Donat el socket TCP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem*” i            */
/* “portTCPrem*” amb respectivament, la seva @IP i #port TCP.             */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem) {
	struct sockaddr_in adr;
	int long_adr = sizeof(adr);
	if ((getpeername(Sck, (struct sockaddr*)&adr, &long_adr)) == -1) return -1;
	strcpy(IPrem, inet_ntoa(adr.sin_addr));
	//*IPrem = inet_ntoa(adr.sin_addr);
	*portTCPrem = ntohs(adr.sin_port);
	return 1;
}

/* Examina simultàniament i sense límit de temps (una espera indefinida)  */
/* els sockets (poden ser TCP, UDP i stdin) amb identificadors en la      */
/* llista “LlistaSck” (de longitud “LongLlistaSck” sockets) per saber si  */
/* hi ha arribat alguna cosa per ser llegida.                             */
/* "LlistaSck" és un vector d'enters d'una longitud >= LongLlistaSck      */
/* Retorna -1 si hi ha error; si arriba alguna cosa per algun dels        */
/* sockets, retorna l’identificador d’aquest socket.                      */
int HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck) {
	//preguntar si inclou teclat i pantalla a LlistaSck
	fd_set conjunt;
	FD_ZERO(&conjunt);
	int i, descmax = 0;
	//FD_SET(0,&conjunt); //afegim el teclat si cal 
	for (i = 0; i<LongLlistaSck; i++) {
		FD_SET(LlistaSck[i], &conjunt);
		if (LlistaSck[i] > descmax) descmax = LlistaSck[i];
	}
	if (select(descmax + 1, &conjunt, NULL, NULL, NULL) == -1) return -1;
	//si sha de comprovar tb el teclat mirar primer desde aqui
	for (i = 0; i<LongLlistaSck; i++) if (FD_ISSET(LlistaSck[i], &conjunt)) break;
	return LlistaSck[i];


}

/* Escriu un missatge de text al flux d’error estàndard stderr, format    */
/* pel text “Text”, un “:”, un espai en blanc, un text que descriu el     */
/* darrer error produït en una crida de sockets, i un canvi de línia.     */
void MostraError(const char *text) {
	fprintf(stderr, "%s: %s\n", text, strerror(errno));
}


/* Si ho creieu convenient, feu altres funcions...                        */


int AplicarProtocol(char tipus, int longMiss, char*info, char* missatgeFinal){
	return sprintf(missatgeFinal,"%c%.3d%s",tipus, longMiss, info);
}


void InterpretarProtocol(char* tipus, int* longMiss, char* info, char* missatge){
	char temp[3];
	int i;
	*tipus = missatge[0];
	for (i=1;i<4;i++) temp[i-1] = missatge[i];
	*longMiss = strtol(temp, (char**)NULL, 10);
	//strcpy(info, missatge+4);
	for(i=4;i<*longMiss+4;i++) info[i-4] = missatge[i];
}
int MI_Rep(int Sck, char *SeqBytes, int LongSeqBytes){
	return TCP_Rep(Sck,SeqBytes,LongSeqBytes);
}

