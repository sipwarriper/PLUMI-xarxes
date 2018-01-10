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
enum Estat {DESCONNECTAT=0, LLIURE=1, OCUPAT=2};

struct Client {
    char nom[20];
    char IP[16];
    enum Estat estat;
    int port;
};

int LUMI_crearSocket(const char *IPloc, int portUDPloc);
int LUMI_iniServ(const char* nomFitxer, int * nClients,struct Client *clients, char* domini);
int LUMI_ActualitzarFitxerRegistre(const struct Client *clients, int nClients, int fid, const char* domini);
int LUMI_connexio(int Sck, const char *IPrem, int portUDPrem);
int LUMI_Desregistre(int Sck, const char * MI);
int LUMI_Registre(int Sck, const char * MI);
int LUMI_Localitzacio(int Sck, const char *MIloc, const char *MIrem);
int LUMI_ServDescxifrarRebut(const char* missatge);
int LUMI_ServidorReg();              //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorDesreg();           //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorLoc();              //nse els parametres, mentre vagi necessitant afegiré
int LUMI_ServidorRLoc();             //nse els parametres, mentre vagi necessitant afagiré


