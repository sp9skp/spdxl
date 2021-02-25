/*
 * spDXL toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 * Modified by SP9SKP, SQ2DK
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define DBS_SIZE 100
#define SEND_INT 2

#define BUFLEN 256
#define PORT 9930  //pub
//#define PORT 9937  //prv


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define sondemod_C_
#ifndef soundctl_H_
#include "soundctl.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#include <fcntl.h>
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef gpspos_H_
#include "gpspos.h"
#endif
#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* decode RS92, RS41, SRS-C34 and DFM06 Radiosonde by OE5DXL */
/*FROM rsc IMPORT initrsc, decodersc; */
#define sondemod_CONTEXTLIFE 3600
/* seconds till forget context after last heared */

#define sondemod_DAYSEC 86400

#define sondemod_GPSTIMECORR 18

#define sondemod_ADCBUFLEN 4096

#define sondemod_BAUDSAMP 65536

#define sondemod_PLLSHIFT 1024

#define sondemod_AFIRLEN 512

#define sondemod_AOVERSAMP 16
/*16*/

#define sondemod_ASYNBITS 10

static char sondemod_CALIBFRAME = 'e';

static char sondemod_GPSFRAME = 'g';

static char sondemod_AUXILLARY = 'h';

static char sondemod_DATAFRAME = 'i';

static char sondemod_EMPTYAUX = '\003';

#define sondemod_PI 3.1415926535898

#define sondemod_RAD 1.7453292519943E-2

#define sondemod_EARTH 6370.0

#define sondemod_MYLAT 8.4214719496019E-1
/* only for show sat elevations if no pos decode */

#define sondemod_MYLONG 2.2755602787502E-1

#define sondemod_NEWALMAGE 30
/* every s reread almanach */

#define sondemod_FASTALM 4
/* reread almanach if old */

uint32_t save2csv, disSKP=0,saveLog=0;

typedef char FILENAME[1024];

typedef char OBJNAME[9];

typedef char CALLSSID[11];

enum CHANNELS {sondemod_LEFT, sondemod_RIGHT};


struct CHAN;


struct CHAN {
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[520];
};

struct CONTEXTR9;


struct CONTEXTR9 {
   char calibdata[512];
   uint32_t calibok;
   char mesok;
   char posok;
   char framesent;
   double lat;
   double long0;
   double heig;
   double speed;
   double dir;
   double climb;
   double lastlat;
   double laslong;
   double lastalt;
   double lastspeed;
   double lastdir;
   double lastclb;
   float hrmsc;
   float vrmsc;
   double hp;
   double hyg;
   double temp;
   double ozontemp;
   double ozon;
   uint32_t goodsats;
   uint32_t timems;
   uint32_t framenum;
//skp
   unsigned char aux;

};

struct CONTEXTC34;

typedef struct CONTEXTC34 * pCONTEXTC34;


struct CONTEXTC34 {
   pCONTEXTC34 next;
   OBJNAME name;
   double clmb;
   double lat;
   double lon;
   double lat1;
   double lon1;
   double latv1;
   double lonv1;
   double alt;
   double vlon;
   double vlat;
   double speed;
   double dir;
   double temp;
   uint32_t lastsent;
   uint32_t gpstime;
   uint32_t tgpstime;
   uint32_t tlat;
   uint32_t tlon;
   uint32_t tlat1;
   uint32_t tlon1;
   uint32_t tlatv1;
   uint32_t tlonv1;
   uint32_t talt;
   uint32_t tspeed;
   uint32_t tdir;
   uint32_t ttemp;
   uint32_t tused;

};

struct CONTEXTDFM6;

typedef struct CONTEXTDFM6 * pCONTEXTDFM6;


struct CONTEXTDFM6 {
   pCONTEXTDFM6 next;
   OBJNAME name;
   double clmb;
   double lat;
   double lon;
   double lat1;
   double lon1;
   double alt;
   double speed;
   double dir;
   uint32_t gpstime;
   uint32_t lastsent;
   uint32_t tlat;
   uint32_t tlon;
   uint32_t tlat1;
   uint32_t tlon1;
   uint32_t talt;
   uint32_t tspeed;
   uint32_t tdir;
   uint32_t actrt;
   uint32_t tused;
   char d9;
   char posok;
   uint32_t poserr; /* count down after position jump */
   char framesent;
   unsigned char aux;
   float mhz0;

};

struct CONTEXTR4;

typedef struct CONTEXTR4 * pCONTEXTR4;
typedef struct CONTEXTR4 * pCONTEXTR5;


struct CONTEXTR4 {
   pCONTEXTR4 next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   uint32_t gpssecond;
   uint32_t framenum;
   uint32_t tused;
   double hp;
   uint32_t ozonInstType;
   uint32_t ozonInstNum;
   double ozonTemp;
   double ozonuA;
   double ozonBatVolt;
   double ozonPumpMA;
   double ozonExtVolt;
   uint32_t burstKill;
//skp
   unsigned char aux;
   unsigned long swVersion;
   unsigned long hwvwersion;
   char serialNumber[6];
   unsigned long killTimer;
   double ozonval;
   float vbat;
};

struct CONTEXTM10;

typedef struct CONTEXTM10 * pCONTEXTM10;


struct CONTEXTM10 {
   pCONTEXTM10 next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   uint32_t gpssecond;
   uint32_t framenum;
   uint32_t tused;
};

struct CONTEXTM20;

typedef struct CONTEXTM20 * pCONTEXTM20;

struct CONTEXTM20 {
   pCONTEXTM10 next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   uint32_t gpssecond;
   uint32_t framenum;
   uint32_t tused;
};


struct CONTEXTMP3;

typedef struct CONTEXTMP3 * pCONTEXTMP3;

struct CONTEXTMP3 {
   pCONTEXTMP3 next;
   OBJNAME name;
   char posok;
   char framesent;
   float freq;
   //uint32_t gpssecond;
   uint32_t prevgpstime;
   //uint32_t tused;
   uint32_t prevfrno;
   double prevalt,prevlon,prevlat;
   double vbat;
   float snd;
};


struct CONTEXTPS;

typedef struct CONTEXTPS * pCONTEXTPS;

struct CONTEXTPS {
   pCONTEXTPS next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   uint32_t gpssecond;
   uint32_t framenum;
   uint32_t tused;
   uint32_t lastframe;
   
};
static OBJNAME pilname;    // <-added for pilot sonde

struct DBS{

    char name[20];
    double lat;
    double lon;
    double alt;
    double speed;
    double climb;
    float dir;
    double frq;
    time_t time;
    time_t sendtime;

    unsigned int frameno;
    int typ;
    char bk;
    unsigned int swv;
    double ozon;
    char aux;
    double press;
    float vbat,t1,t2,hum;
} dBs[DBS_SIZE];

static FILENAME semfile;

static FILENAME yumafile;

static FILENAME rinexfile;

static uint32_t sendquick; /* 0 send if full calibrated, 1 with mhz, 2 always */

static uint32_t almread;

/* time last almanach read */
static uint32_t almrequest;

/* seconds rinex age to request new */
static uint32_t almage;

static uint32_t systime;

static FILENAME soundfn;

static struct CHAN chan[2];

static gpspos_SATS lastsat;

static float coeff[256];

static float mhz;

static OBJNAME objname;

static int32_t rxsock;

static uint32_t maxalmage;

static uint32_t lastip;

static uint32_t lastport;

static char mycall[100];

static struct CONTEXTR9 contextr9;

static pCONTEXTC34 pcontextc;

static pCONTEXTDFM6 pcontextdfm6;

static pCONTEXTR4 pcontextr4;
static pCONTEXTPS pcontextps;
static pCONTEXTM10 pcontextm10;
static pCONTEXTM20 pcontextm20;
static pCONTEXTMP3 pcontextmp3;

// SKP

static char dbAddr[100];
static char dbPass[20];
time_t oldMTime;

#include <openssl/md5.h>
#include <stdio.h>
#include <time.h>

#include<sys/socket.h>
#include<netdb.h> //hostent
#include<arpa/inet.h>

struct sockaddr_in serv_addr;
int sockfd, i, slen=sizeof(serv_addr);
char UDPbuf[BUFLEN];


int h2ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ";");
            tok && *tok;
            tok = strtok(NULL, ";\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}





int save_csv() {
    FILE* stream;
    stream = fopen("/tmp/sonde.csv", "w+");
    if (stream){
	for (i=0;i<DBS_SIZE;i++) if (dBs[i].name[0]) fprintf(stream,"%s;%0.5f;%0.5f;%0.0f;%0.2f;%0.2f;%0.0f;%0.3f;%lu\n",dBs[i].name,dBs[i].lat,dBs[i].lon,dBs[i].alt,dBs[i].speed,dBs[i].climb,dBs[i].dir,dBs[i].frq,dBs[i].time);
	fclose(stream);
    }
}


int read_csv()
{
    FILE* stream;
    char line[200];
    int cnt=0,rep=0,i,j ;
    uint32_t time_last=0;
    double dlat,dlon;

    char *txt;

    stream = fopen("/tmp/sonde.csv", "r");

    if(stream){
        cnt=0;
        while (fgets(line, 200, stream) && cnt<DBS_SIZE )
        {
	    line[sizeof(line)-1]=0;
            char* tmp = strdup(line);
            txt=strtok(tmp,";");
            i=0;
            while(txt!=NULL){
                switch(i){
                    case 0:
			strcpy(dBs[cnt].name,txt);
			break;
                    case 1:
			dBs[cnt].lat=atof(txt);
			break;
                    case 2:
			dBs[cnt].lon=atof(txt);
			break;
                    case 3:
			dBs[cnt].alt=atol(txt);
			break;
                    case 4:
			dBs[cnt].speed=atof(txt);
			break;
                    case 5:
			dBs[cnt].climb=atof(txt);
			break;
                    case 6:
			dBs[cnt].dir=atof(txt);
			break;
                    case 7:
			dBs[cnt].frq=atof(txt);
			break;
                    case 8:
			dBs[cnt].time=atol(txt);
			break;
                }
		i++;
		txt= strtok(NULL, ";");
            }

            cnt++;
            free(tmp);
        }
        fclose(stream);
    }


}



char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);
    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
}

size_t write_data( void *buffer, size_t size, size_t nmemb, void *userp )
{
    return size * nmemb;
}

unsigned int passAprs(char *pas){

    unsigned int hash = 0x73e2;
    char i = 0,endc=0;
    char len = strlen(pas);

    while((pas[i]!='-')&&(i<len))
	i++;
    endc=i;
    i=0;

    while (i < endc) {
        hash ^= pas[i]<<8;
        hash ^= pas[i + 1];
        i += 2;
    }
    hash &= 0x7fff;
    return hash;
}


void  saveMysql( char *name,unsigned int frameno, double lat, double lon, double alt, double speed, double dir, double climb,int typ,char bk, unsigned int swv,double ozon, char aux, double press,  float frq, float vbat, float t1, float t2, float hum){
    char str[1024];
    char hash[40];

    char ToHash[300];
    char Pass[20];
    char dp=strlen(dbPass);
    char cp=strlen(mycall);
    double dlat,dlon;

    if((dp>4)||(cp>3)){

	if(dp<4){	//jesli nie ma mojego hasla 
	    sprintf(str,"%u",passAprs(mycall)); //generujemy auto z znaku delikwenta
    	    strcpy(Pass,str);
	}
	else
    	    strcpy(Pass,dbPass);
	str[0]=0;

        sprintf( UDPbuf, "S0;1;0;0;%s;%lf;%lf;%5.1lf;%u;%3.1f;%3.0f;%3.1f;%4.1f;%4.1f;%u;%i;%i;%i;%7.3f;%3.2f;%3.1f;%3.1f;%3.0f;%s",
                                name,lat,lon,alt,frameno,speed,dir,climb,press,ozon,swv,bk,typ,aux,frq,vbat,t1,t2,hum,mycall);

    	//wylicznie hasha
    	strcpy(ToHash,UDPbuf);
    	strcat(ToHash,Pass);
    	char *hash = str2md5(ToHash, strlen(ToHash));

    	//dopisanie hasha
    	strcat(UDPbuf,";");
    	strcat(UDPbuf,hash);


        if (sendto(sockfd, UDPbuf, BUFLEN, 0, (struct sockaddr*)&serv_addr, slen)==-1)
            printf("err: sendto()");
	else
	    printf("send to DB\n");


    }
}


