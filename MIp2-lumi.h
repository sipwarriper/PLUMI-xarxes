/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de lumi.c                                             */
/*                                                                        */
/* Autors: Ismael El Habri, Lluís Trilla                                                           */
/*                                                                        */
/**************************************************************************/
#ifndef LMAO
#define LMAO

/* Declaració de funcions externes de lumi.c, és a dir, d'aquelles que es */
/* faran servir en un altre fitxer extern, p.e., MIp2-p2p.c,              */
/* MIp2-nodelumi.c, o MIp2-agelumic.c. El fitxer extern farà un #include  */
/* del fitxer .h a l'inici, i així les funcions seran conegudes en ell.   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI.                                  */
/* Les funcions externes les heu de dissenyar vosaltres...                */
enum Estat {DESCONNECTAT=0, LLIURE=1, OCUPAT=2};
enum TipusMissatge {REGISTRE,DESREGISTRE,LOCALITZACIO,ACK,RESPOSTALOC};

struct Client {
    char nom[50];
    char IP[16];
    enum Estat estat;
    int port;
};

int ResolDNSaIP(const char *NomDNS, char *IP);
int LUMI_crearSocket(const char *IPloc, int portUDPloc);
int LUMI_iniServ(const char* nomFitxer, int * nClients,struct Client *clients, char* domini);
int LUMI_finiServ();
int LUMI_ActualitzarFitxerRegistre(const struct Client *clients, int nClients, const char *nomFitxer, const char* domini);
int LUMI_connexio(int Sck, const char *IPrem, int portUDPrem);
int LUMI_Desregistre(int Sck, const char * MI);
int LUMI_Registre(int Sck, const char * MI);
int LUMI_Localitzacio(int Sck, const char *MIloc, const char *MIrem, char * IP, int * portTCP);
int LUMI_RLocalitzacio(int Sck, const char* IP, int portTCP, int estat);
int LUMI_RLocOcupat(int Sck);
int LUMI_ServDescxifrarRebut(const char* missatge);
int LUMI_ServidorReg(struct Client *clients, int nClients,const char *Entrada,const char *IP, int port, const char *nomFitxer, const char* domini, int socket);
int LUMI_ServidorDesreg( struct Client *clients, int nClients,const char *Entrada,const char *IP, int port, const char *nomFitxer, const char* domini, int socket);
int LUMI_ServidorLoc(int Sck, char * missatge, int longMissatge, const char* dominiloc, struct Client *clients, int nClients, const char* IPrem, int portRem);
int LUMI_ServidorRLoc(int Sck, char * missatge, int longMissatge, const char* dominiloc, struct Client *clients, int nClients);

int LUMI_EsperaMissatge(int sock);
int LUMI_Rep(int Sck, char *SeqBytes, int LongSeqBytes);
int LUMI_RepDe(int Sck, char *IPrem, int *portUDPrem, char *SeqBytes, int LongSeqBytes);
#endif
