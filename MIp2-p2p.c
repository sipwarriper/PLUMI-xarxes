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
	int portloc, sesc, sck_rep, portrem, scon, bytes_llegits, sckRep_Conv;
	char iprem[16];
	char iploc[16];
    char ipServ[16];
    char usuariMIloc[20];
    char usuariMIrem[20];
	char nick[16];
	char nickRem[16];
	char linia[300];
	int opcio=1;
    int sckUDP, socUDP;
 /* Declaració de variables, p.e., int n;                                 */
	///*burrar i aplicar lo de sota abans dentregar, aixo es per debbugejar*/strcpy(iploc, "192.168.1.42");
	strcpy(iploc, "0.0.0.0");
	if ((sesc = MI_IniciaEscPetiRemConv(0,&portloc, iploc)) == -1) {
		perror("socket\n");
		exit(-1);
	}
    printf("IP@port: %s@%u\n", iploc,portloc);
    printf("Entra el teu usuari MI\n");
	scanf("%s",usuariMIloc);
	char nomDns[20];
	strcpy(nomDns,usuariMIloc);
	char *novaStrink;
	novaStrink=strtok(nomDns,"@");
	novaStrink=strtok(NULL,"@");
	if (ResolDNSaIP(novaStrink,ipServ)==-1){
		perror("error al resoldre DNS\n");
		exit(1);
	}
	//printf("%s\n",ipServ);
    sckUDP=LUMI_crearSocket(iploc,0);
    socUDP=LUMI_connexio(sckUDP,ipServ,1714);
	//printf("%d\n",sckUDP);
	if(socUDP==-1||sckUDP==-1){
		printf("Error amb els sockets udp");
		return -1;
	}
    //cal fer el registre
    int opReg;
    if((opReg=LUMI_Registre(sckUDP,usuariMIloc))==-2){
        perror("servidor LUMI apagat\n");
        exit(-1);
    }
    else if(opReg==-1) {
        perror("error registre\n");
        exit(-1);
    }
	else if(opReg==1){
		perror("usuari inexistent\n");
		exit(-1);
	}
	else if(opReg==2){
		perror("error format incorrecte\n");
		exit(-1);
	}
    int llistaSockets[] = {sesc,socUDP};
	while(opcio!=0){
		printf("entra 0 per sortir, o un qualsevol per iniciar conversació, o espera connexió:\n");
		sck_rep = MI_HaArribatPetiConv(llistaSockets,2);
	 /* Expressions, estructures de control, crides a funcions, etc.          */
		if (sck_rep == 0) { // TECLAT
			scanf("%i",&opcio);
			if (opcio==0) break;
			printf("Entra la adreça MI a la que et vols conectar:\n");
			scanf("%s",usuariMIrem);
			int x = LUMI_Localitzacio(sckUDP,usuariMIloc,usuariMIrem,iprem, &portrem);
			if (x==-1) perror("error Localitzacio\n");
			else if(x==1) perror("usuari offline\n");
			else if(x==2) perror("usuari innexistent\n");
			else if(x==3) perror("usuari ocupat\n");
            printf("Entrar nick \n");
            bytes_llegits = MI_Rep(0,nick,sizeof(nick));
            nick[bytes_llegits-1]='\0';
			if((scon = MI_DemanaConv(iprem, portrem, iploc,0, nick, nickRem))==-1){
				printf("error demanaConv\n");
				exit(-1);
			}
		}
		else if(sck_rep == sesc){ // SOCKET
			printf("Entrar nick \n");
			bytes_llegits = MI_Rep(0,nick,sizeof(nick));
			nick[bytes_llegits-1]='\0';
			if ((scon=MI_AcceptaConv(sesc, iprem, &portrem, iploc, portloc, nick, nickRem))==-1){
				printf("error acceptaConv\n");
				exit(-1);
			}
		}
		else if(sck_rep==socUDP){
			puts("REBEM PETICIO DE LOCALITZACIO");
            LUMI_RLocalitzacio(sckUDP,usuariMIloc,iploc,portloc,0);
		}
		else{
			printf("LA HAS CAGAO LOKO \n");
		}
		iprem[15]='\0';
		iploc[15]='\0';
		printf("Remot IP@port: %s@%u\n", iprem,portrem);
		printf("conversi\n");
		do{
			sckRep_Conv = MI_HaArribatLinia(scon);
			if (sckRep_Conv==0){ //teclat
				bytes_llegits = read(0,linia,sizeof(linia));
				if(linia[0] == ':') break;
				linia[bytes_llegits-1]='\0'; //perpoder fer strlen, si passa salt de linia, fer -1
				MI_EnviaLinia(scon, linia);
			}else{ //socket
				bytes_llegits = MI_RepLinia(sckRep_Conv, linia);
				if(bytes_llegits!=-2) printf("%s: %s\n", nickRem, linia);
			}

		}while(bytes_llegits!=-2);
		MI_AcabaConv(scon);
	}

	MI_AcabaEscPetiRemConv(sesc); //Tencar escolta al tencar bucle

    if((opReg=LUMI_Desregistre(sckUDP,usuariMIloc))==-2){
        perror("servidor LUMI apagat\n");
        exit(-1);
    }
    else if(opReg==-1) {
        perror("error registre\n");
        exit(-1);
    }

	return 0;
 }
