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


/* Definició de constants, p.e., #define MAX_LINIA 150                    */

#define MAXCLIENTS 10

int main(int argc,char *argv[])
{
    /* Declaració de variables, p.e., int n;                                 */
    int fd, nClients;
    char * nomFitxer= "MIp2-nodelumi.cfg";
    char domini[30];
    struct Client clients[MAXCLIENTS];
    /* Expressions, estructures de control, crides a funcions, etc.          */
    fd=LUMI_iniServ(nomFitxer,nClients,clients, domini);
    //aqui va el bucle infinit que fa tot
    if (fd == -1){
        perror("error obrir fitxer");
    }
    while(1){

    }
    return 0;

}
