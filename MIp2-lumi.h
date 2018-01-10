/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de lumi.c                                             */
/*                                                                        */
/* Autors: Ismael El Habri, Lluís Trilla                                                           */
/*                                                                        */
/**************************************************************************/


/* Declaració de funcions externes de lumi.c, és a dir, d'aquelles que es */
/* faran servir en un altre fitxer extern, p.e., MIp2-p2p.c,              */
/* MIp2-nodelumi.c, o MIp2-agelumic.c. El fitxer extern farà un #include  */
/* del fitxer .h a l'inici, i així les funcions seran conegudes en ell.   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI.                                  */
/* Les funcions externes les heu de dissenyar vosaltres...                */


int LUMI_crearSocket(const char *IPloc, int portUDPloc);
int LUMI_iniServ(const char* nomFitxer); //probablament necessiti nom fitxer i ja...
int LUMI_ActualitzarFitxerRegistre(struct Client * clients, int fid);
int LUMI_connexio(int Sck, const char *IPrem, int portUDPrem);
int LUMI_Desregistre();              //nse els parametres, mentre vagi necessitant afegiré
int LUMI_Registre();                 //nse els parametres, mentre vagi necessitant afegiré
int LUMI_Localitzacio();             //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorReg();              //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorDesreg();           //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorLoc();              //nse els parametres, mentre vagi necessitant afegiré


struct Client {
    char nom[20];
    char IP[16];
    int port;
};