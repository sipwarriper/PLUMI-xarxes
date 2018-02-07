/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer nodelumi.c que implementa la interfície aplicació-administrador */
/* d'un node de LUMI, sobre la capa d'aplicació de LUMI (fent crides a la */
/* interfície de la capa LUMI -fitxers lumi.c i lumi.h-).                 */
/* Autors: Ismael El Habri, Lluís Trilla                                                           */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
/* Incloem "MIp2-lumi.h" per poder fer crides a la interfície de LUMI     */


#include "MIp2-lumi.h"
#include <stdio.h>
#include <string.h>


/* Definició de constants, p.e., #define MAX_LINIA 150                    */

#define MAXCLIENTS 10
#define MAXMISSATGE 200
int main(int argc,char *argv[])
{
    /* Declaració de variables, p.e., int n;                                 */
    int nClients;
    char * nomFitxer= "MIp2-nodelumi.cfg";
    char domini[30];
    char IPloc[16];
    struct Client clients[MAXCLIENTS];
    int socket;
    /* Expressions, estructures de control, crides a funcions, etc.          */
    if (LUMI_iniServ(nomFitxer,&nClients,clients, domini) == -1){
        perror("error obrir fitxer");
    }
    ///*burrar i aplicar lo de sota abans dentregar, aixo es per debbugejar*/strcpy(IPloc, "192.168.1.42");
    strcpy(IPloc, "0.0.0.0");
    socket = LUMI_crearSocket(IPloc,1714);

    int resposta;
    while(1){
        printf("CICLE\n");
        resposta=LUMI_EsperaMissatge(socket);
        if(resposta!=-1){
			printf("HE REBUT ALGO!!!!!!!!!! \n");
            char missatge[MAXMISSATGE];
            int portClient;
            char ipClient[16];
            int longitud=LUMI_RepDe(socket,ipClient,&portClient,missatge,MAXMISSATGE);
            missatge[longitud]='\0';
            switch(LUMI_ServDescxifrarRebut(missatge)){
                case REGISTRE:{
                    LUMI_ServidorReg(clients,nClients,missatge,ipClient,portClient,nomFitxer,domini,socket);
                    break;
                }
                case DESREGISTRE:{
                    LUMI_ServidorDesreg(clients,nClients,missatge,ipClient,portClient,nomFitxer,domini,socket);
                    break;
                }
                case LOCALITZACIO:{
                    LUMI_ServidorLoc(socket,missatge,longitud,domini,clients,nClients,ipClient,portClient);
                    break;
                }
                case RESPOSTALOC:{
                    LUMI_ServidorRLoc(socket,missatge,longitud,domini,clients,nClients);
                    break;
                }
                default:
                    break;
            }
        }
    }
    LUMI_finiServ();
    return 0;
}
