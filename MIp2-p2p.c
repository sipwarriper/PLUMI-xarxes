/**************************************************************************/
/*                                                                        */
/* P1 - MI amb sockets TCP/IP - Part I                                    */
/* Fitxer p2p.c que implementa la interfície aplicació-usuari de          */
/* l'aplicació de MI, sobre la capa d'aplicació de MI (fent crides a la   */
/* interfície de la capa MI -fitxers mi.c i mi.h-).                       */
/* Autors: Lluís Trilla, Ismael El Habri                                  */
/*                                                                        */
/**************************************************************************/


/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
/* Incloem "MIp1v4-mi.h" per poder fer crides a la interfície de MI       */
#include "MIp2-mi.h"
#include "MIp2-lumi.h"
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


int main(int argc,char *argv[])
{
	int portloc, sesc, sck_rep, portrem, scon, bytes_llegits, bytes_escrits,sckRep_Conv;
	char iprem[16];
	char iploc[16];
    char ipServ[16];
    char usuariMIloc[20];
	char nick[16];
	char nickRem[16];
	char linia[300];
	char buffer[304];
	int opcio=1;
    int sckUDP, socUDP;
 /* Declaració de variables, p.e., int n;                                 */
	strcpy(iploc, "0.0.0.0");
	char ipMostrar[16];
	int portMostrar;
	if ((sesc = MI_IniciaEscPetiRemConv(0,&portMostrar, ipMostrar)) == -1) {
		perror("socket\n");
		exit(-1);
	}
    printf("IP@port: %s@%u\n", ipMostrar,portMostrar);
    printf("Entra el teu usuari MI");
    int bytesllegitsr=read(0,usuariMIloc,20);

    sckUDP=LUMI_crearSocket(iploc,0);
    socUDP=LUMI_connexio(sckUDP,ipServ,1714);

	while(opcio!=0){
		printf("entra 0 per sortir, o un qualsevol per iniciar conversació, o espera connexió:\n");
		sck_rep = MI_HaArribatPetiConv(sesc);
	 /* Expressions, estructures de control, crides a funcions, etc.          */
		if (sck_rep == 0) { // TECLAT
			scanf("%i",&opcio);
			if (opcio==0) break;
			printf("Entra IP a la que et vols connectar:\n");
			bytes_llegits = MI_Rep(0, iprem, sizeof(iprem));
			printf("Entra el port al que et vols connectar:\n");
			scanf("%i", &portrem);
			printf("Entrar nick \n");
			bytes_llegits = MI_Rep(0,nick,sizeof(nick));
			nick[bytes_llegits-1]='\0';
			if((scon = MI_DemanaConv(iprem, portrem, iploc,0, nick, nickRem))==-1){
				printf("error demanaConv\n");
				exit(-1);
			}
		}
		else { // SOCKET
			printf("Entrar nick \n");
			bytes_llegits = MI_Rep(0,nick,sizeof(nick));
			nick[bytes_llegits-1]='\0';
			if ((scon=MI_AcceptaConv(sesc, iprem, &portrem, iploc, portloc, nick, nickRem))==-1){
				printf("error acceptaConv\n");
				exit(-1);
			}
		}
		iprem[15]='\0';
		ipMostrar[15]='\0';
		//printf("Local IP@port: %s@%u\n", ipMostrar,portMostrar);
		printf("Remot IP@port: %s@%u\n", iprem,portrem);
		printf("conversi\n");
		do{
			sckRep_Conv = MI_HaArribatLinia(scon);
			if (sckRep_Conv==0){ //teclat
				bytes_llegits = read(0,linia,sizeof(linia));
				if(linia[0] == ':') break;
				linia[bytes_llegits-1]='\0'; //perpoder fer strlen, si passa salt de linia, fer -1
				bytes_escrits = MI_EnviaLinia(scon, linia);
			}else{ //socket
				bytes_llegits = MI_RepLinia(sckRep_Conv, linia);
				if(bytes_llegits!=-2) printf("%s: %s\n", nickRem, linia);
			}

		}while(bytes_llegits!=-2);
		MI_AcabaConv(scon);
	}

	MI_AcabaEscPetiRemConv(sesc); //Tencar escolta al tencar bucle


	return 0;
 }