void save_Slog( char *name,unsigned int frameno, double lat, double lon, double alt, double speed, double dir, double climb,int typ,char bk, unsigned int swv,double ozon, char aux, double press,  float frq, float vbat, float t1, float t2, float hum){

    int i,newS=1;
    time_t minTime=time(NULL),difftime,lTime=time(NULL);
    struct tm* tm_info;

    tm_info = gmtime(&lTime);

    char s[30],s1[20],sf[50];
    strftime(s, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    strftime(s1, 26, "%Y-%m-%d", tm_info);
    sprintf(sf,"/tmp/log_%s.csv",s1);
    FILE* fi;

//char *name,frameno,   lat,     lon,  alt,speed,   dir,clmb,typ, ozon,aux,press,       frq,vbat, float t1, float t2, float hum
//    M4353239;01151;49.44570;17.14525;1704 ; 9.14;162.18;5.28;9  ,0.000;0  ;  0.0;         0;0.0 ;0.0;0.0;0.0
//    M4353239;02920;49.38820;17.25604;10342;22.68;158.89;5.77;9  ,0.000;0  ;244.3;2684354560;0.0 ;26815615859

//    if(saveLog){
        fi = fopen(sf, "a+");
	if (fi){
    	    fprintf(fi,"%s;%s;%05u;%0.5f;%0.5f;%0.0f;%0.2f;%0.2f;%0.2f;%i,%0.3f;%i;%0.1f;%0.3f;%02.1f;%03.1f;%03.1f;%03.1f;\n",
		s,name,frameno,X2C_DIVL(lat,1.7453292519943E-2),X2C_DIVL(lon,1.7453292519943E-2),alt,speed,dir,climb, typ,ozon,aux,press,frq,vbat,t1,t2,hum);
	    fclose(fi);
        }
	
//    }



}

int store_sonde_db( char *name,unsigned int frameno, double lat, double lon, double alt, double speed, double dir, double climb,int typ,char bk, unsigned int swv,double ozon, char aux, double press,  float frq, float vbat, float t1, float t2, float hum){

    int i,newS=1;
    time_t minTime=time(NULL),difftime,lTime=time(NULL);
    struct tm* tm_info;
    
    if(t1<-250 || t1>80) t1=0;
    if(typ!=20 &&(t2<-250 || t2>80)) t2=0;
    if(hum<0 || hum>100) hum=0;
    if(vbat<1 || vbat>40) vbat=0;

    for (i=0;i<DBS_SIZE;i++){
	if (!strncmp(dBs[i].name,name,19)) break;
	if (!dBs[i].name[0]) break;
    }
    if (i==DBS_SIZE){ 
	for (i=0;i<(DBS_SIZE-1);i++) memcpy(&dBs[i],&dBs[i+1],sizeof(struct DBS));
    }

    strcpy(dBs[i].name,name);
    dBs[i].lat=X2C_DIVL(lat,1.7453292519943E-2);
    dBs[i].lon=X2C_DIVL(lon,1.7453292519943E-2);
    dBs[i].alt=alt;
    dBs[i].speed=speed;
    dBs[i].climb=climb;
    dBs[i].dir=dir;
    dBs[i].frq=frq;
    dBs[i].time=time(NULL);
    dBs[i].frameno=frameno;
    dBs[i].typ=typ;
    dBs[i].bk=bk;
    dBs[i].swv=swv;
    dBs[i].ozon=ozon;
    dBs[i].aux=aux;
    dBs[i].press=press;
    dBs[i].vbat=vbat;
    dBs[i].t1=t1;
    dBs[i].t2=t2;
    dBs[i].hum=hum;

    difftime=minTime-dBs[i].sendtime;
    printf("***TIME %lu ALT:%1.0f NEW:%i\n",difftime,alt,newS);
    if(disSKP==0){
	if(alt<3000 || difftime>SEND_INT){
	    saveMysql( name, frameno, dBs[i].lat, dBs[i].lon, alt, speed, dir, climb, typ, bk, swv, ozon, aux, press, frq,vbat,t1,t2,hum);
	    dBs[i].sendtime=time(NULL);
	}
    }
    if(save2csv) save_csv();


}
 

//-SKP

//SQ6QV
int store_sonde_rs(char *name,unsigned int frameno, double lat, double lon, double alt, double speed, double dir, double climb,int typ,char bk, unsigned int swv,double ozon, char aux, double press,  float frq, float vbat, float t1, float t2, float hum, char * src_call){

FILE * band_lock;

    if (alt<2000){
	band_lock=fopen("/tmp/band_lock","w+");
	if (band_lock){
    	    fprintf(band_lock,"%s alt: %f",name, alt);
    	    fclose(band_lock);
	}
    }

//#include "sondemod_qv.c"

}


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static float pow0(float x, uint32_t y)
{
   float z;
   z = x;
   while (y>1UL) {
      z = z*x;
      --y;
   }
   return z;
} /* end pow() */


static double atan20(double x, double y)
{
   double w;
   if (fabs(x)>fabs(y)) {
      w = (double)osic_arctan((float)(X2C_DIVL(y,x)));
      if (x<0.0) {
         if (y>0.0) w = 3.1415926535898+w;
         else w = w-3.1415926535898;
      }
   }
   else if (y!=0.0) {
      w = (double)(1.5707963267949f-osic_arctan((float)(X2C_DIVL(x,
                y))));
      if (y<0.0) w = w-3.1415926535898;
   }
   else w = 0.0;
   return w;
} /* end atan2() */


static char GetNum(const char h[], uint32_t h_len, char eot,
                 uint32_t * p, uint32_t * n)
{
   *n = 0UL;
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */


static void Parms(void)
{
   char err;
   char lowbeacon;
   FILENAME h;
   uint32_t cnum;
   uint32_t i;
   mycall[0U] = 0;
   semfile[0] = 0;
   yumafile[0] = 0;
   rinexfile[0] = 0;
   pilname[0] = 0;   // <- added for pilot sonde
   err = 0;
   rxsock = -1L;
   sondeaprs_dao = 0;
   lowbeacon = 0;
   maxalmage = 21600UL;
   almrequest = 14400UL;
   sondeaprs_verb = 0;
   sondeaprs_verb2 = 0;
   save2csv = 0;
   saveLog=0;
   sendquick = 1UL;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='d') sondeaprs_dao = 1;
         else if (h[1U]=='F') sondeaprs_nofilter = 1;
         else if (h[1U]=='o') {
            osi_NextArg(soundfn, 1024ul);
            if (aprsstr_StrToCard(soundfn, 1024ul, &cnum)) {
               /* listen on UDP instead of soundcard */
               soundfn[0] = 0;
               rxsock = openudp();
               if (rxsock<0L) Error("cannot open rx udp socket", 26ul);
               if (bindudp(rxsock, cnum)<0L) {
                  Error("cannot bind inport", 19ul);
               }
            }
         }
         else if (h[1U]=='T') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            maxalmage = cnum*60UL;
         }
         else if (h[1U]=='R') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            almrequest = cnum*60UL;
         }
         else if (h[1U]=='p') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            sendquick = cnum;
         }
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 100ul);
            if ((uint8_t)mycall[0U]<' ') {
               Error("-I <mycall>", 12ul);
            }
         }
         else if (h[1U]=='s') {
            osi_NextArg(semfile, 1024ul);
            if ((uint8_t)semfile[0U]<' ') Error("-s <filename>", 14ul);
         }
         else if (h[1U]=='x') {
            osi_NextArg(rinexfile, 1024ul);
            if ((uint8_t)rinexfile[0U]<' ') Error("-x <filename>", 14ul);
         }
         else if (h[1U]=='y') {
            osi_NextArg(yumafile, 1024ul);
            if ((uint8_t)yumafile[0U]<' ') Error("-y <filename>", 14ul);
         }
         // skp
        else if (h[1U]=='K') {
            osi_NextArg(dbPass, 20ul);
            if ((unsigned char)dbPass[0U]<' ') {
               Error("-K <database password>", 23ul);
            }
         }
         else if (h[1U]=='e') {
            save2csv=1;
         }
         else if (h[1U]=='l') {
            saveLog=1;
         }
         else if (h[1U]=='D') {
            disSKP=1;
         }

         else if (h[1U]=='v') sondeaprs_verb = 1;
         else if (h[1U]=='V') {
            sondeaprs_verb = 1;
            sondeaprs_verb2 = 1;
         }
         else {
            if (h[1U]=='h') {
               osi_WrStr("sondemod(c) 2.0", 16ul);
               osi_WrStrLn(" multichannel decoder RS92, RS41, SRS-C34 Radiosondes", 54ul);
               osi_WrStrLn(" -A <meter>     at lower altitude use -B beacon time (meter) -A 1000", 69ul);
               osi_WrStrLn(" -B <seconds>   low altitude send intervall -B 10", 50ul);
               osi_WrStrLn(" -b <seconds>   high altitude minimum send intervall -b 20", 59ul);
               osi_WrStrLn(" -d             dao extension for 20cm APRS resolution instead of 18m", 70ul);
               osi_WrStrLn(" -F             trackfilter off, DO NOT USE THIS SENDING TO THE WORLD!", 71ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -I <mycall>    Sender of Object Callsign -I OE0AAA if not sent by \'sondeudp\'", 78ul);
               osi_WrStrLn(" -o <UDPport>   receive demodulated data via UDP port from \'sondeudp -u ...\'", 77ul);
               osi_WrStrLn(" -p <num>       0 send if weather data ready, 1 if MHz known, 2 send immediatly (1)", 84ul);
               osi_WrStrLn(" -R <minutes>   request new rinex almanach after minutes if receiving gps (-R 240)", 83ul);
               osi_WrStrLn("                use somewhat like \'getalmd\'-script to download", 63ul);
               osi_WrStrLn(" -r <ip>:<port> send AXUDP -r 127.0.0.1:9001 use udpgate4 or aprsmap as receiver", 81ul);
               osi_WrStrLn(" -s <filename>  gps almanach sem format (DO NOT USE, not exact)", 64ul);
               osi_WrStrLn(" -T <minutes>   stop sending data after almanach age (-T 360)", 62ul);
               osi_WrStrLn(" -t <filename>  append comment lines from this file", 52ul);
	       osi_WrStrLn(" -n <pilot name> name for pilot sonde object", 45ul);
               osi_WrStrLn(" -V             more verbous", 29ul);
               osi_WrStrLn(" -v             verbous", 24ul);
               osi_WrStrLn(" -x <filename>  gps almanach rinexnavigation format (prefered)", 63ul);
               osi_WrStrLn(" -y <filename>  gps almanach yuma format (DO NOT USE, not exact)", 65ul);
	       osi_WrStrLn(" -K <password>  password for SP9SKP database", 45ul);
	       osi_WrStrLn(" -e             write last 30 radiosondes data to /tmp/sonde.csv", 66ul);
	       osi_WrStrLn(" -l             write all received frames to /tmp/log.csv", 58ul);
	       osi_WrStrLn(" -D             Disable sending to SKP database", 47ul);	
               osi_WrStrLn("example: sondemod -o 18000 -x almanach.txt -d -A 1500 -B 10 -I OE0AAA -r 127.0.0.1:9001", 88ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
   if (!lowbeacon) sondeaprs_lowaltbeacontime = sondeaprs_beacontime;
} /* end Parms() */

/*  WrStr(" "); WrHex(n DIV 01000000H MOD 256, 2);
                WrHex(n DIV 010000H MOD 256, 2);
                WrHex(n DIV 0100H MOD 256, 2); WrHex(n MOD 256, 2);  */
#define sondemod_Z 48


static void degtostr(float d, char lat, char form,
                char s[], uint32_t s_len)
{
   uint32_t i;
   uint32_t n;
   if (s_len-1<11UL) {
      s[0UL] = 0;
      return;
   }
   if (form=='2') i = 7UL;
   else if (form=='3') i = 8UL;
   else i = 9UL;
   if (d<0.0f) {
      d = -d;
      if (lat) s[i] = 'S';
      else s[i+1UL] = 'W';
   }
   else if (lat) s[i] = 'N';
   else s[i+1UL] = 'E';
   if (form=='2') {
      /* DDMM.MMNDDMM.MME */
      n = osi_realcard(d*3.4377467707849E+5f+0.5f);
      s[0UL] = (char)((n/600000UL)%10UL+48UL);
      i = (uint32_t)!lat;
      s[i] = (char)((n/60000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/6000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/1000UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/100UL)%10UL+48UL);
      ++i;
      s[i] = '.';
      ++i;
      s[i] = (char)((n/10UL)%10UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
   }
   else if (form=='3') {
      /* DDMM.MMMNDDMM.MMME */
      n = osi_realcard(d*3.4377467707849E+6f+0.5f);
      s[0UL] = (char)((n/6000000UL)%10UL+48UL);
      i = (uint32_t)!lat;
      s[i] = (char)((n/600000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/60000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/10000UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/1000UL)%10UL+48UL);
      ++i;
      s[i] = '.';
      ++i;
      s[i] = (char)((n/100UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/10UL)%10UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
   }
   else {
      /* DDMMSS */
      n = osi_realcard(d*2.062648062471E+5f+0.5f);
      s[0UL] = (char)((n/360000UL)%10UL+48UL);
      i = (uint32_t)!lat;
      s[i] = (char)((n/36000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/3600UL)%10UL+48UL);
      ++i;
      s[i] = 'd';
      ++i;
      s[i] = (char)((n/600UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/60UL)%10UL+48UL);
      ++i;
      s[i] = '\'';
      ++i;
      s[i] = (char)((n/10UL)%6UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
      s[i] = '\"';
      ++i;
   }
   ++i;
   s[i] = 0;
} /* end degtostr() */


static void initcontext(struct CONTEXTR9 * cont)
{
   memset((char *)cont,(char)0,sizeof(struct CONTEXTR9));
   cont->lastlat = 8.4214719496019E-1;
   cont->laslong = 2.2755602787502E-1;
} /* end initcontext() */


static void dogps(const char sf[], uint32_t sf_len,
                struct CONTEXTR9 * cont, uint32_t * timems,
                uint32_t * gpstime)
{
   uint32_t i;
   gpspos_SATS sats;
   int32_t res;
   int32_t d1;
   int32_t d;
   char h[100];
   struct CONTEXTR9 * anonym;
   cont->lat = 0.0;
   cont->long0 = 0.0;
   cont->heig = 0.0;
   cont->speed = 0.0;
   cont->dir = 0.0;
   /*WrStrLn("gps:"); */
   /*FOR i:=0 TO 121 DO WrHex(ORD(sf[i]), 3) END; WrStrLn(""); */
   *timems = (uint32_t)(uint8_t)sf[0UL]+(uint32_t)(uint8_t)
                sf[1UL]*256UL+(uint32_t)(uint8_t)
                sf[2UL]*65536UL+(uint32_t)(uint8_t)sf[3UL]*16777216UL;
   if (sondeaprs_verb2) {
      aprsstr_TimeToStr(( *timems/1000UL)%86400UL, h, 100ul);
      osi_WrStr("time ms day: ", 14ul);
      osi_WrStr(h, 100ul);
      osic_WrINT32( *timems%1000UL, 4UL);
      osic_WrINT32( *timems/86400000UL, 2UL);
      osi_WrStrLn("", 1ul);
   }
   /*  WrInt(ORD(sf[4]), 4); WrInt(ORD(sf[5]), 4); WrStrLn(""); */
   /*  FILL(ADR(sats), 0C, SIZE(sats)); */
   for (i = 0UL; i<=3UL; i++) {
      sats[i*3UL].prn = (uint32_t)(uint8_t)sf[i*2UL+6UL]&31UL;
      sats[i*3UL+1UL].prn = (uint32_t)(uint8_t)
                sf[i*2UL+6UL]/32UL+(uint32_t)(uint8_t)
                sf[i*2UL+7UL]*8UL&31UL;
      sats[i*3UL+2UL].prn = (uint32_t)(uint8_t)sf[i*2UL+7UL]/4UL&31UL;
   } /* end for */
   if (sondeaprs_verb2) {
      osi_WrStr("prn:", 5ul);
      for (i = 0UL; i<=11UL; i++) {
         osic_WrINT32(sats[i].prn, 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      osi_WrStr("sig: ", 6ul);
      for (i = 0UL; i<=11UL; i++) {
         osi_WrHex((uint32_t)(uint8_t)sf[i+14UL], 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      osi_WrStrLn("rang:", 6ul);
   }
   for (i = 0UL; i<=11UL; i++) {
      if (sats[i].prn>0UL) {
         sats[i].rang = (int32_t)((uint32_t)(uint8_t)
                sf[i*8UL+26UL]+(uint32_t)(uint8_t)
                sf[i*8UL+27UL]*256UL+(uint32_t)(uint8_t)
                sf[i*8UL+28UL]*65536UL+(uint32_t)(uint8_t)
                sf[i*8UL+29UL]*16777216UL);
         sats[i].rang1 = (int32_t)((uint32_t)(uint8_t)
                sf[i*8UL+30UL]+(uint32_t)(uint8_t)
                sf[i*8UL+31UL]*256UL+(uint32_t)(uint8_t)
                sf[i*8UL+32UL]*65536UL);
         sats[i].rang1 = sats[i].rang1&8388607L;
         sats[i].rang3 = (int32_t)(signed char)(uint8_t)sf[i*8UL+33UL];
         d = sats[i].rang-lastsat[i].rang;
         d1 = sats[i].rang1-lastsat[i].rang1;
         if (sondeaprs_verb2) {
            osic_WrINT32(sats[i].prn, 3UL);
            osic_WrINT32((uint32_t)sats[i].rang, 12UL);
            osic_WrINT32((uint32_t)sats[i].rang1, 12UL);
            osic_WrINT32((uint32_t)sats[i].rang3, 5UL);
            osic_WrINT32((uint32_t)d, 12UL);
            osic_WrINT32((uint32_t)(d-lastsat[i].lastd), 12UL);
            osi_WrStrLn("", 1ul);
         }
         sats[i].lastd = d;
         sats[i].lastd1 = d1;
      }
   } /* end for */
   memcpy(lastsat,sats,sizeof(gpspos_SATS));
   { /* with */
      struct CONTEXTR9 * anonym = cont;
      systime = osic_time();
      if (almread>systime) almread = 0UL;
      if (almread+60UL>systime) {
         *gpstime = systime;
         res = gpspos_getposit(anonym->timems, gpstime, sats,
                anonym->lastlat, anonym->laslong, anonym->lastalt,
                &anonym->lat, &anonym->long0, &anonym->heig, &anonym->speed,
                &anonym->dir, &anonym->climb, &anonym->hrmsc, &anonym->vrmsc,
                 &anonym->goodsats);
      }
      else res = -2L;
      if (res>=0L) {
         anonym->lastlat = anonym->lat;
         anonym->laslong = anonym->long0;
         anonym->lastalt = anonym->heig;
         anonym->lastspeed = anonym->speed;
         anonym->lastdir = anonym->dir;
         anonym->lastclb = anonym->climb;
      }
      else *gpstime = 0UL;
   }
   if (sondeaprs_verb && res>=0L) {
      degtostr((float)cont->lat, 1, '3', h, 100ul);
      osi_WrStr(h, 100ul);
      osi_WrStr(" ", 2ul);
      degtostr((float)cont->long0, 0, '3', h, 100ul);
      osi_WrStr(h, 100ul);
      /*    WrStr("pos: "); WrFixed(lat/RAD, 5, 12);
                WrFixed(long/RAD, 5, 12); */
      osic_WrFixed((float)cont->heig, 0L, 10UL);
      osi_WrStr("m ", 3ul);
      osic_WrFixed((float)(cont->speed*3.6), 1L, 6UL);
      osi_WrStr("km/h ", 6ul);
      osic_WrFixed((float)cont->dir, 0L, 5UL);
      osi_WrStr("deg ", 5ul);
      osic_WrFixed((float)cont->climb, 1L, 7UL);
      osi_WrStr("m/s", 4ul);
      osi_WrStr(" h/vrms:", 9ul);
      osic_WrFixed(cont->hrmsc, 1L, 0UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(cont->vrmsc, 1L, 0UL);
      osi_WrStrLn("", 1ul);
   }
} /* end dogps() */


static void decodecalib(const char cd[], uint32_t cd_len)
{
   uint32_t n;
   uint32_t i;
   uint32_t cr;
   uint32_t tmp;
   memset((char *)coeff,(char)0,sizeof(float [256]));
   i = 64UL;
   for (tmp = 88UL;;) {
      n = (uint32_t)(uint8_t)cd[i];
      cr = (uint32_t)(uint8_t)cd[i+1UL]+(uint32_t)(uint8_t)
                cd[i+2UL]*256UL+(uint32_t)(uint8_t)
                cd[i+3UL]*65536UL+(uint32_t)(uint8_t)
                cd[i+4UL]*16777216UL;
      coeff[n] = *X2C_CAST(&cr,uint32_t,float,float *);
      if (!tmp) break;
      --tmp;
      i += 5UL;
   } /* end for */
} /* end decodecalib() */


static float coef(float ref, float u, float c)
{
   float x;
   float v;
   v = X2C_DIVR(ref,u);
   x = 1.0f-v*(1.0f-c);
   if (x!=0.0f) return X2C_DIVR(v,x);
   return 0.0f;
} /* end coef() */


static float extr(uint32_t hi, uint32_t lo, uint32_t u,
                uint32_t idx)
{
   float f;
   float x;
   float v;
   uint32_t i;
   uint32_t tmp;
   if (hi<=lo || u<=lo) return 0.0f;
   v = coef((float)(hi-lo), (float)(u-lo), coeff[idx+7UL]);
   x = 0.0f;
   f = 1.0f;
   tmp = idx+5UL;
   i = idx;
   if (i<=tmp) for (;; i++) {
      /* sum(x^n * k[n] */
      x = x+coeff[i]*f;
      f = f*v;
      if (i==tmp) break;
   } /* end for */
   return x;
} /* end extr() */


static void domes(const char md[], uint32_t md_len, double * hp,
                double * hyg, double * temp)
{
   uint32_t i;
   int32_t m[8];
   float d5;
   float d4;
   float d3;
   float p;
   float hr2;
   float hr1;
   for (i = 0UL; i<=7UL; i++) {
      m[i] = (int32_t)((uint32_t)(uint8_t)md[i*3UL]+(uint32_t)
                (uint8_t)md[i*3UL+1UL]*256UL+(uint32_t)(uint8_t)
                md[i*3UL+2UL]*65536UL);
   } /* end for */
   /* hygro 1 */
   /*  IF verb THEN WrStr(" <h> ") END; */
   hr1 = extr((uint32_t)m[3U], (uint32_t)m[7U], (uint32_t)m[1U], 40UL);
   hr2 = extr((uint32_t)m[3U], (uint32_t)m[7U], (uint32_t)m[2U], 50UL);
   if (hr2>hr1) hr1 = hr2;
   if (hr1<2.0f) hr1 = 0.0f;
   else if (hr1>100.0f) hr1 = 100.0f;
   *hyg = (double)hr1;
   /* temp */
   *temp = (double)extr((uint32_t)m[3U], (uint32_t)m[7U],
                (uint32_t)m[0U], 30UL);
   /* baro */
   d3 = (float)(m[3U]-m[7U]);
   d4 = (float)(m[4U]-m[7U]);
   d5 = (float)(m[5U]-m[7U]);
   p = extr((uint32_t)m[3U], (uint32_t)m[7U], (uint32_t)m[5U],
                10UL)+coeff[60U]*extr((uint32_t)m[3U], (uint32_t)m[7U],
                (uint32_t)m[4U], 20UL)+X2C_DIVR(coeff[61U]*coeff[20U]*d3,
                d5)+X2C_DIVR(coeff[61U]*coeff[21U]*coef(d3, d4,
                coeff[27U])*d3,d5)+X2C_DIVR(coeff[61U]*coeff[22U]*pow0(coef(d3,
                 d4, coeff[27U]), 2UL)*d3,
                d5)+X2C_DIVR(coeff[61U]*coeff[23U]*pow0(coef(d3, d4,
                coeff[27U]), 3UL)*d3,
                d5)+X2C_DIVR(coeff[62U]*coeff[20U]*d3*d3,
                d5*d5)+X2C_DIVR(coeff[62U]*coeff[21U]*coef(d3, d4,
                coeff[27U])*d3*d3,
                d5*d5)+X2C_DIVR(coeff[62U]*coeff[22U]*pow0(coef(d3, d4,
                coeff[27U]), 2UL)*d3*d3,
                d5*d5)+X2C_DIVR(coeff[62U]*coeff[23U]*pow0(coef(d3, d4,
                coeff[27U]), 3UL)*d3*d3,
                d5*d5)+X2C_DIVR(coeff[63U]*coeff[20U]*pow0(d3, 3UL),pow0(d5,
                3UL))+X2C_DIVR(coeff[63U]*coeff[21U]*coef(d3, d4,
                coeff[27U])*pow0(d3, 3UL),pow0(d5,
                3UL))+X2C_DIVR(coeff[63U]*coeff[22U]*pow0(coef(d3, d4,
                coeff[27U]), 2UL)*pow0(d3, 3UL),pow0(d5,
                3UL))+X2C_DIVR(coeff[63U]*coeff[23U]*pow0(coef(d3, d4,
                coeff[27U]), 3UL)*pow0(d3, 3UL),pow0(d5, 3UL));
   /*
          + coeff[70]*pow(coeff[20],2)
          + coeff[70]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)
          + coeff[70]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)
          + coeff[70]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)
   
          + coeff[71]*pow(coeff[20],2)*d3/d5
          + coeff[71]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)*d3/d5
          + coeff[71]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)*d3/d5
          + coeff[71]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)*d3/d5
   
          + coeff[72]*pow(coeff[20],2)*pow(d3,2)/pow(d5,2)
          + coeff[72]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)*pow(d3,
                2)/pow(d5,2)
          + coeff[72]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)*pow(d3,
                2)/pow(d5,2)
          + coeff[72]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)*pow(d3,
                2)/pow(d5,2)
   
          + coeff[73]*pow(coeff[20],2)*pow(d3,3)/pow(d5,3)
          + coeff[73]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)*pow(d3,
                3)/pow(d5,3)
          + coeff[73]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)*pow(d3,
                3)/pow(d5,3)
          + coeff[73]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)*pow(d3,
                3)/pow(d5,3)
   
          + coeff[80]*pow(coeff[20],3)
          + coeff[80]*pow(coeff[21],3)*pow(coef(d3, d4, coeff[27]),3)
          + coeff[80]*pow(coeff[22],3)*pow(coef(d3, d4, coeff[27]),6)
          + coeff[80]*pow(coeff[23],3)*pow(coef(d3, d4, coeff[27]),9)
   
          + coeff[81]*pow(coeff[20],3)*d3/d5
          + coeff[81]*pow(coeff[21],3)*pow(coef(d3, d4, coeff[27]),3)*d3/d5
          + coeff[81]*pow(coeff[22],3)*pow(coef(d3, d4, coeff[27]),6)*d3/d5
          + coeff[81]*pow(coeff[23],3)*pow(coef(d3, d4, coeff[27]),9)*d3/d5
   
          + coeff[82]*pow(coeff[20],3)*pow(d3,2)/pow(d5,2)
          + coeff[82]*pow(coeff[21],3)*pow(coef(d3, d4, coeff[27]),3)*pow(d3,
                2)/pow(d5,2)
          + coeff[82]*pow(coeff[22],3)*pow(coef(d3, d4, coeff[27]),6)*pow(d3,
                2)/pow(d5,2)
          + coeff[82]*pow(coeff[23],3)*pow(coef(d3, d4, coeff[27]),9)*pow(d3,
                2)/pow(d5,2)
   */
   *hp = (double)p;
   /*  
   x10:=c[10] + ....
   x20:=c[20] + c[21]*v(m4) + c[22]*v(m4)^2 ...
   x60:=-c[60]*x20 + 10*c[61]*x20 - 100*c[62]*x20 + 1000*c[63]*x20
   x70:=c[70]*x20^2 - 10*c[71]*x20^2 + 100*c[72]*x20^2 - 1000*c[72]*x20^2
   x80:=-c[80]*x20^3 + 10*c[81]*x20^3 - 100*c[82]*x20^3
   p:=x10 + x20 + x60 + x70 + x80
   */
   if (sondeaprs_verb) {
      osi_WrStr("mes:", 5ul);
      if (sondeaprs_verb2) {
         for (i = 0UL; i<=7UL; i++) {
            osic_WrINT32((uint32_t)m[i], 7UL);
            osi_WrStr(" ", 2ul);
         } /* end for */
         osi_WrStrLn("", 1ul);
      }
      osic_WrFixed((float)*temp, 3L, 7UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(hr1, 3L, 7UL);
      /*WrStr(" ");WrFixed(hr2, 3,7); */
      osi_WrStr(" ", 2ul);
      osic_WrFixed(p, 2L, 8UL);
   }
/*WrStr(" ");WrFixed(x2, 2,8); */
/*WrStrLn(""); */
} /* end domes() */

static float sondemod_P[13] = {1000.0f,150.0f,100.0f,70.0f,60.0f,50.0f,
                40.0f,30.0f,20.0f,15.0f,10.0f,8.0f,0.0f};

static float sondemod_C[13] = {1.0f,1.0f,1.01f,1.022f,1.025f,1.035f,
                1.047f,1.065f,1.092f,1.12f,1.17f,1.206f,1.3f};

static float _cnst0[13] = {1.0f,1.0f,1.01f,1.022f,1.025f,1.035f,1.047f,
                1.065f,1.092f,1.12f,1.17f,1.206f,1.3f};
static float _cnst[13] = {1000.0f,150.0f,100.0f,70.0f,60.0f,50.0f,40.0f,
                30.0f,20.0f,15.0f,10.0f,8.0f,0.0f};

static double getOzoneCorr(double p)
/* From from ftp://ftp.cpc.ncep.noaa.gov/ndacc/meta/sonde/cv_payerne_snd.txt */
{
   uint32_t i;
   i = 12UL;
   while (i>0UL && (double)_cnst[i]<p) --i;
   return (double)_cnst0[i];
} /* end getOzoneCorr() */

/*
03 03 00 00 00 00 00 00 00 00 B2 7D  no aux
00 03 21 02 5C 5F 00 00 78 1C D4 C9  open input
00 03 00 00 54 5F 00 00 20 1C D8 1F
ozon zero 544
0.35nA/step  
volt 0.13mV/step
220cm3/min
mPa=0.043085*i*flow*temp(kelvin)
flow=time(s)/100cm3  (27.27)
ground 1..7mPa, stratosphere <25mPa
*/
#define sondemod_T20 25000.0
/* adc 20C */

#define sondemod_TM7 65535.0
/* adc fullrange - temp */

#define sondemod_OZON0 550.0
/* adc zero level ozon */

#define sondemod_OZONADC 0.31
/* nA per step */

#define sondemod_MPAUA 4
/* mPa per uA */


static void doozon(const char s[], uint32_t s_len,
                const double airpres, double * otemp,
                double * ozon)
{
   *otemp = (double)(float)((uint32_t)(uint8_t)
                s[4UL]+(uint32_t)(uint8_t)s[5UL]*256UL);
   *ozon = (double)(float)((uint32_t)(uint8_t)
                s[2UL]+(uint32_t)(uint8_t)s[3UL]*256UL);
   *otemp = (65535.0-*otemp)*1.3568521031208E-3-35.0;
   *ozon = (*ozon-550.0)*0.00124;
   *ozon =  *ozon*(*otemp+273.15)*3.0769230769231E-3*getOzoneCorr(airpres);
                /* temp and pressure correction */
   if (*ozon<=0.0) *ozon = 0.0;
   if (sondeaprs_verb) {
      osi_WrStr("ozon:", 6ul);
      osic_WrFixed((float)*ozon, 1L, 5UL);
      osi_WrStr("mPa temp:", 10ul);
      osic_WrFixed((float)*otemp, 1L, 5UL);
      osi_WrStrLn("C", 2ul);
   }
/*WrStr(" ");WrFixed(FLOAT(ORD(s[8])+ORD(s[9])*256), 0,8); */
} /* end doozon() */


/*
static void calibfn(char obj[], uint32_t obj_len, char fn[],uint32_t fn_len)
{
   uint32_t i,len;

   X2C_PCOPY((void **)&obj,obj_len);

   while (i<=obj_len-1 && obj[i]) {
      if (((uint8_t)obj[i]<'0' || (uint8_t)obj[i]>'9') && ((uint8_t)obj[i]<'A' || (uint8_t)obj[i]>'Z')) {
         osi_WrStr("bad sonde serial obj: ", 23ul);
         osi_WrStr(obj, 1024ul);
	osi_WrStr("\n", 2ul);
         obj[0UL] = 0;
         goto label;
      }
      ++i;
   }

   aprsstr_Assign(fn, fn_len, "/tmp/", 6ul);
   aprsstr_Append(fn, fn_len, obj, obj_len);
   aprsstr_Append(fn, fn_len, ".cal", 5ul);

   label:;

   X2C_PFREE(obj);
} // end calibfn() 
*/



static void calibfn(char obj[], uint32_t obj_len, char fn[],uint32_t fn_len)
{
   uint32_t i,len;

   X2C_PCOPY((void **)&obj,obj_len);

   sprintf(fn,"/tmp/%s.cal",obj);    

   i = 0UL;
   while (i<=obj_len-1 && obj[i]) {
      if (((uint8_t)obj[i]<'0' || (uint8_t)obj[i]>'9') && ((uint8_t)obj[i]<'A' || (uint8_t)obj[i]>'Z')) {
         fn[0UL] = 0;
      }
      ++i;
   }
   X2C_PFREE(obj);
} // end calibfn() 


static void readcontext(struct CONTEXTR9 * cont, char objname0[],
                uint32_t objname_len)
{
   char fn[1024];
   int32_t fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   initcontext(cont);
   calibfn(objname0, objname_len, fn, 1024ul);
   fd = osi_OpenRead(fn, 1024ul);
   if (fd>=0L) {
      if (osi_RdBin(fd, (char *)cont, sizeof(struct CONTEXTR9)/1u,
                sizeof(struct CONTEXTR9))!=(int32_t)
                sizeof(struct CONTEXTR9)) initcontext(cont);
      osic_Close(fd);
   }
   X2C_PFREE(objname0);
} /* end readcontext() */


static void wrcontext(struct CONTEXTR9 * cont, char objname0[],
                uint32_t objname_len)
{
   char fn[1024];
   int32_t fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   calibfn(objname0, objname_len, fn, 1024ul);
   if (fn[0U]) {
      fd = osi_OpenWrite(fn, 1024ul);
      if (fd>=0L) {
         osi_WrBin(fd, (char *)cont, sizeof(struct CONTEXTR9)/1u,
                sizeof(struct CONTEXTR9));
         osic_Close(fd);
      }
   }
   X2C_PFREE(objname0);
} /* end wrcontext() */


static void docalib(const char sf[], uint32_t sf_len,
                char objname0[], uint32_t objname_len,
                struct CONTEXTR9 * cont, float * mhz0,
                uint32_t * frameno)
{
   uint32_t idx;
   uint32_t j;
   uint32_t i;
   char new0;
   *mhz0 = 0.0f;
   new0 = 0;
   i = 0UL;
   for (j = 2UL; j<=11UL; j++) {
      /* object name */
      /*    IF (1 IN cont.calibok) & (sf[j]<>cont.calibdata[j+20])
                THEN cont.calibok:=SET32{} END; */
      if (i<=objname_len-1 && (uint8_t)sf[j]>' ') {
         if (objname0[i]!=sf[j]) new0 = 1;
         objname0[i] = sf[j];
         ++i;
      }
   } /* end for */
   if (i<=objname_len-1) objname0[i] = 0;
   if (new0) readcontext(cont, objname0, objname_len);
   *frameno = (uint32_t)(uint8_t)sf[0UL]+(uint32_t)(uint8_t)
                sf[1UL]*256UL;
   if (sondeaprs_verb) {
      if (new0) osi_WrStr("new ", 5ul);
      osic_WrINT32(*frameno, 1UL); /* frame no */
      osi_WrStr(" ", 2ul);
      osi_WrStr(objname0, objname_len); /*WrStr(" bat:");
                WrHex(ORD(sf[12]), 2);*/
   }
   idx = (uint32_t)(uint8_t)sf[15UL];
   if (idx<32UL) {
      j = idx*16UL;
      for (i = 16UL; i<=31UL; i++) {
         if (j<=511UL) {
            /*      IF (idx IN cont.calibok) & (cont.calibdata[j]<>sf[i])
                THEN cont.calibok:=SET32{} END; */
            cont->calibdata[j] = sf[i];
         }
         ++j;
      } /* end for */
      if (!X2C_IN(idx,32,cont->calibok)) {
         /* got more new info */
         cont->calibok |= (1UL<<idx);
         wrcontext(cont, objname0, objname_len);
      }
      /*    INCL(cont.calibok, idx); */
      if ((0x1UL & cont->calibok)) {
         *mhz0 = (float)(400000UL+((uint32_t)(uint8_t)
                cont->calibdata[2U]+(uint32_t)(uint8_t)
                cont->calibdata[3U]*256UL)*10UL)*0.001f;
         if (sondeaprs_verb) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed(*mhz0, 2L, 6UL);
            osi_WrStr("MHz ", 5ul);
         }
      }
      if (sondeaprs_verb) {
         osi_WrStr(" calib: ", 9ul);
         for (i = 0UL; i<=31UL; i++) {
            if (i==idx) osi_WrStr("!", 2ul);
            else if (X2C_IN(i,32,cont->calibok)) osi_WrStr("+", 2ul);
            else osi_WrStr("-", 2ul);
         } /* end for */
      }
      if (cont->calibok==0xFFFFFFFFUL) {
         /* calibration ready now */
         decodecalib(cont->calibdata, 512ul);
      }
   }
} /* end docalib() */


static uint32_t calperc(uint32_t cs)
{
   uint32_t n;
   uint32_t i;
   n = 0UL;
   for (i = 0UL; i<=31UL; i++) {
      if (X2C_IN(i,32,cs)) ++n;
   } /* end for */
   return (n*100UL)/32UL;
} /* end calperc() */


static void WrRinexfn(uint32_t t)
{
   char fn[31];
   uint32_t y;
   uint32_t d;
   int32_t f;
   /*DateToStr(t, fn); WrStrLn(fn); */
   d = 25568UL+t/86400UL;
   y = (d*4UL)/1461UL;
   d = 1UL+((d*4UL)%1461UL)/4UL;
   strncpy(fn,"brdc0000.00n",31u);
   fn[4U] = (char)(d/100UL+48UL);
   fn[5U] = (char)((d/10UL)%10UL+48UL);
   fn[6U] = (char)(d%10UL+48UL);
   fn[9U] = (char)((y/10UL)%10UL+48UL);
   fn[10U] = (char)(y%10UL+48UL);
   if (sondeaprs_verb) osi_WrStrLn(fn, 31ul);
   f = osi_OpenWrite("getalmanach", 12ul);
   if (f>=0L) {
      osi_WrBin(f, (char *)fn, 31u/1u, aprsstr_Length(fn, 31ul));
      osic_Close(f);
   }
   else osi_WrStrLn("can not write getalmanach file", 31ul);
} /* end WrRinexfn() */


static void getcall(const char b[], uint32_t b_len, char call[],
                uint32_t call_len)
{
   uint32_t c;
   uint32_t n;
   uint32_t i;
   char tmp;
   call[0UL] = 0;
   n = (uint32_t)(uint8_t)b[0UL]*16777216UL+(uint32_t)(uint8_t)
                b[1UL]*65536UL+(uint32_t)(uint8_t)
                b[2UL]*256UL+(uint32_t)(uint8_t)b[3UL];
   if (n>0UL && (uint32_t)(uint8_t)b[4UL]<=15UL) {
      for (i = 5UL;; i--) {
         c = n%37UL;
         if (c==0UL) call[i] = 0;
         else if (c<27UL) call[i] = (char)((c+65UL)-1UL);
         else call[i] = (char)((c+48UL)-27UL);
         n = n/37UL;
         if (i==0UL) break;
      } /* end for */
      call[6UL] = 0;
      c = (uint32_t)(uint8_t)b[4UL];
      if (c>0UL) {
         aprsstr_Append(call, call_len, "-", 2ul);
         if (c>=10UL) {
            aprsstr_Append(call, call_len, "1", 2ul);
            c = c%10UL;
         }
         aprsstr_Append(call, call_len,
                (char *)(tmp = (char)(c+48UL),&tmp), 1u/1u);
      }
   }
/*WrStr("usercall:");WrStrLn(call); */
} /* end getcall() */

static uint16_t sondemod_POLYNOM = 0x1021U;


static void decodeframe(uint8_t m, uint32_t ip, uint32_t fromport)
{
   uint32_t gpstime;
   uint32_t frameno;
   uint32_t len;
   uint32_t ic;
   uint32_t p;
   uint32_t j;
   uint32_t i;
   uint32_t almanachage;
   uint16_t crc;
   char typ;
   char sf[256];
   char bb[256];
   char b[256];
   char crdone;
   char calok;
   CALLSSID usercall;
   struct CONTEXTR9 * anonym;
   struct CONTEXTR9 * anonym0;
   struct CONTEXTR9 * anonym1;
   uint32_t tmp;
   //SKP
   contextr9.aux=0;  
   FILE * ftoold;

   /*
   -- reedsolomon is done by sondeudp
     FOR i:=0 TO HIGH(b) DO b[i]:=0C END;
     FOR i:=0 TO 240-6-24-1 DO b[(255-24-1)-i]:=chan[m].rxbuf[i+6] END;
     FOR i:=0 TO 24-1 DO b[(255-1)-i]:=chan[m].rxbuf[i+(240-24)] END;
   --  WrStrLn(" ecco: ");
   --  FOR i:=216 TO 239 DO WrHex(ORD(chan[m].rxbuf[i]), 4) END; WrStrLn("");
                 WrStrLn("");
   --bb:=b;
     res:=decodersc(b, eraspos, 0);
     IF res>0 THEN
       FOR i:=0 TO 240-6-24-1 DO chan[m].rxbuf[i+6]:=b[(255-24-1)-i] END;
       FOR i:=0 TO 24-1 DO chan[m].rxbuf[i+(240-24)]:=b[(255-1)-i] END;
       IF verb THEN WrInt(res, 1); WrStr(" bytes corrected "); END;
     END;
   */
   /*
     WrInt(res, 1); WrStrLn("=rs");
     FOR i:=0 TO 254 DO
       IF b[i]<>bb[i] THEN
         WrInt(i, 4); WrStr(":");WrHex(ORD(bb[i]), 2); WrStr("-");
                WrHex(ORD(b[i]), 2);
       END;
     END;
     WrStrLn(" diffs");
   */
   for (i = 0UL; i<=255UL; i++) {
      b[i] = chan[m].rxbuf[i];
   } /* end for */
   calok = 0;
   getcall(b, 256ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, bb, 256ul);
      osi_WrStr(bb, 256ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }
   p = 6UL;
   crdone = 1;
   contextr9.posok = 0;
   contextr9.ozontemp = 0.0;
   contextr9.ozon = 0.0;
   mhz = 0.0f;
   for (;;) {
      typ = b[p];
      if (typ=='e') {
         if (sondeaprs_verb) {
            osi_WrStr("cal  ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='g') {
         if (sondeaprs_verb) {
            osi_WrStr("gps  ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='h') {
         if (b[p+2UL]!='\003' && sondeaprs_verb) {
            osi_WrStr("aux ", 5ul);
            crdone = 0;
         }
      }
      else if (typ=='i') {
         if (sondeaprs_verb) {
            osi_WrStr("data ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='\377') break;
      else {
         osi_WrStr("R92 end ", 9ul);
         if (sondeaprs_verb) {
            osi_WrHex((uint32_t)(uint8_t)typ, 4UL);
            crdone = 0;
         }
         break;
      }
      ++p;
      len = (uint32_t)(uint8_t)b[p]*2UL+2UL; /* +crc */
      if (len>=240UL) {
         if (sondeaprs_verb) {
            osi_WrStr("RS92 Frame too long ", 21ul);
            osic_WrINT32(len, 1UL);
            crdone = 0;
         }
         break;
      }
      ++p;
      j = 0UL;
      /*WrInt(len 3);WrStrLn("=len"); */
      crc = 0xFFFFU;
      while (j<len) {
         sf[j] = b[p];
         if (j+2UL<len) {
            for (ic = 0UL; ic<=7UL; ic++) {
               if (((0x8000U & crc)!=0)!=X2C_IN(7UL-ic,8,
                (uint8_t)(uint8_t)b[p])) {
                  crc = X2C_LSH(crc,16,1)^0x1021U;
               }
               else crc = X2C_LSH(crc,16,1);
            } /* end for */
         }
         ++p;
         ++j;
         if (p>240UL) {
            osi_WrStr("eof", 4ul); /* error */
            crdone = 0;
            goto loop_exit;
         }
      }
      crdone = 0;
      if ((char)crc!=sf[len-2UL] || (char)X2C_LSH(crc,16,
                -8)!=sf[len-1UL]) {
         if (sondeaprs_verb) osi_WrStrLn("********* crc error", 20ul);
      }
      else {
         if (typ=='e') {
            docalib(sf, 256ul, objname, 9ul, &contextr9, &mhz, &frameno);
            if (frameno>contextr9.framenum) {
               /* new frame number */
               contextr9.mesok = 0;
               contextr9.posok = 0;
               contextr9.framesent = 0;
               calok = 1;
               contextr9.framenum = frameno;
            }
            else if (contextr9.framenum==frameno && !contextr9.framesent) {
               calok = 1;
            }
            else if (frameno<contextr9.framenum && sondeaprs_verb) {
               osi_WrStrLn("", 1ul);
               osi_WrStr("got out of order frame number ", 31ul);
               osic_WrINT32(frameno, 1UL);
               osi_WrStr(" expecting ", 12ul);
               osic_WrINT32(contextr9.framenum, 1UL);
	       osi_WrStrLn(" ", 1ul);
               crdone = 0;
            }
         }
         else if (typ=='i') {
            { /* with */
               struct CONTEXTR9 * anonym = &contextr9;
               if (calok && anonym->calibok==0xFFFFFFFFUL) {
                  domes(sf, 256ul, &anonym->hp, &anonym->hyg, &anonym->temp);
                  anonym->mesok = 1;
               }
            }
         }
         else if (typ=='g') {
            if (calok) {
               { /* with */
                  struct CONTEXTR9 * anonym0 = &contextr9;
                  dogps(sf, 256ul, &contextr9, &anonym0->timems, &gpstime);
                  if (almread+30UL<=systime) {
                     if (gpspos_readalmanach(semfile, 1024ul, yumafile,
                1024ul, rinexfile, 1024ul, anonym0->timems/1000UL, &almage,
                sondeaprs_verb)) {
                        if (almread+60UL<=systime) {
                           dogps(sf, 256ul, &contextr9, &anonym0->timems,&gpstime);
                        }
                        almread = systime;
                     }
                     else {
			ftoold=fopen("/tmp/toold", "ab+");
			if(ftoold) fclose(ftoold);
                        almread = 0UL;
                        almage = 0UL;
                        osi_WrStrLn("almanach read error", 20ul);
                     }
                     if (rinexfile[0U]
                && (almage==0UL || gpstime>almage && gpstime-almage>almrequest)
                ) {
                        /* request a new almanach */
                        if (gpstime==0UL) WrRinexfn(systime);
                        else WrRinexfn(gpstime);
                     }
                     crdone = 0;
                  }
                  if (gpstime>0UL && gpstime>=almage) {
                     almanachage = gpstime-almage;
                  }
                  else {
                     almanachage = 0UL;
                  }
                  if (almage+maxalmage>gpstime) anonym0->posok = 1;
                  else if (almanachage>0UL) {
		     FILE *ftoold=fopen("/tmp/toold", "ab+");
		     if(ftoold) fclose(ftoold);
                     osic_WrINT32(almanachage/60UL, 10UL);
                     osi_WrStrLn(" Min (almanach too old)", 24ul);
                     if (almread+4UL<=systime) {
                        almread = systime-30UL;
                /* look often for new almanach */
                     }
                  }
               }
               crdone = 1;
            }
         }
         else if (typ=='h') {
            if (sf[0U]!='\003') {
	       contextr9.aux=1;
               if (sondeaprs_verb2) {
                  tmp = len-1UL;
                  j = 0UL;
                  if (j<=tmp) for (;; j++) {
                     osi_WrHex((uint32_t)(uint8_t)sf[j], 3UL);
                     if (j==tmp) break;
                  } /* end for */
                  osi_WrStrLn("", 1ul);
                  crdone = 1;
               }
               if (sf[0U]==0) {
                  doozon(sf, 256ul, contextr9.hp, &contextr9.ozontemp,
                &contextr9.ozon);
                  crdone = 1;
               }
            }
         }
         else if (sondeaprs_verb2) {
            tmp = len-1UL;
            j = 0UL;
            if (j<=tmp) for (;; j++) {
               osi_WrHex((uint32_t)(uint8_t)sf[j], 3UL);
               if (j==tmp) break;
            } /* end for */
            crdone = 0;
         }
         if (sondeaprs_verb && !crdone) {
            osi_WrStrLn("", 1ul);
            crdone = 1;
         }
      }
   }
   loop_exit:;
/*   if (((((contextr9.posok && calok) && almread+60UL>systime)
                && (((sendquick==2UL || sondeaprs_nofilter)
                || contextr9.calibok==0xFFFFFFFFUL)
                || (contextr9.calibok&0x1UL)!=0UL && sendquick==1UL))
                && contextr9.lat!=0.0) && contextr9.long0!=0.0) {
*/
    printf("TEMP: %f\t\tHUM: %f\t\t PRES: %f *****************\n",contextr9.temp,contextr9.hyg,contextr9.hp);

   if (contextr9.posok  && contextr9.lat!=0.0 && contextr9.long0!=0.0 && contextr9.heig==0.0 && contextr9.dir==0.0) {
	FILE *ftoold=fopen("/tmp/toold", "ab+");
	if(ftoold) fclose(ftoold);
   }

	 if(mhz>467.0 && mhz<491.0){
	    contextr9.lat=(50+contextr9.hp/1000)*1.7453292519943E-2;
	    contextr9.long0=(18+contextr9.hp/1000)*1.7453292519943E-2;
	    contextr9.posok=1;
	 }    


   if (contextr9.posok  && contextr9.lat!=0.0 && contextr9.long0!=0.0) {


      { /* with */
         struct CONTEXTR9 * anonym1 = &contextr9;
         if (!anonym1->mesok || anonym1->calibok!=0xFFFFFFFFUL) {
            anonym1->hp = 0.0;
            anonym1->hyg = 0.0;
            anonym1->temp = (double)X2C_max_real;
         }
		
         anonym1->framesent = 1;
        store_sonde_db( objname,frameno,anonym1->lat,anonym1->long0,anonym1->heig,anonym1->speed,anonym1->dir,anonym1->climb,9,2,0,anonym1->ozon,anonym1->aux,anonym1->hp,(double)mhz,0,anonym1->temp,0,anonym1->hyg);
        store_sonde_rs( objname,frameno,anonym1->lat,anonym1->long0,anonym1->heig,anonym1->speed,anonym1->dir,anonym1->climb,9,2,0,anonym1->ozon,anonym1->aux,anonym1->hp,(double)mhz,0,anonym1->temp,0,anonym1->hyg,usercall);
	if(saveLog) save_Slog( objname,frameno,anonym1->lat,anonym1->long0,anonym1->heig,anonym1->speed,anonym1->dir,anonym1->climb,9,2,0,anonym1->ozon,anonym1->aux,anonym1->hp,(double)mhz,0,anonym1->temp,0,anonym1->hyg);
      }
      crdone = 1;
   }
   if (sondeaprs_verb) {
      if (!crdone) osi_WrStrLn("", 1ul);
      osi_WrStrLn("------------", 13ul);
   }
} /* end decodeframe() */

/*------------------------------ C34 C50 */

static double latlong(uint32_t val, char c50)
{
   double hf;
   double hr;
   hr = (double)(float)(val%0x080000000UL);
   if (c50) hr = X2C_DIVL(hr,1.E+7);
   else hr = X2C_DIVL(hr,1.E+6);
   hf = (double)(float)(uint32_t)X2C_TRUNCC(hr,0UL,
                X2C_max_longcard);
   hr = hf+X2C_DIVL(hr-hf,0.6);
   if (val>=0x080000000UL) hr = -hr;
   return hr;
} /* end latlong() */

#define sondemod_MAXEXTEND 3.0
/* limit extrapolation range */

#define sondemod_MAXTIMESPAN 10

#define sondemod_MAXRANGE 4.7123889803847E-4
/* max jump in rad */


static double extrapolate(double yold, double y,
                uint32_t told, uint32_t t, uint32_t systime0,
                char * good)
{
   double maxex;
   double maxr;
   double dy;
   double k;
   uint32_t maxt;
   maxr = 4.7123889803847E-4;
   maxt = 10UL;
   maxex = 3.0;
   if (sondeaprs_nofilter) {
      maxr = 1.8849555921539E-3;
      maxt = 40UL;
      maxex = 12.0;
   }
   *good = 1;
   if (t>=systime0) return y;
   /* point is just in time */
   if (told<t) {
      k = (double)(X2C_DIVR((float)(systime0-told),
                (float)(t-told)));
      if (k>maxex || told+maxt<systime0) *good = 0;
      dy = y-yold;
      if (fabs(dy)>maxr) *good = 0;
      return yold+dy*k;
   }
   *good = 0;
   return y;
} /* end extrapolate() */


static double dist(double a, double b)
{
   double d;
   d = a-b;
   if (d>3.1415926535898) d = d-6.2831853071796;
   else if (d<(-3.1415926535898)) d = d+6.2831853071796;
   return d;
} /* end dist() */

#define sondemod_MINTV 8
/* min seconds for speed out of positions */

#define sondemod_VLIM 2.6164311878598E-5
/* max speed */


static void decodec34(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   OBJNAME nam;
   char cb[10];
   char s[1001];
   CALLSSID usercall;
   uint32_t val;
   uint32_t sum2;
   uint32_t sum1;
   uint32_t j;
   uint32_t i;
   double ve;
   double exlat;
   double exlon;
   double hr;
   pCONTEXTC34 pc0;
   pCONTEXTC34 pc1;
   pCONTEXTC34 pc;
   double stemp;
   char c50;
   char latok;
   char lonok;
   char posok;
   char tmp[10];
   double frq;
   struct CONTEXTC34 * anonym;
   struct CONTEXTC34 * anonym0;
   struct CONTEXTC34 * anonym1;
   struct CONTEXTC34 * anonym2;
   if (rxb[0UL]!='S' || rxb[1UL]!='C') return;
   /* no srsc34 frame */
   c50 = rxb[2UL]=='5'; /* is a sc50 */

    tmp[0]=rxb[22];
    tmp[1]=rxb[23];
    tmp[2]=rxb[24];
    tmp[3]='.';
    tmp[4]=rxb[25];
    tmp[5]=rxb[26];
    tmp[6]=rxb[27];
    tmp[7]=0;
    frq=atof(tmp);


   i = 0UL;
   do {
      nam[i] = rxb[i];
      ++i;
   } while (i<=8UL);
   if (nam[0U]==0) return;
   /* wait for id */
   ++i;
   j = 0UL;
   do {
      cb[j] = rxb[i];
      ++i;
      ++j;
   } while (j<=4UL);
   getcall(cb, 10ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   j = 0UL;
   do {
      cb[j] = rxb[i];
      ++i;
      ++j;
   } while (j<=9UL);
   sum1 = 0UL;
   sum2 = 65791UL;
   for (i = 0UL; i<=4UL; i++) {
      sum1 += (uint32_t)(uint8_t)cb[i];
      sum2 -= (uint32_t)(uint8_t)cb[i]*(5UL-i);
   } /* end for */
   sum1 = sum1&255UL;
   sum2 = sum2&255UL;
   if (sum1!=(uint32_t)(uint8_t)cb[5U] || sum2!=(uint32_t)(uint8_t)
                cb[6U]) return;
   /* chesum error */
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStr(" ", 2ul);
   }
   osi_WrStr(nam, 9ul);
   osi_WrStr(" ", 2ul);
   pc = pcontextc;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         /* timed out */
         if (pc0==0) pcontextc = pc1;
         else pc0->next = pc1;
         osic_free((char * *) &pc, sizeof(struct CONTEXTC34));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((char * *) &pc, sizeof(struct CONTEXTC34));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((char *)pc,(char)0,sizeof(struct CONTEXTC34));
      pc->next = pcontextc;
      pcontextc = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   pc->tused = systime;
   val = (uint32_t)(uint8_t)cb[4U]+(uint32_t)(uint8_t)
                cb[3U]*256UL+(uint32_t)(uint8_t)
                cb[2U]*65536UL+(uint32_t)(uint8_t)cb[1U]*16777216UL;
   hr = (double)*X2C_CAST(&val,uint32_t,float,float *);
   posok = 0;
   if (c50) {
      switch ((unsigned)cb[0U]) {
      case '\003':
         if (hr<99.9 && hr>(-99.9)) {
            if (sondeaprs_verb) {
               osi_WrStr("temp ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("oC", 3ul);
            }
            pc->temp = hr;
            pc->ttemp = systime;
         }
         break;
      case '\024':
         if (sondeaprs_verb) {
            osi_WrStr("date", 5ul);
            aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL, s,
                1001ul);
            s[0U] = ' ';
            osi_WrStr(s, 1001ul);
         }
         break;
      case '\025':
         pc->gpstime = (val/10000UL)*3600UL+((val%10000UL)/100UL)
                *60UL+val%100UL;
         pc->tgpstime = systime;
         if (sondeaprs_verb) {
            aprsstr_TimeToStr(pc->gpstime, s, 1001ul);
            osi_WrStr("time ", 6ul);
            osi_WrStr(s, 1001ul);
         }
         break;
      case '\026':
         hr = latlong(val, c50);
         if (hr<89.9 && hr>(-89.9)) {
            if (sondeaprs_verb) {
               osi_WrStr("lat  ", 6ul);
               osic_WrFixed((float)hr, 5L, 0UL);
            }
            if (pc->tlat!=systime) {
               { /* with */
                  struct CONTEXTC34 * anonym = pc;
                  anonym->lat1 = anonym->lat;
                  anonym->tlat1 = anonym->tlat;
                  anonym->lat = hr*1.7453292519943E-2;
                  anonym->tlat = systime;
                  if (anonym->tlat<anonym->tlatv1) {
                     anonym->tlatv1 = anonym->tlat;
                /* repair back jumped time */
                  }
                  if (anonym->tlat>anonym->tlatv1+8UL) {
                     /* south-north speed */
                     ve = X2C_DIVL(dist(anonym->lat, anonym->latv1),
                (double)(anonym->tlat-anonym->tlatv1));
                     /*WrStr(" ");WrFixed(ve*(EARTH*1000), 1, 9);
                WrStr("VTn"); */
                     if (fabs(ve)<=2.6164311878598E-5) {
                        anonym->vlat = anonym->vlat+(ve-anonym->vlat)*0.5;
                     }
                     anonym->latv1 = anonym->lat;
                     anonym->tlatv1 = anonym->tlat;
                  }
               }
               posok = 1;
            }
         }
         break;
      case '\027':
         hr = latlong(val, c50);
         if (hr<180.0 && hr>(-180.0)) {
            if (sondeaprs_verb) {
               osi_WrStr("long ", 6ul);
               osic_WrFixed((float)hr, 5L, 0UL);
            }
            if (pc->tlon!=systime) {
               { /* with */
                  struct CONTEXTC34 * anonym0 = pc;
                  anonym0->lon1 = anonym0->lon;
                /* save 2 values for extrapolating */
                  anonym0->tlon1 = anonym0->tlon;
                  anonym0->lon = hr*1.7453292519943E-2;
                  anonym0->tlon = systime;
                  if (anonym0->tlon<anonym0->tlonv1) {
                     anonym0->tlonv1 = anonym0->tlon;
                /* repair back jumped time */
                  }
                  if (anonym0->tlat>0UL && anonym0->tlon>anonym0->tlonv1+8UL)
                 {
                     /* east-west speed */
                     ve = X2C_DIVL(dist(anonym0->lon,
                anonym0->lonv1)*(double)osic_cos((float)
                anonym0->lat),(double)(anonym0->tlon-anonym0->tlonv1));
                     /*WrStr(" ");WrFixed(ve*(EARTH*1000), 1, 9);
                WrStr("VTe"); */
                     if (fabs(ve)<=2.6164311878598E-5) {
                        anonym0->vlon = anonym0->vlon+(ve-anonym0->vlon)*0.5;
                     }
                     anonym0->lonv1 = anonym0->lon;
                     anonym0->tlonv1 = anonym0->tlon;
                  }
               }
               posok = 1;
            }
         }
         break;
      case '\030':
         hr = (double)((float)val*0.1f);
         if (hr<50000.0) {
            if (sondeaprs_verb) {
               osi_WrStr("alti ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("m", 2ul);
            }
            if (pc->talt<systime) {
               pc->clmb = pc->clmb+(X2C_DIVL(hr-pc->alt,
                (double)(float)(systime-pc->talt))-pc->clmb)*0.25;
            }
            pc->alt = hr;
            pc->talt = systime;
         }
         break;
      default:;
         if (sondeaprs_verb) {
            osi_WrHex((uint32_t)(uint8_t)cb[0U], 0UL);
            osi_WrStr(" ", 2ul);
            osi_WrHex((uint32_t)(uint8_t)cb[1U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[2U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[3U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[4U], 0UL);
            osic_WrFixed((float)hr, 2L, 10UL);
         }
         break;
      } /* end switch */
      { /* with */
         struct CONTEXTC34 * anonym1 = pc;
         anonym1->tspeed = systime;
         anonym1->tdir = systime;
         anonym1->speed = (double)(osic_sqrt((float)
                (anonym1->vlon*anonym1->vlon+anonym1->vlat*anonym1->vlat))
                *6.37E+6f); /* speed out of moved distance km/h */
         anonym1->dir = atan20(anonym1->vlat, anonym1->vlon);
         if (anonym1->dir<0.0) anonym1->dir = anonym1->dir+6.2831853071796;
         anonym1->dir = anonym1->dir*5.7295779513082E+1;
      }
   }
   else {
      /*WrStrLn(""); WrStr("vlat,vlon spd, dir:");
                WrFixed(vlat*(EARTH*3600), 1,7); */
      /*WrFixed(vlon*(EARTH*3600), 1,7); WrFixed(speed*3.6, 1,8);
                WrFixed(dir, 1,9) ; WrFixed(alt, 1,9); */
      /* SC34 */
      switch ((unsigned)cb[0U]) {
      case '\003':
         if (hr<99.9 && hr>(-99.9)) {
            if (sondeaprs_verb) {
               osi_WrStr("temp ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("oC", 3ul);
            }
            pc->temp = hr;
            pc->ttemp = systime;
         }
         break;
      case '\024':
         /*
              |CHR(07H): IF (hr<99.9) & (hr>-99.9) THEN 
                           IF verb THEN WrStr("dewp "); WrFixed(hr, 1, 0);
                WrStr("oC"); END;
                           pc^.dewp:=hr;
                           pc^.tdewp:=systime;
                         END;
         */
         if (sondeaprs_verb) {
            osi_WrStr("date", 5ul);
            aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL, s,
                1001ul);
            s[0U] = ' ';
            osi_WrStr(s, 1001ul);
         }
         break;
      case '\025':
         pc->gpstime = (val/10000UL)*3600UL+((val%10000UL)/100UL)
                *60UL+val%100UL;
         pc->tgpstime = systime;
         if (sondeaprs_verb) {
            aprsstr_TimeToStr(pc->gpstime, s, 1001ul);
            osi_WrStr("time ", 6ul);
            osi_WrStr(s, 1001ul);
         }
         break;
      case '\026':
         hr = latlong(val, c50);
         if (hr<89.9 && hr>(-89.9)) {
            if (sondeaprs_verb) {
               osi_WrStr("lati ", 6ul);
               osic_WrFixed((float)hr, 5L, 0UL);
            }
            if (pc->tlat!=systime) {
               pc->lat1 = pc->lat;
               pc->tlat1 = pc->tlat;
               pc->lat = hr*1.7453292519943E-2;
               pc->tlat = systime;
               posok = 1;
            }
         }
         break;
      case '\027':
         hr = latlong(val, c50);
         if (hr<180.0 && hr>(-180.0)) {
            if (sondeaprs_verb) {
               osi_WrStr("long ", 6ul);
               osic_WrFixed((float)hr, 5L, 0UL);
            }
            if (pc->tlon!=systime) {
               pc->lon1 = pc->lon; /* save 2 values for extrapolating */
               pc->tlon1 = pc->tlon;
               pc->lon = hr*1.7453292519943E-2;
               pc->tlon = systime;
               posok = 1;
            }
         }
         break;
      case '\030':
         hr = (double)((float)val*0.1f);
         if (hr<50000.0) {
            if (sondeaprs_verb) {
               osi_WrStr("alti ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("m", 2ul);
            }
            if (pc->talt<systime) {
               pc->clmb = pc->clmb+(X2C_DIVL(hr-pc->alt,
                (double)(float)(systime-pc->talt))-pc->clmb)*0.25;
            }
            pc->alt = hr;
            pc->talt = systime;
         }
         break;
      case '\031':
         hr = (double)((float)val*1.851984E-1f);
                /*1.609*/ /*1.852*/ /* guess knots or miles */
         if (hr>=0.0 && hr<1000.0) {
            if (sondeaprs_verb) {
               osi_WrStr("wind ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("km/h", 5ul);
            }
            pc->speed = hr*2.7777777777778E-1;
            pc->tspeed = systime;
         }
         break;
      case '\032':
         hr = (double)((float)val*0.1f);
         if (hr>=0.0 && hr<=360.0) {
            if (sondeaprs_verb) {
               osi_WrStr("wdir ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("deg", 4ul);
            }
            pc->dir = hr;
            pc->tdir = systime;
         }
         break;
      default:;
         if (sondeaprs_verb) {
            osi_WrHex((uint32_t)(uint8_t)cb[0U], 0UL);
            osi_WrStr(" ", 2ul);
            osi_WrHex((uint32_t)(uint8_t)cb[1U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[2U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[3U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[4U], 0UL);
            osic_WrFixed((float)hr, 2L, 10UL);
         }
         break;
      } /* end switch */
   }
   { /* with */
      struct CONTEXTC34 * anonym2 = pc;
/*      if (posok && (sondeaprs_nofilter || (((((anonym2->lastsent!=systime && anonym2->tlon+8UL>systime)
                 && anonym2->tlat+8UL>systime) && anonym2->talt+20UL>systime)
                 && anonym2->tspeed+120UL>systime)
                && anonym2->tdir+120UL>systime)
                && anonym2->tgpstime+120UL>systime)) {
*/
      if (posok){
         if (anonym2->ttemp+30UL>systime) stemp = anonym2->temp;
         else stemp = (double)X2C_max_real;
         exlon = extrapolate(anonym2->lon1, anonym2->lon, anonym2->tlon1,
                anonym2->tlon, systime, &lonok);
         exlat = extrapolate(anonym2->lat1, anonym2->lat, anonym2->tlat1,
                anonym2->tlat, systime, &latok);
         /*
         IF lonok THEN WrStrLn("--good ") ELSE WrStrLn("--bad  ") END;
         WrInt(systime-tlon1, 10); WrInt(systime-tlon, 10);
         WrFixed(lon1/RAD, 5,0); WrStr(" ");WrFixed(lon/RAD, 5,0);
                WrStr(" ");
         WrFixed(exlon/RAD, 5,0); WrStrLn("t1 t x1 x xext");
         */
         if (lonok && latok) {
            anonym2->lastsent = systime;
	    int tc=3;
	    if(anonym2->name[2]=='5') tc=5;
            store_sonde_db(anonym2->name,0,exlat,exlon,anonym2->alt,anonym2->speed,anonym2->dir,anonym2->clmb,tc,2,0,0.0,0,0,frq,0,0,0,0);
            store_sonde_rs(anonym2->name,0,exlat,exlon,anonym2->alt,anonym2->speed,anonym2->dir,anonym2->clmb,tc,2,0,0.0,0,0,frq,0,0,0,0,usercall);
	    if(saveLog) save_Slog(anonym2->name,0,exlat,exlon,anonym2->alt,anonym2->speed,anonym2->dir,anonym2->clmb,tc,2,0,0.0,0,0,frq,0,0,0,0);
         }
      }
   }
   if (sondeaprs_verb) osi_WrStrLn("", 1ul);
} /* end decodec34() */

/*------------------------------ DFM06 */


static uint32_t sondemod_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};


static void decodedfm6(const char rxb[], uint32_t rxb_len, uint32_t ip, uint32_t fromport)
{
   uint32_t rt;
   pCONTEXTDFM6 pc0;
   pCONTEXTDFM6 pc1;
   pCONTEXTDFM6 pc;
   char cb[10];
   char s[1001];
   CALLSSID usercall;
   double frq;
   char tmp[20];
   uint32_t gpstime;

   double lat,lon,alt,vH,vV,Dir,T,T1,Vcc;
   int yr,mon,day,hr,min,sec;
   unsigned int frno,i,j,ok=0;
   char id[20];
   char typ[10];
   int  typm=6;
    printf("DFM: %c%c%c\n",rxb[0UL],rxb[1UL],rxb[2UL]);

   if (((rxb[0UL]!='D')||(rxb[0UL]!='E'))&&( (rxb[1UL]!='6') && (rxb[1UL]!='9') && (rxb[1UL]!='D') && (rxb[1UL]!='F') && (rxb[1UL]!='X'))) return;

    if(rxb[1UL]=='9') typm=7;
    else if(rxb[1UL]=='F') typm=15;
    else if(rxb[1UL]=='D') typm=17;
    
    if(rxb[0UL]=='E') typm+=100;

    sprintf(typ,"DFM0%c",rxb[1UL]);
    tmp[0]=rxb[0];    tmp[1]=rxb[1];    tmp[2]=rxb[2];    tmp[3]=rxb[3];    tmp[4]=rxb[4];    tmp[5]=rxb[5];    tmp[6]=rxb[6];
    tmp[7]=rxb[7];    tmp[8]=rxb[8];    tmp[9]=rxb[9];    tmp[10]=0;
    strcpy(id,tmp);

    tmp[0]=rxb[10];    tmp[1]=rxb[11];    tmp[2]=rxb[12];    tmp[3]=rxb[13];    tmp[4]=0;
    frno=atoi(tmp);

    tmp[0]=rxb[14];    tmp[1]=rxb[15];    tmp[2]='.';    tmp[3]=rxb[16];    tmp[4]=rxb[17];    tmp[5]=rxb[18];
    tmp[6]=rxb[19];    tmp[7]=rxb[20];    tmp[8]=rxb[21];    tmp[9]=0;
    lat=atof(tmp)*1.7453292519943E-2;

    tmp[0]=rxb[22];    tmp[1]=rxb[23];    tmp[2]=rxb[24];    tmp[3]='.';    tmp[4]=rxb[25];    tmp[5]=rxb[26];
    tmp[6]=rxb[27];    tmp[7]=rxb[28];    tmp[8]=rxb[29];    tmp[9]=rxb[30];    tmp[10]=0;
    lon=atof(tmp)*1.7453292519943E-2;

    tmp[0]=rxb[31];    tmp[1]=rxb[32];    tmp[2]=rxb[33];    tmp[3]=rxb[34];    tmp[4]=rxb[35];    tmp[5]=0;
    alt=atoi(tmp);

    tmp[0]=rxb[36];    tmp[1]=rxb[37];    tmp[2]=rxb[38];    tmp[3]='.';    tmp[4]=rxb[39];    tmp[5]=rxb[40];  tmp[6]=0;
    vH=atof(tmp);

    tmp[0]=rxb[41];    tmp[1]=rxb[42];    tmp[2]=rxb[43];    tmp[3]='.';    tmp[4]=rxb[44];    tmp[5]=0;
    Dir=atof(tmp);

    tmp[0]=rxb[45];    tmp[1]=rxb[46];    tmp[2]=rxb[47];    tmp[3]='.';    tmp[4]=rxb[48];    tmp[5]=rxb[49];  tmp[6]=0;
    vV=atof(tmp);

    tmp[0]=rxb[50];    tmp[1]=rxb[51];    tmp[2]=rxb[52];    tmp[3]='.';    tmp[4]=rxb[53];    tmp[5]=rxb[54];    tmp[6]=rxb[55];    tmp[7]=0;
    frq=(double)atof(tmp);

    tmp[0]=rxb[56];    tmp[1]=rxb[57];    tmp[2]=rxb[58];    tmp[3]=rxb[59];    tmp[4]=0;
    T= atoi(tmp)/10.0-273.0;

    tmp[0]=rxb[60];    tmp[1]=rxb[61];    tmp[2]=rxb[62];    tmp[3]=rxb[63];    tmp[4]=0;
    Vcc= atoi(tmp)/100.0;

    tmp[0]=rxb[64];    tmp[1]=rxb[65];    tmp[2]=rxb[66];    tmp[3]=rxb[67];    tmp[4]=rxb[68];   tmp[5]=0;
    T1= atoi(tmp)/100.0-273.0;

    tmp[0]=rxb[69];    tmp[1]=rxb[70];    tmp[2]=rxb[71];    tmp[3]=rxb[72];    tmp[4]=0;
    yr= atoi(tmp);

    tmp[0]=rxb[73];    tmp[1]=rxb[74];    tmp[2]=0;
    mon= atoi(tmp);

    tmp[0]=rxb[75];    tmp[1]=rxb[76];    tmp[2]=0;
    day= atoi(tmp);

    tmp[0]=rxb[77];    tmp[1]=rxb[78];    tmp[2]=0;
    hr= atoi(tmp);

    tmp[0]=rxb[79];    tmp[1]=rxb[80];    tmp[2]=0;
    min= atoi(tmp);

    tmp[0]=rxb[81];    tmp[1]=rxb[82];    tmp[2]=0;
    sec= atoi(tmp);

    tmp[0]=rxb[83]; tmp[1]=rxb[84]; tmp[2]=rxb[85]; tmp[3]=rxb[86]; tmp[4]=rxb[87]; tmp[5]=0;

   getcall(tmp, 6, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);

   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }

   frno=sec+min*60+hr*3600+day*86400;
   printf("%s[%i][%04i-%02i-%02i %02i:%02i:%02i]: La:%f, Lo:%f, Alt:%5.0f vH:%5.2f vV:%5.2f D:%3.1f T:%4.2f T1:%4.2f Vcc:%4.2f %6.3fMHz\r\n",id,frno,yr,mon,day,hr,min,sec,lat/1.7453292519943E-2,lon/1.7453292519943E-2,alt,vH,vV,Dir,T,T1,Vcc,frq);


   rt = 0UL;
   for (j = 0UL; j<=3UL; j++) {
      rt = rt*256UL+(uint32_t)(uint8_t)rxb[i];
      ++i;
   }

   pc = pcontextdfm6;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         if (pc0==0) pcontextdfm6 = pc1;
         else pc0->next = pc1;
         osic_free((char * *) &pc, sizeof(struct CONTEXTDFM6));
      }
      else {
         if (aprsstr_StrCmp(id+2, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((char * *) &pc, sizeof(struct CONTEXTDFM6));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((char *)pc,(char)0,sizeof(struct CONTEXTDFM6));
      pc->next = pcontextdfm6;
      pcontextdfm6 = pc;
      aprsstr_Assign(pc->name, 9ul, id+2, 9ul);
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   gpstime=sec+min*60+hr*3600+86382UL;
   if(gpstime > pc->gpstime){
    pc->gpstime=gpstime;
    ok=1;
   }
    pc->tused = systime;
    pc->posok = 1;

    { struct CONTEXTDFM6 * anonym = X2C_CHKNIL(pCONTEXTDFM6,pc);



        if(lat>0 && lon>0 && alt>=0){ // && ok>0

            anonym->lastsent = osic_time();
            store_sonde_db(id+2,frno,lat,lon,alt,vH,Dir,vV,typm,2,0,0.0,0,0.0,frq,Vcc,T,T1,0); 
            store_sonde_rs(id+2,frno,lat,lon,alt,vH,Dir,vV,typm,2,0,0.0,0,0.0,frq,Vcc,T,T1,0,usercall); 
	    if(saveLog) save_Slog(id+2,frno,lat,lon,alt,vH,Dir,vV,typm,2,0,0.0,0,0.0,frq,Vcc,T,T1,0); 
        }
    }
    

} /* end decodedfm6() */

/*------------------------------ RS41 */

static void WrChChk(char ch)
{
   if ((uint8_t)ch>=' ' && (uint8_t)ch<'\177') {
      osi_WrStr((char *) &ch, 1u/1u);
   }
} /* end WrChChk() */

#define sondemod_EARTHA 6.378137E+6

#define sondemod_EARTHB 6.3567523142452E+6

#define sondemod_E2 6.6943799901413E-3

#define sondemod_EARTHAB 4.2841311513312E+4


static void wgs84r(double x, double y, double z,
                double * lat, double * long0,
                double * heig)
{
   double sl;
   double ct;
   double st;
   double t;
   double rh;
   double xh;
   double h;
   h = x*x+y*y;
   if (h>0.0) {
      rh = (double)osic_sqrt((float)h);
      xh = x+rh;
      *long0 = atan20(xh, y)*2.0;
      if (*long0>3.1415926535898) *long0 = *long0-6.2831853071796;
      t = (double)osic_arctan((float)(X2C_DIVL(z*1.003364089821,
                rh)));
      st = (double)osic_sin((float)t);
      ct = (double)osic_cos((float)t);
      *lat = (double)osic_arctan((float)
                (X2C_DIVL(z+4.2841311513312E+4*st*st*st,
                rh-4.269767270718E+4*ct*ct*ct)));
      sl = (double)osic_sin((float)*lat);
      *heig = X2C_DIVL(rh,(double)osic_cos((float)*lat))-(double)(X2C_DIVR(6.378137E+6f,
                osic_sqrt((float)(1.0-6.6943799901413E-3*sl*sl))));
   }
   else {
      *lat = 0.0;
      *long0 = 0.0;
      *heig = 0.0;
   }
} /* end wgs84r() */


static int32_t getint32(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   uint32_t n;
   uint32_t i;
   n = 0UL;
   for (i = 3UL;; i--) {
      n = n*256UL+(uint32_t)(uint8_t)frame[p+i];
      if (i==0UL) break;
   } /* end for */
   return (int32_t)n;
} /* end getint32() */


static uint32_t getcard16(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   return (uint32_t)(uint8_t)frame[p]+256UL*(uint32_t)(uint8_t)
                frame[p+1UL];
} /* end getcard16() */


static int32_t getint16(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   uint32_t n;
   n = (uint32_t)(uint8_t)frame[p]+256UL*(uint32_t)(uint8_t)
                frame[p+1UL];
   if (n>=32768UL) return (int32_t)(n-65536UL);
   return (int32_t)n;
} /* end getint16() */

static int32_t getint24(const char frame[], uint32_t frame_len, uint32_t p)
{
   uint32_t n;
   n = (uint32_t)(uint8_t)frame[p]+ 256UL*(uint32_t)(uint8_t)frame[p+1UL] +512UL*(uint32_t)(uint8_t)frame[p+2UL];
   if (n>=8388608UL) return (int32_t)(n-16777216UL);
   return (int32_t)n;
} /* end getint16() */


static uint32_t gethex(const char frame[], uint32_t frame_len,
                uint32_t p, uint32_t nibb)
{
   uint32_t c;
   uint32_t n;
   n = 0UL;
   while (nibb>0UL) {
      n = n*16UL;
      /*WrStr("<<"); WrStr(frame[p]); WrStr(">>"); */
      c = (uint32_t)(uint8_t)frame[p];
      if (c>=48UL && c<=57UL) n += c-48UL;
      else if (c>=65UL && c<=70UL) n += c-55UL;
      else return 0UL;
      ++p;
      --nibb;
   }
   /*WrInt(n,5); */
   return n;
} /* end gethex() */


static void posrs41(const char b[], uint32_t b_len, uint32_t p,
                double * lat, double * long0,
                double * heig, double * speed,
                double * dir, double * clmb)
{
   double vu;
   double ve;
   double vn;
   double vz;
   double vy;
   double vx;
   double z;
   double y;
   double x;
   x = (double)getint32(b, b_len, p)*0.01;
   y = (double)getint32(b, b_len, p+4UL)*0.01;
   z = (double)getint32(b, b_len, p+8UL)*0.01;
   wgs84r(x, y, z, lat, long0, heig);
   if (sondeaprs_verb) {
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)(X2C_DIVL(*lat,1.7453292519943E-2)), 5L, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)(X2C_DIVL(*long0,1.7453292519943E-2)), 5L, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)*heig, 1L, 1UL);
      osi_WrStr("m ", 3ul);
   }
   if (*heig<(-500.0) || *heig>50000.0) {
      *lat = 0.0;
      *long0 = 0.0;
      *heig = 0.0;
   }
   /*speed */
   vx = (double)getint16(b, b_len, p+12UL)*0.01;
   vy = (double)getint16(b, b_len, p+14UL)*0.01;
   vz = (double)getint16(b, b_len, p+16UL)*0.01;
   vn = (-(vx*(double)osic_sin((float)*lat)*(double)
                osic_cos((float)*long0))-vy*(double)
                osic_sin((float)*lat)*(double)osic_sin((float)
                *long0))+vz*(double)osic_cos((float)*lat);
   ve = -(vx*(double)osic_sin((float)*long0))+vy*(double)
                osic_cos((float)*long0);
   vu = vx*(double)osic_cos((float)*lat)*(double)
                osic_cos((float)*long0)+vy*(double)
                osic_cos((float)*lat)*(double)osic_sin((float)
                *long0)+vz*(double)osic_sin((float)*lat);
   *dir = X2C_DIVL(atan20(vn, ve),1.7453292519943E-2);
   if (*dir<0.0) *dir = 360.0+*dir;
   *speed = (double)osic_sqrt((float)(vn*vn+ve*ve));
   *clmb = vu;
   if (sondeaprs_verb) {
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)( *speed*3.6), 2L, 1UL);
      osi_WrStr("km/h ", 6ul);
      osic_WrFixed((float)*dir, 1L, 1UL);
      osi_WrStr("deg ", 5ul);
      osic_WrFixed((float)vu, 1L, 1UL);
      osi_WrStr("m/s", 4ul);
   }
} /* end posrs41() */


static double altToPres(double a)
/* meter to hPa */
{
   if (a<=0.0) return 1010.0;
   else if (a>40000.0) return 0.0;
   else if (a>15000.0) {
      return (double)(osic_exp((float)(a*(-1.5873015873016E-4)
                +0.2629))*1000.0f);
   }
   else {
      return (double)(1010.0f*osic_exp(osic_ln((float)
                ((293.0-0.0065*a)*3.4129692832765E-3))*5.26f));
   }
   return 0;
} /* end altToPres() */


static double calcOzone(double uA, double temp,
                double airpres)
{
   return 4.307E-4*uA*(temp+273.15)*28.57*getOzoneCorr(airpres);
/*
        From Mast/Keystone ozonsensor 730-10 datasheet:
                1 uA per 50 umb Ozone (1 uA per 5 mPa)
                Airflow 190-230 ml/min (avg. 210 ml/min => 3.5 ml/s => 100 ml in 28,
                57 s

        Default ozone formula from ftp://ftp.cpc.ncep.noaa.gov/ndacc/meta/sonde/cv_payerne_snd.txt

                POZ(nb)  = 0.004307 * i * Tp * t * E(p)
                => POZ(mPa)  = 0.0004307 * i * Tp * t * E(p)

                where:  i is the current from the sensor in uA
                         t is the time in seconds to pump 0.100 liters of air through the pump
                         E(p) is the pump efficiency correction
                         Tp is the pump temperature
*/
} /* end calcOzone() */

static uint16_t sondemod_POLYNOM0 = 0x1021U;

static uint16_t sondemod_burstIndicatorBytes[12] = {2U,262U,276U,391U,306U,
                0U,0U,0U,255U,255U,0U,0U};

static uint16_t _cnst1[12] = {2U,262U,276U,391U,306U,0U,0U,0U,255U,255U,0U,
                0U};

static void decoders41(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   OBJNAME nam;
   int32_t res;
   char s[1001];
   CALLSSID usercall;
   uint32_t frameno;
   uint32_t len;
   uint32_t p;
   uint32_t ic;
   uint32_t j;
   uint32_t i;
   char nameok;
   char calok;
   uint16_t crc;
   char typ;
   pCONTEXTR4 pc0;
   pCONTEXTR4 pc1;
   pCONTEXTR4 pc;
   double ozonval;
   double climb;
   double dir;
   double speed;
   double heig;
   double long0;
   double lat;
   uint32_t tmp;
   calok = 0;
   nameok = 0;
   nam[0U] = 0;
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
   ozonval = 0.0;

   //SKP
   char serialNumber[6];
   unsigned long firmwareVersion;
   unsigned long version;
   unsigned long subversionMajor;
   unsigned long subversionMinor;

   getcall(rxb, rxb_len, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);

   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }
   p = 57UL;



   if (sondeaprs_verb) osi_WrStr("R41 ", 5ul);
   for (;;) {
//    for(int i=0;i<100;i++)
//	printf("%c:",rxb[i]);
//	printf("[%i]:%c ",i,rxb[i]);

      if (p+4UL>=rxb_len-1) break;
      typ = rxb[p];
//	printf("TYP:%c\n",typ);
      ++p;
      len = (uint32_t)(uint8_t)rxb[p]+2UL;
      ++p;
      if (p+len>=rxb_len-1) break;
      /*
      WrStrLn("");
      FOR i:=0 TO len-1 DO WrHex(ORD(rxb[p+i]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;
      WrStrLn("");
      */
      j = 0UL;
      crc = 0xFFFFU;
      while (j<len && p+j<rxb_len-1) {
         if (j+2UL<len) {
            for (ic = 0UL; ic<=7UL; ic++) {
               if (((0x8000U & crc)!=0)!=X2C_IN(7UL-ic,8,
                (uint8_t)(uint8_t)rxb[p+j])) {
                  crc = X2C_LSH(crc,16,1)^0x1021U;
               }
               else crc = X2C_LSH(crc,16,1);
            } /* end for */
         }
         ++j;
      }
      if ((char)crc!=rxb[(p+len)-2UL] || (char)X2C_LSH(crc,16,
                -8)!=rxb[(p+len)-1UL]) {
         if (sondeaprs_verb) osi_WrStr(" ----  crc err ", 16ul);
         break;
      }
      if (typ=='y') {
         nameok = 1;
         for (i = 0UL; i<=7UL; i++) {
            nam[i] = rxb[p+2UL+i];
            if ((uint8_t)nam[i]<=' ' || (uint8_t)nam[i]>'Z') nameok = 0;
         } /* end for */
         nam[8U] = 0;
         pc = pcontextr4;
         pc0 = 0;
         for (;;) {
            if (pc==0) break;
            pc1 = pc->next;
            if (pc->tused+3600UL<systime) {
               /* timed out */
               if (pc0==0) pcontextr4 = pc1;
               else pc0->next = pc1;
               osic_free((char * *) &pc, sizeof(struct CONTEXTR4));
            }
            else {
               if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
               pc0 = pc;
            }
            pc = pc1;
         }
         if (pc==0) {
            osic_alloc((char * *) &pc, sizeof(struct CONTEXTR4));
            if (pc==0) Error("allocate context out im memory", 31ul);
            memset((char *)pc,(char)0,sizeof(struct CONTEXTR4));
            pc->next = pcontextr4;
            pcontextr4 = pc;
            aprsstr_Assign(pc->name, 9ul, nam, 9ul);
            if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
         }
         frameno = (uint32_t)getint16(rxb, rxb_len, p);
	 
	
         if (frameno>pc->framenum) {
            /* new frame number */
            pc->framesent = 0;
            calok = 1;
            pc->framenum = frameno;
            pc->tused = systime;
	    pc->vbat = (float)(getint24(rxb, rxb_len, p+10))/10.0;
	     printf("VBAT:%f\n",pc->vbat);
         }
         else if (pc->framenum==frameno && !pc->framesent) calok = 1;
         else if (frameno<pc->framenum && sondeaprs_verb) {
            osi_WrStrLn("", 1ul);
            osi_WrStr("got out of order frame number ", 31ul);
            osic_WrINT32(frameno, 1UL);
            osi_WrStr(" expecting ", 12ul);
            osic_WrINT32(pc->framenum, 1UL);
	    osi_WrStrLn(" ", 1ul);
         }
         if (rxb[p+23UL]==0) {
            pc->mhz0 = (float)(getcard16(rxb, rxb_len,
                p+26UL)/64UL+40000UL)*0.01f+0.0005f;
         }

	 if (rxb[p+23UL]==1UL) {

                for (i = 0UL; i<=4UL; i++) {
                      serialNumber[i] = rxb[p+24UL+i];
                      if ((unsigned char)serialNumber[i]<=' ' || (unsigned char)serialNumber[i]>'Z') {
                          break;
                          //nameok = 0;
                      }
                } /* end for */
                serialNumber[i] = 0;
                if (nameok) {
                      aprsstr_Assign(pc->serialNumber, 6ul, serialNumber, 6ul);
                }
                //firmware wersion
                pc->swVersion=(unsigned long)(getcard16(rxb, rxb_len,p+29UL));

                if (sondeaprs_verb) {
            	    osi_WrStr(", SerNum=", 10ul);
                    osic_WrStr(pc->serialNumber,6ul);

                    version=pc->swVersion / 10000;
                    subversionMajor=(pc->swVersion-(version*10000)) / 100;
                    subversionMinor=pc->swVersion-(version*10000)-(subversionMajor*100);

                    osi_WrStr(", SwVersion=", 13ul);
                    if (version<10){
                        osi_WrStr("0", 2ul);
                        osic_WrUINT32(version,1ul);
                    } else 
			osic_WrUINT32(version,2ul);
            	    osi_WrStr(".", 2ul);

                    if (subversionMajor<10){
                         osi_WrStr("0", 2ul);
                         osic_WrUINT32(subversionMajor,1ul);
                    } else 
			 osic_WrUINT32(subversionMajor,2ul);
                    osi_WrStr(".", 2ul);

                    if (subversionMinor<10){
                         osi_WrStr("0", 2ul);
                         osic_WrUINT32(subversionMinor,1ul);
                    } else 
			osic_WrUINT32(subversionMinor,2ul);
                }
	 }

         if (sondeaprs_verb) {
            osi_WrStr(nam, 9ul);
	    osi_WrStr(" ", 2ul);
            osic_WrINT32(pc->framenum, 0UL);
         }
         /*i:=0;WHILE (i<=11) DO WrHex(ORD(rxb[p+23+i]), 3); INC(i) END; */
         /* appended by SQ7BR BURST KILL CHECK */
         i = 0UL;
         while (i<=11UL && (_cnst1[i]>=256U || rxb[p+23UL+i]==(char)
                _cnst1[i])) ++i;
         if (i>11UL) {
            pc->burstKill = ((uint32_t)(uint8_t)rxb[p+35UL]&1UL)+1UL;
            if (sondeaprs_verb) {
               osi_WrStr(" BK=", 5ul);
               osic_WrINT32(pc->burstKill-1UL, 1UL);
            }
         }
      }
      else if (typ=='z') {
      }
      else if (typ=='|') {
         /*
                 // 02 06 14 87 32 00 00 00 FF FF 00 00    01
                    int bkSign=0;
                   for (i = 0UL; i<=11UL;
                i++) {         // 8 znakow nazwy od pozycji 61(59+2)
                do 68(59+2+7)
                      if ( rxb[p+23UL+i]== burstIndicatorBytes[i] ) bkSign++;
                   } //for
                   if (bkSign==12) {
                     pc->burstKill =(unsigned long)
                (rxb[p+23UL+12UL] && 0x01UL)+1UL;
                     osi_WrStr(" BK=",5ul);
                     osic_WrINT32(pc->burstKill, 1UL);
                     osi_WrStrLn("",1ul);
                   }
         */
         /* appended by SQ7BR */
         /*             WrStrLn("7A frame"); */
         /*             WrStrLn("7C frame"); */
         if (pc) {
            pc->gpssecond = (uint32_t)((getint32(rxb, rxb_len,
                p+2UL)/1000L+86382L)%86400L); /* gps TOW */
         }
      }
      else if (typ=='}') {
      }
      else if (typ=='{') {
//	printf("POSRS41\n");
         /*             WrStrLn("7D frame"); */
         /*             WrStrLn("7B frame"); */
    //     if (pc) {
            posrs41(rxb, rxb_len, p, &lat, &long0, &heig, &speed, &dir,
                &climb);
            pc->hp = altToPres(heig);
                /* make hPa out of gps alt for ozone */
    //     }
      }
      else if (typ=='~') {
         /* external device */
         if (len==23UL) {
            /* ozon values */
            if (pc) {
               /*          pc^.ozonInstType:=gethex(rxb, p+1, 2); */
               /*          pc^.ozonInstNum:=gethex(rxb, p+3, 2); */
               res = (int32_t)gethex(rxb, rxb_len, p+5UL, 4UL);
               if (res>=32768L) {
                  res = 32768L-res;
               }
               pc->ozonTemp = (double)res*0.01;
               pc->ozonuA = (double)gethex(rxb, rxb_len, p+9UL,
                5UL)*0.0001;
               pc->ozonBatVolt = (double)gethex(rxb, rxb_len, p+14UL,
                2UL)*0.1;
               pc->ozonPumpMA = (double)gethex(rxb, rxb_len, p+16UL,
                3UL);
               pc->ozonExtVolt = (double)gethex(rxb, rxb_len, p+19UL,
                2UL)*0.1;
               ozonval = calcOzone(pc->ozonuA, pc->ozonTemp, pc->hp);
	       pc->ozonval=ozonval; 	//SKP
	       pc->aux=1;
               if (sondeaprs_verb) {
                  osi_WrStr(" OZON:(", 8ul);
                  osic_WrFixed((float)pc->ozonTemp, 2L, 1UL);
                  osi_WrStr("oC ", 4ul);
                  osic_WrFixed((float)pc->ozonuA, 4L, 1UL);
                  osi_WrStr("uA ", 4ul);
                  osic_WrFixed((float)ozonval, 3L, 1UL);
                  osi_WrStr("mPa ", 5ul);
                  osic_WrFixed((float)pc->ozonBatVolt, 1L, 1UL);
                  osi_WrStr("BatV ", 6ul);
                  osic_WrFixed((float)pc->ozonPumpMA, 0L, 1UL);
                  osi_WrStr("mA ", 4ul);
                  osic_WrFixed((float)pc->ozonExtVolt, 1L, 1UL);
                  osi_WrStr("ExtV", 5ul);
                  osi_WrStr(")", 2ul);
               }
            }
         }
         else if (len==24UL) {
            /* Ozon id-data */
            if (sondeaprs_verb) {
               osi_WrStr(" OZONID:(", 10ul);
               tmp = p+12UL;
               i = p+5UL;
               if (i<=tmp) for (;; i++) {
                  WrChChk(rxb[i]);
                  if (i==tmp) break;
               } /* end for */
               if (((uint32_t)(uint8_t)rxb[17UL]&1)) {
                  osi_WrStr(" NotCal", 8ul);
               }
               osi_WrStr(" V:", 4ul);
               osic_WrFixed((float)gethex(rxb, rxb_len, p+18UL, 2UL)*0.1f,
                 1L, 1UL);
               osi_WrStr(")", 2ul);
            }
         }
      }
/*      else if (typ=='v') {
      }
      else {
         //             WrStrLn("76 frame");
         break;
      }
*/
      if (typ=='v') break;
      p += len;
   }
   if (sondeaprs_verb) osi_WrStrLn("", 1ul);

    //store_sonde_db( pc->name,frameno,lat,long0,heig,speed,dir,climb,4,pc->burstKill,pc->swVersion,pc->ozonval,pc->aux,0.0,(double)pc->mhz0,pc->vbat,0,0,0);

   if ((((pc && nameok) && calok) && lat!=0.0) && long0!=0.0) {
      pc->framesent = 1;
      //SKP
      store_sonde_db( pc->name,frameno,lat,long0,heig,speed,dir,climb,4,pc->burstKill,pc->swVersion,pc->ozonval,pc->aux,0.0,(double)pc->mhz0,pc->vbat,0,0,0);
      if(saveLog) save_Slog( pc->name,frameno,lat,long0,heig,speed,dir,climb,4,pc->burstKill,pc->swVersion,pc->ozonval,pc->aux,0.0,(double)pc->mhz0,pc->vbat,0,0,0);
   }
   
     
     if (((pc && nameok) && lat!=0.0) && long0!=0.0) {
        store_sonde_rs( pc->name,frameno,lat,long0,heig,speed,dir,climb,4,pc->burstKill,pc->swVersion,pc->ozonval,pc->aux,0.0,(double)pc->mhz0,pc->vbat,0,0,0,usercall);
	if(saveLog) save_Slog( pc->name,frameno,lat,long0,heig,speed,dir,climb,4,pc->burstKill,pc->swVersion,pc->ozonval,pc->aux,0.0,(double)pc->mhz0,pc->vbat,0,0,0);
     }
/*  IF verb THEN WrStrLn("") END;   */
} /* end decoders41() */

/*------------------------------ M10 */

int isNDig(char *txt){
    int len,i;
    int ret=0;
    len=strlen(txt);
    if(len==0) return(1);

    for(i=0;i<len;i++){
        if(txt[i]<47 || txt[i]>57){
            if(txt[i]!='.' && txt[i]!='-' && txt[i]!=' ')
                ret=1;
        }
    }
    return (ret);
}


static void decodem10(const char rxb[], uint32_t rxb_len, uint32_t ip, uint32_t fromport)
{
   uint32_t i;
   double dir;
   double fq555;
   double v;
   double vv;
   double alt;
   double lon;
   double lat;
   char nam[15];
   char s[1001];
   CALLSSID usercall;
   uint32_t frameno;
   char nameok;
   char calok;
   pCONTEXTM10 pc0;
   pCONTEXTM10 pc1;
   pCONTEXTM10 pc;
   calok = 0;
   nameok = 0;
   pc = 0;
   lat = 0.0;
   lon = 0.0;
   double frq;
   char tmps[15];
   float vbat,temp1,temp2;
    char to[1200];
    uint32_t time0;




    int cnt=0;
    for (i=0; i<105; i++)
        if(rxb[i] == ',') cnt++;
    if(cnt!=13) return;
    char *tmp = strtok(rxb, ",");

    getcall(tmp, 6, usercall, 11ul);
    if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);

   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }


        tmp = strtok(NULL, ",");        //frq
         if(isNDig(tmp)) return(0);
	frq=atof(tmp)/1000;

        tmp = strtok(NULL, ",");        //nazwa
         if(isNDig(tmp)) return(0);
         strcpy(nam,tmp);

        tmp = strtok(NULL, ",");        //time
         if(isNDig(tmp)) return(0);
         time0=atol(tmp);
	 frameno=time0;

        tmp = strtok(NULL, ",");        //lat
         if(isNDig(tmp)) return(0);
         lat=atof(tmp);

        tmp = strtok(NULL, ",");        //lon
         if(isNDig(tmp)) return(0);
        lon=atof(tmp);

        tmp = strtok(NULL, ",");        //alt
         if(isNDig(tmp)) return(0);
        alt=atof(tmp);

        tmp = strtok(NULL, ",");        //dir
         if(isNDig(tmp)) return(0);
        dir=atof(tmp);

        tmp = strtok(NULL, ",");        //v
         if(isNDig(tmp)) return(0);
        v=atof(tmp);

        tmp = strtok(NULL, ",");        //vv
        if(isNDig(tmp)) return(0);
        vv=atof(tmp);

        tmp = strtok(NULL, ",");        //vbat
         if(isNDig(tmp)) return(0);
        vbat=atof(tmp);

        tmp = strtok(NULL, ",");        //temp1
         if(isNDig(tmp)) return(0);
        temp1=atof(tmp);

        tmp = strtok(NULL, ",");        //temp2
         if(isNDig(tmp)) return(0);
        temp2=atof(tmp);

        tmp = strtok(NULL, ",");        //fq555
	tmp[6]=0;

         if(isNDig(tmp)) return(0);
        fq555=atof(tmp);


      pc = pcontextm10;
      pc0 = 0;
      for (;;) {
         if (pc==0) break;
         pc1 = pc->next;
         if (pc->tused+3600UL<systime) {
            
            if (pc0==0) pcontextm10 = pc1;
            else pc0->next = pc1;
            osic_free((char * *) &pc, sizeof(struct CONTEXTM10));
         }
         else {
            if (aprsstr_StrCmp(nam, 10ul, pc->name, 10ul)) break;
            pc0 = pc;
         }
         pc = pc1;
      }
      if (pc==0) {
         osic_alloc((char * *) &pc, sizeof(struct CONTEXTM10));
         if (pc==0) Error("allocate context out im memory", 31ul);
         memset((char *)pc,(char)0,sizeof(struct CONTEXTM10));
         pc->next = pcontextm10;
         pcontextm10 = pc;
         aprsstr_Assign(pc->name, 10ul, nam, 10ul);
         if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
      }

      frameno = time0;
      pc->gpssecond = time0+86382UL;
      if (frameno > pc->framenum) {
         pc->framesent = 0;
         calok = 1;
         pc->framenum = frameno;
         pc->tused = systime;
      }
      else if (pc->framenum==frameno) {
         if (!pc->framesent) calok = 1;
      }
      else if (sondeaprs_verb) {
         osi_WrStr(" got old frame ", 16ul);
         osic_WrINT32(frameno, 1UL);
         osi_WrStr(" expected> ", 12ul);
         osic_WrINT32(pc->framenum, 1UL);
         osi_WrStr(" ", 2ul);
      }


   if (pc && lat>0 && lat<90 && lon>0 && alt<45000 ) {

      if (sondeaprs_verb) 
	printf("M10: (%s) %s,%012lu,%09.5f,%010.5f,%05.0f,%03.0f,%05.1f,%05.1f,%05.2f,%06.1f,%06.1f,%06.0f\n",usercall,nam,time0,lat,lon,alt,dir,v,vv,vbat,temp1,temp2,fq555);

      store_sonde_db( pc->name,pc->framenum,lat* 1.7453292519943E-2,lon* 1.7453292519943E-2,alt,v,dir,vv,1,0,0,0,0,0.0,frq,vbat,temp1,temp2,fq555);
      store_sonde_rs( pc->name,pc->framenum,lat* 1.7453292519943E-2,lon* 1.7453292519943E-2,alt,v,dir,vv,1,0,0,0,0,0.0,frq,vbat,temp1,temp2,fq555,usercall);
      if(saveLog) save_Slog( pc->name,pc->framenum,lat* 1.7453292519943E-2,lon* 1.7453292519943E-2,alt,v,dir,vv,1,0,0,0,0,0.0,frq,vbat,temp1,temp2,fq555);
      pc->framesent = 1;

    }


} /* end decodem10() */



/*------------------------------ M20 */


static void decodem20(const char rxb[], uint32_t rxb_len, uint32_t ip, uint32_t fromport)
{
   uint32_t i;
   double dir;
   double v;
   double vv;
   double alt;
   double lon;
   double lat;
   char nam[15];
   char s[1001];
   CALLSSID usercall;
   uint32_t frameno;
   char nameok;
   char calok;
   pCONTEXTM20 pc0;
   pCONTEXTM20 pc1;
   pCONTEXTM20 pc;
   calok = 0;
   nameok = 0;
   pc = 0;
   lat = 0.0;
   lon = 0.0;
   double frq;
   char tmps[15];
   float vbat,temp1,temp2;
    char to[1200];
    uint32_t time0;




    int cnt=0;
    for (i=0; i<100; i++)
        if(rxb[i] == ',') cnt++;
    if(cnt!=12) return;
    char *tmp = strtok(rxb, ",");
    
    getcall(tmp, 6, usercall, 11ul);
    if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);

   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }

        tmp = strtok(NULL, ",");        //frq
         if(isNDig(tmp)) return(0);
	frq=atof(tmp)/1000;
        tmp = strtok(NULL, ",");        //nazwa
         if(isNDig(tmp)) return(0);
         strcpy(nam,tmp);
        tmp = strtok(NULL, ",");        //time
         if(isNDig(tmp)) return(0);
         time0=atol(tmp);
	 frameno=time0;
        tmp = strtok(NULL, ",");        //lat
         if(isNDig(tmp)) return(0);
         lat=atof(tmp);
        tmp = strtok(NULL, ",");        //lon
         if(isNDig(tmp)) return(0);
        lon=atof(tmp);
        tmp = strtok(NULL, ",");        //alt
         if(isNDig(tmp)) return(0);
        alt=atof(tmp);
        tmp = strtok(NULL, ",");        //dir
         if(isNDig(tmp)) return(0);
        dir=atof(tmp);
        tmp = strtok(NULL, ",");        //v
         if(isNDig(tmp)) return(0);
        v=atof(tmp);
        tmp = strtok(NULL, ",");        //vv
        if(isNDig(tmp)) return(0);
        vv=atof(tmp);
        tmp = strtok(NULL, ",");        //vbat
         if(isNDig(tmp)) return(0);
        vbat=atof(tmp);
        tmp = strtok(NULL, ",");        //temp1
         if(isNDig(tmp)) return(0);
        temp1=atof(tmp);
        tmp = strtok(NULL, ",");        //temp2
	tmp[6]=0;

         if(isNDig(tmp)) return(0);
        temp2=atof(tmp);


      pc = pcontextm20;
      pc0 = 0;
      for (;;) {
         if (pc==0) break;
         pc1 = pc->next;
         if (pc->tused+3600UL<systime) {
            
            if (pc0==0) pcontextm20 = pc1;
            else pc0->next = pc1;
            osic_free((char * *) &pc, sizeof(struct CONTEXTM20));
         }
         else {
            if (aprsstr_StrCmp(nam, 10ul, pc->name, 10ul)) break;
            pc0 = pc;
         }
         pc = pc1;
      }
      if (pc==0) {
         osic_alloc((char * *) &pc, sizeof(struct CONTEXTM20));
         if (pc==0) Error("allocate context out im memory", 31ul);
         memset((char *)pc,(char)0,sizeof(struct CONTEXTM20));
         pc->next = pcontextm20;
         pcontextm20 = pc;
         aprsstr_Assign(pc->name, 10ul, nam, 10ul);
         if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
      }

      frameno = time0;
      pc->gpssecond = time0+86382UL;
      if (frameno > pc->framenum) {
         pc->framesent = 0;
         calok = 1;
         pc->framenum = frameno;
         pc->tused = systime;
      }
      else if (pc->framenum==frameno) {
         if (!pc->framesent) calok = 1;
      }
      else if (sondeaprs_verb) {
         osi_WrStr(" got old frame ", 16ul);
         osic_WrINT32(frameno, 1UL);
         osi_WrStr(" expected> ", 12ul);
         osic_WrINT32(pc->framenum, 1UL);
         osi_WrStr(" ", 2ul);
      }

   if (pc && lat>0 && lat<90 && lon>0 && alt<45000 ) {

      if (sondeaprs_verb) 
	printf("M20: (%s) %s,%012lu,%09.5f,%010.5f,%05.0f,%03.0f,%05.1f,%05.1f,%05.2f,%06.1f,%06.1f\n",usercall,nam,time0,lat,lon,alt,dir,v,vv,vbat,temp1,temp2);

      store_sonde_db( pc->name,pc->framenum,lat* 1.7453292519943E-2,lon* 1.7453292519943E-2,alt,v,dir,vv,2,0,0,0,0,0.0,frq,vbat,temp1,temp2,0);
      store_sonde_rs( pc->name,pc->framenum,lat* 1.7453292519943E-2,lon* 1.7453292519943E-2,alt,v,dir,vv,2,0,0,0,0,0.0,frq,vbat,temp1,temp2,0,usercall);
      if(saveLog) save_Slog( pc->name,pc->framenum,lat* 1.7453292519943E-2,lon* 1.7453292519943E-2,alt,v,dir,vv,2,0,0,0,0,0.0,frq,vbat,temp1,temp2,0);
      pc->framesent = 1;

    }


} /* end decodem20() */





//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//PILOTSONDE
//
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


static int32_t getint32r(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   uint32_t n;
   uint32_t i;
   n = 0UL;
   for (i = 0UL;; i++) {
      n = n*256UL+(uint32_t)(uint8_t)frame[p+i];
      if (i==3UL) break;
   } /* end for */
   return (int32_t)n;
} /* end getint32r() */


static int32_t getint16r(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   uint32_t n;
   n = (uint32_t)(uint8_t)frame[p+1UL]+256UL*(uint32_t)(uint8_t)
                frame[p];
   if (n>=32768UL) return (int32_t)(n-65536UL);
   return (int32_t)n;
} 
/* end getint16r() */




int dzienRoku(int year, int month, int dayOfMonth) {
    int wynik= dayOfMonth + ((month < 3) ? (int)((306 * month - 301) / 10) :(int)((306 * month - 913) / 10) + ((year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 60 : 59)); 
return wynik;
}

static void decodepils(const char rxb[], uint32_t rxb_len, uint32_t ip, uint32_t fromport)
{
   OBJNAME nam;
   int32_t res;
   char s[1001];
   //char tmp_name[9];
   CALLSSID usercall;
   uint32_t frnr;
   uint32_t len;
   uint32_t ic;
   uint32_t j;
   uint32_t i;
   uint16_t crc;
   int typ;
   pCONTEXTPS pc0;
   pCONTEXTPS pc1;
   pCONTEXTPS pc;
   double climb;
   double dir;
   double speed;
   double heig;
   double long0;
   double lat;
   uint32_t tmp;
   uint32_t offs;
   double phrms;
   double pvrms;
   uint32_t pgoodsat;
   double ve;
   double vn;
   double vz;
   double vy;
   double vx;
   double vu;
   char tmps[10];
   double frq;
   int32_t timegp;
   int32_t dategp,gpstime;
   
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
    unsigned char  bytes[4];

    getcall(rxb, rxb_len, usercall, 11ul);  //decode callsign from table
    
/*    if (pilname[0UL]!=0UL){
	for (i=0UL; i<8UL; i++){
	    tmp_name[i]=(uint8_t)pilname[i];
	}
    }
*/     
//    if (pilname[0UL]=0UL){
    //assign name to object (no number coded in sonde) - pilotson

//    if(gpstime-pc->lastframe>300 || pc->name[0]==0){	
//        czas=dzienRoku((1900+tm.tm_year),tm.tm_mon+1,tm.tm_mday);

//    }     
//    else strcpy(nam,pc->

    tmps[0]=rxb[55];
    tmps[1]=rxb[56];
    tmps[2]=rxb[57];
    tmps[3]='.';
    tmps[4]=rxb[58];
    tmps[5]=rxb[59];
    tmps[6]=rxb[60];
    tmps[7]=0;
    frq=atof(tmps);

    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    int czas=tm.tm_mon + 1 + tm.tm_mday;

    nam[0]='P'; 
    nam[1]=65+tm.tm_hour; 
    nam[2]=65+(int)(czas/25);
    nam[3]=65+czas%25;
    nam[4]=rxb[57]; 
    nam[5]='4';//rxb[58]; 
    nam[6]='7';//rxb[59]; 
    nam[7]='0';//rxb[60]; 
    nam[8]=0;

    gpstime=(int32_t)(86382UL+tm.tm_sec+tm.tm_min*60+tm.tm_hour*3600);

    char str[15];

    if (sondeaprs_verb && fromport>0UL) {
        osi_WrStr("UDP:", 5ul);
        aprsstr_ipv4tostr(ip, s, 1001ul);
        osi_WrStr(s, 1001ul);
        osi_WrStr(":", 2ul);
        osic_WrINT32(fromport, 1UL);
        if (usercall[0U]) {
    	    osi_WrStr(" (", 3ul);
            osi_WrStr(usercall, 11ul);
            osi_WrStr(")", 2ul);
        }
        osi_WrStr(" ", 2ul);
        osi_WrStr(" name=",7UL);
        osi_WrStr(nam,8UL);  
	
    }
      
    pc = pcontextps;
    pc0 = 0;
         
    for (;;) {                            //inicjacja polaczenia chyba
        if (pc==0) break;              
        pc1 = pc->next;
        if (pc->tused+3600UL<systime) {
           /* timed out */
           if (pc0==0) pcontextps = pc1;
           else pc0->next = pc1;
           osic_free((char * *) &pc, sizeof(struct CONTEXTPS));
        }
        else {
           if (aprsstr_StrCmp(nam, 8ul, pc->name, 8ul)) break;
               pc0 = pc;
        }
        pc = pc1;
    }
    if (pc==0) {
        osic_alloc((char * *) &pc, sizeof(struct CONTEXTPS));
        if (pc==0) Error(" allocate context out im memory", 32ul);
        memset((char *)pc,(char)0,sizeof(struct CONTEXTPS));
        pc->next = pcontextps;
        pcontextps = pc;
	pc->tused=systime;
        aprsstr_Assign(pc->name, 8ul, nam, 8ul);
        if (sondeaprs_verb) osi_WrStrLn(" is new ", 9ul);
    }
    
    //now extract data from table

    offs=8UL;    //offset - 5 added and 3 preamble bytes - now taken for callsign
    lat=(double)getint32r(rxb,rxb_len,offs+3UL)*0.000001;   //calculate lat in DD.DDDDDD
    long0=(double)getint32r(rxb,rxb_len,offs+7UL)*0.000001; //claculate long in DDD.DDDDDD
    heig=(double)getint32r(rxb,rxb_len,offs+11UL)*.01;      //height in m


    for (i = 0; i < 4; i++)  bytes[i] = rxb[offs+15UL+ i];
    timegp = 0;
    for (i = 0; i < 4; i++)  timegp |= bytes[i] << (8*(3-i));

    for (i = 0; i < 4; i++)  bytes[i] = rxb[offs+19UL + i];
    dategp = 0;
    for (i = 0; i < 4; i++)  dategp |= bytes[i] << (8*(3-i));

    gpstime=(int32_t)(((timegp%100000)/1000.0)+ (timegp%10000000)/100000*60+ 3600*timegp/10000000) +86400* dategp%100;
    

/*
 fprintf(stdout, " %02d-%02d-%02d", datum.date/10000, (datum.date%10000)/100, datum.date%100);
        //fprintf(stdout, " (%09d)", datum.time);
        fprintf(stdout, " %02d:%02d:%06.3f ", datum.time/10000000, (datum.time%10000000)/100000, (datum.time%100000)/1000.0);

      time0 = tow/1000UL+week*604800UL+315964800UL;
      frameno = time0;
      X2C_CHKNIL(pCONTEXTM10,pc)->gpssecond = time0+86382UL;


    time=getint32r(rxb,rxb_len,offs+15UL)
    date=getint32r(rxb,rxb_len,offs+15UL)
*/
//    printf("%02d-%02d-%02d %02d:%02d:%06.3f", datum.date/10000, (datum.date%10000)/100, datum.date%100,datum.time/10000000, (datum.time%10000000)/100000, (datum.time%100000)/1000.0);

//    frnr=(datum.time%100000)/1000.0 + 60*(datum.time%10000000)/100000 + 3600*datum.time/10000000  + 86400* datum.date%100;

    if (lat>89.9) {lat=0.0;}                                //in case lat/lon invalid
    if (long0>179.9) {long0=0.0;}
		
    if (heig<(-500.0) || heig>50000.0) {                    //make sure that wrong altitude eliminates all
        lat = 0.0;
	long0 = 0.0;
	heig = 0.0; }
		
	lat=lat*1.7453292519943E-2;
        long0=long0*1.7453292519943E-2;
		
	pgoodsat=(uint32_t)rxb[offs+1UL];                       //sats used
//	phrms=(double)getint16r(rxb,rxb_len,offs+30UL)*0.01;    //hrms/vrms - quality bytes
//	pvrms=(double)getint16r(rxb,rxb_len,offs+32UL)*0.01;
	phrms=0.0;                                               //ignore previous
	pvrms=0.0;
		
		
       /*speed */
       vx = (double)getint16r(rxb, rxb_len, offs+15UL)*(0.01);     //get N/S, E/W, and climb speed data  
       vy = (double)getint16r(rxb, rxb_len, offs+17UL)*(0.01);     //somehow it rather should be +
       vz = (double)getint16r(rxb, rxb_len, offs+19UL)*(0.01);     //but it makes more sense with -
               
       //somple procedure for speed and dir:
	       
       speed = (double)osic_sqrt((float)(vx*vx+vy*vy));  // m/s?
       dir = atan2(vx, vy) * X2C_DIVL(180.0, 3.141592654);
       if (dir<0.0) dir = 360.0+dir;
	       
       climb = vz;                                                //climb speed 
       if (sondeaprs_verb) {
	 osi_WrStr("lat=", 5ul);
         osic_WrFixed((float)(lat/1.7453292519943E-2), 4UL, 6UL);
         osi_WrStr(" long=", 6ul);
         osic_WrFixed((float)long0/1.7453292519943E-2, 4UL, 7UL);
         osi_WrStr(" alt=", 6ul);
         osic_WrFixed((float)heig, 2UL, 7UL);
         osi_WrStr("m speed=", 9ul);
	 osic_WrFixed((float)speed, 2UL, 4UL);
         osi_WrStr(" dir=", 6ul);
         osic_WrFixed((float)dir, 2UL, 4UL);
         osi_WrStr(" climb=", 8ul);
         osic_WrFixed((float)climb, 2UL, 4UL);
    	 osi_WrStrLn("", 1ul);
      }
   if ((pc && lat!=0.0) && long0!=0.0) {                            //if connected and valid lat/long
     
      
      store_sonde_db( pc->name,gpstime,lat,long0,heig,speed,dir,climb,8,0,0,0,0,0.0,frq,0,0,0,0);
      store_sonde_rs( pc->name,gpstime,lat,long0,heig,speed,dir,climb,8,0,0,0,0,0.0,frq,0,0,0,0,usercall);
      if(saveLog) save_Slog( pc->name,gpstime,lat,long0,heig,speed,dir,climb,8,0,0,0,0,0.0,frq,0,0,0,0);

      pc->framesent = 1;
   }
} /* end decodepils() */



//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//-----------------------------------------------------------------------------------------------------
//PILOTSONDE
//
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

unsigned char reverse(unsigned char b) {

   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

   return b;

}

uint16_t crc_mp3(uint8_t buf[], int st,int len)
{
  uint16_t crc = 0xFFFF;

  for (int pos = st; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;
}

float course(float lat1, float lon1, float lat2, float lon2) {
  
//  float lat1, lat2;
  
  float dlon = lon2-lon1;
//  lat1 = radians(la1);
//  lat2 = radians(la2);
  float a1 = sin(dlon) * cos(lat2);
  float a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0) {
    a2 += 6.28318;      // modulo operator doesn't seem to work on floats
  }
  a1=a2*180.0/M_PI;
  return a1;
}


static void decodemp3(const char rxb[], uint32_t rxb_len, uint32_t ip, uint32_t fromport)
{
   CALLSSID usercall;
   double frq;
   OBJNAME nam;
   char tmps[10];
   uint8_t tmpD[155];
   uint32_t gpstime,frno;
   pCONTEXTMP3 pc;
   pCONTEXTMP3 pc0;
   pCONTEXTMP3 pc1;

   pc = 0;

    getcall(rxb+103, rxb_len-103, usercall, 11ul);  //decode callsign from table

    tmps[0]=rxb[109];
    tmps[1]=rxb[110];
    tmps[2]=rxb[111];
    tmps[3]='.';
    tmps[4]=rxb[112];
    tmps[5]=rxb[113];
    tmps[6]=rxb[114];
    tmps[7]=0;
    frq=atof(tmps);

    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    struct tm tmfn = *gmtime(&t);
    int czas=tm.tm_mon + 1 + tm.tm_mday;


    int s,q,p;
    uint16_t tmpb;
    p=0;
    for(s=0;s<102;s+=2){
        tmpb=0;
        tmpb=(rxb[s]&0xff)<<8 | (rxb[s+1]&0xff);
        tmpD[p]=0;
        for(q=0;q<8;q++){
            tmpD[p]<<=1;
            if((tmpb & 0xC000) == 0x8000) tmpD[p]|=1;

            switch(tmpb & 0xC000){
                case 0xC000:
                case 0x0000:
	                return;
                        break;
            }
            tmpb<<=2;
        }
        p++;
    }
    for(s=0;s<51;s++){
        uint8_t calc,t1,t2;
        calc=reverse(tmpD[s]&0xff);
        t1=calc>>4 &0b00001111;
        t2=calc<<4 &0b11110000;
        calc=0;calc=t1|t2;
        tmpD[s]=calc;
    }
    uint16_t crc=tmpD[49] | tmpD[50]<<8;
    if(crc==crc_mp3(&tmpD,4,49)){
        double lat,lon,alt;
        uint32_t x,y,z;
	int16_t tmpS;
	double speed=0.0,dir=0.0;

        double vx,vy,vz,vE,vN,vU;

        x=tmpD[12]; x<<=8; x|=tmpD[11]; x<<=8; x|=tmpD[10]; x<<=8; x|=tmpD[9];
        y=tmpD[16]; y<<=8; y|=tmpD[15]; y<<=8; y|=tmpD[14]; y<<=8; y|=tmpD[13];
        z=tmpD[20]; z<<=8; z|=tmpD[19]; z<<=8; z|=tmpD[18]; z<<=8; z|=tmpD[17];

        wgs84r(0.01*x, 0.01*y, 0.01*z, &lat, &lon, &alt);

        gpstime= mktime(&tmfn);
	frno=(int32_t)(tmpD[7]+tmpD[6]*60+tmpD[5]*3600);


      pc = pcontextmp3;
      pc0 = 0;
      for (;;) {
         if (pc==0) break;
         pc1 = pc->next;
         if (pc->prevgpstime+3600UL<gpstime) {

            if (pc0==0) pcontextmp3 = pc1;
            else pc0->next = pc1;
            osic_free((char * *) &pc, sizeof(struct CONTEXTMP3));
         }
         else {
            if (aprsstr_StrCmp(nam, 10ul, pc->name, 10ul)) break;
            pc0 = pc;
         }
         pc = pc1;
      }
      if (pc==0) {
         osic_alloc((char * *) &pc, sizeof(struct CONTEXTMP3));
         if (pc==0) Error("allocate context out im memory", 31ul);
         memset((char *)pc,(char)0,sizeof(struct CONTEXTMP3));
         pc->next = pcontextmp3;
         pcontextmp3 = pc;
         aprsstr_Assign(pc->name, 10ul, nam, 10ul);
	 pc->prevalt=alt;
	 pc->prevfrno = frno;
	 pc->prevgpstime=gpstime;
         if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
      }

      if (frno != pc->prevfrno) {

            pc->framesent = 0;
	    double vv;
	    float dirC;
	    long dt;

	    dt=gpstime-pc->prevgpstime;
	    if(dt==0) { dt=1; vv=0;}
	    else
		vv=(double)(alt-pc->prevalt)/(dt);
	    dirC=course(pc->prevlat,pc->prevlon,lat,lon);
	    pc->prevalt=alt;
	    pc->prevlat=lat;pc->prevlon=lon;
	    pc->prevgpstime=gpstime;
	    pc->prevfrno=frno;

	    nam[0]='M';	    nam[1]='3';
	    if(tm.tm_hour>6 && tm.tm_hour<18) nam[2]='A';
	    else nam[2]='P';
	    nam[3]=65+(int)(czas/25);
	    nam[4]=65+czas%25;
	    nam[5]=rxb[111]; nam[6]=rxb[112]; nam[7]=rxb[113];   nam[8]=0;//rxb[114]; 

	    uint16_t vb;
	    uint32_t sn;
	    if(tmpD[44]==0x0d) {
		sn=tmpD[48];
		sn<<=8;
		sn|=tmpD[47];
		sn<<=8;
		sn|=tmpD[46];
		sn<<=8;
		sn|=tmpD[45];
		pc->snd=(float)sn;
	    }

	    if(lat>0.0 && lon>0.0 && alt<40000.0){
		printf("%s %02i:%02i:%02i f: %f La: %f Lo: %f Alt: %0.0fm\n",nam, tmpD[5],tmpD[6],tmpD[7],frq,lat/1.7453292519943E-2,lon/1.7453292519943E-2,alt);
		store_sonde_db( pc->name,frno,lat,lon,alt,speed,dirC,vv,20,0,0,0,0,0.0,frq,pc->vbat,0,pc->snd,0);
        	store_sonde_rs( pc->name,frno,lat,lon,alt,speed,dirC,vv,20,0,0,0,0,0.0,frq,pc->vbat,0,0,0,usercall);
        	if(saveLog) save_Slog( pc->name,frno,lat,lon,alt,speed,dirC,vv,20,0,0,0,0,0.0,frq,pc->vbat,0,pc->snd,0);
		pc->framesent=1;
	    }
      }
      else if (sondeaprs_verb) {
	 printf("MP3 got old frame %i expected > %i\n",frno,pc->prevfrno);
      }

    }


} /* end decodemp3() */



static void udprx(void)
{
   uint32_t fromport;
   uint32_t ip;
   int32_t len;
   len = udpreceiveblock(rxsock, chan[sondemod_LEFT].rxbuf, 520L, &fromport, &ip);
   systime = osic_time();

  if (len>0) switch (len){ 
      case 240: decodeframe(sondemod_LEFT, ip, fromport); break;
      case 28:  decodec34(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport); break;
      case 93:  decodedfm6(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport); break;
      case 520: decoders41(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport); break;
      case 105: decodem10(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport); break;
      case 100: decodem20(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport); break;
      case 61:  decodepils(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport); break;
      case 115: decodemp3(chan[sondemod_LEFT].rxbuf, 115ul, ip, fromport); break;
      default: fprintf(stderr,"unsupported frame len %d\n", len);
   }
} 


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   char ip[50],i;
   
   if(disSKP==0){
	if(h2ip("skp.wodzislaw.pl",ip)){
	    fprintf(stderr,"\r\nCan't resolve DNS address using 194.140.233.120\r\n");
	    sprintf(ip,"194.140.233.120");
	    //return 0;
	}
   }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        printf("err: socket\n");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_aton(ip, &serv_addr.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }



   for(i=0;i<DBS_SIZE;i++) memset(&dBs[i],0,sizeof(struct DBS));

   read_csv();

   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(OBJNAME)!=9) X2C_ASSERT(0);
   if (sizeof(CALLSSID)!=11) X2C_ASSERT(0);
   sondeaprs_BEGIN();
   gpspos_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   Parms();
   /*  initrsc; */
   initcontext(&contextr9);
   pcontextc = 0;
   pcontextdfm6 = 0;
   pcontextr4 = 0;
   pcontextm10 = 0;
   pcontextm20 = 0;
   pcontextps = 0;
   objname[0] = 0;
   almread = 0UL;
   almage = 0UL;
   lastip = 0UL;
   lastport = 0UL;
   systime = osic_time();

   /*testalm; */
   for (;;) udprx();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
