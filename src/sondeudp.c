/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 * Modified by SP9SKP, SQ2DK
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define sondeudp_C_
#ifndef soundctl_H_
#include "soundctl.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
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
#ifndef rsc_H_
#include "rsc.h"
#endif
#include <stdio.h>

#include <time.h>

/* demodulate RS92 sonde (2400bit/s manchester)
   and SRS-C34 (2400Bd AFSK 2000/3800Hz
   and DFM06 (2500bit/s manchester)
   and RS41 (4800Bd GFSK) and send as AXUDP by OE5DXL */
/*FROM fcntl IMPORT creat, open; */
/*IMPORT reedsolomon; */
/* link init_rs_char.o decode_rs_char.o */
/* gcc  -o sondeudp Lib.o aprsstr.o filesize.o flush.o osi.o ptty.o rsc.o sondeudp.o soundctl.o symlink.o tcp.o timec.o udp.o init_rs_char.o decode_rs_char.o /usr/local/xds/lib/x86/libts.a /usr/local/xds/lib/x86/libxds.a  -lm */
#define sondeudp_MAXCHAN 64

#define sondeudp_CONTEXTLIFE 3600
/* seconds till forget context after last heared */

#define sondeudp_ADCBYTES 2

#define sondeudp_MAXLEN 9
/* data frame size c34 */

#define sondeudp_ADCBUFLEN 4096

#define sondeudp_BAUDSAMP 65536

#define sondeudp_PLLSHIFT 1024

#define sondeudp_RAD 1.7453292519943E-2

#define sondeudp_DFIRLEN 64

#define sondeudp_AFIRLEN 32

#define sondeudp_AOVERSAMP 16
/*16*/

#define sondeudp_ASYNBITS 10

static char sondeudp_CALIBFRAME = 'e';

static char sondeudp_GPSFRAME = 'g';

static char sondeudp_UNKNOWN = 'h';

static char sondeudp_DATAFRAME = 'i';

#define sondeudp_DOVERSAMP 16

#define sondeudp_CIDTIMEOUT 3600
/* s to delete c34 sonde id */

#define sondeudp_DFIDTIMEOUT 900
/* s to delete dfm sonde id */

#define sondeudp_FLEN6 264
/* dfm06 frame len */

#define sondeudp_DFMSYN 0x45CF 
/* frame sync sequence */

#define sondeudp_FLENRS41 520
/* rs41  frame len */

#define sondeudp_FLEN10 101
/* M10 framelen */
#define sondeudp_M10SYN 0x649F20.
/* M10 sync */

#define sondeudp_FLEN20 70
/* M20 framelen */
#define sondeudp_M10SYN 0x4520
/* M20 sync */



/*
  rs41x 0x86, 0x35, 0xf4, 0x40, 0x93, 0xdf, 0x1a, 0x60
  rs41  0x10, 0xB6, 0xCA, 0x11, 0x22, 0x96, 0x12, 0xF8
*/
#define sondeudp_RHEAD41 "000010000110110101010011100010000100010001101001010\
0100000011111"

static uint8_t sondeudp_EXOR41[64] = {150U,131U,62U,81U,177U,73U,8U,152U,
                50U,5U,89U,14U,249U,68U,198U,38U,33U,96U,194U,234U,121U,93U,
                109U,161U,84U,105U,71U,12U,220U,232U,92U,241U,247U,118U,130U,
                127U,7U,153U,162U,44U,147U,124U,48U,99U,245U,16U,46U,97U,
                208U,188U,180U,182U,6U,170U,244U,35U,120U,110U,59U,174U,191U,
                123U,76U,193U};

typedef char FILENAME[1024];

typedef char CNAMESTR[9];

typedef float AFIRTAB[512];

typedef float DFIRTAB[1024];

typedef float DFIR[64];

struct UDPTX;

typedef struct UDPTX * pUDPTX;

struct UDPTX {
   pUDPTX next;
   uint32_t ip;
   uint32_t destport;
   int32_t udpfd;
};


struct R92;


struct R92 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float noise;
   float bitlev;
   float lastu;
   char cbit;
   char oldd;
   char plld;
   char lastmanch;
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   uint32_t headerrs;
   char rxbuf[256];
   AFIRTAB afirtab;
   int32_t asynst[10];
   uint32_t demodbaud;
   uint32_t configbaud;
};

struct MP3;

struct MP3 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   float noise0;
   float bitlev0;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char rev;
   char headok;
   float lastu;
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[120],rxprev[120];
   AFIRTAB afirtab;
   uint32_t demodbaud;
   uint32_t configbaud;
   uint32_t synp;
   char synbuf[64];
   char fixbytes[101];
   uint8_t fixcnt[520];
};


struct R41;

struct R41 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   float noise0;
   float bitlev0;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char rev;
   char headok;
   float lastu;
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[520];
   AFIRTAB afirtab;
   uint32_t demodbaud;
   uint32_t configbaud;
   uint32_t synp;
   char synbuf[64];
   char fixbytes[520];
   uint8_t fixcnt[520];
};


//Added ------------------------------ by SQ2DK

unsigned int pok=1,nok=1,cnt=1;

struct PILS;

struct PILS {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   float noise0;
   float bitlev0;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char rev;
   char headok;
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   float lastu;
   char rxbuf[56+6];               //only 55 required
   AFIRTAB afirtab;
   uint32_t demodbaud;
   uint32_t configbaud;
   uint32_t synp;
   char synbuf[64];
   char fixbytes[56];           //not sure what for
   uint8_t fixcnt[56];
   time_t lastfr;
   double poklat;
   double poklon;
   double pokalt;



//   int16_t configequalizer;
};
//-------------------------------------------------------


struct DFM6;
struct DFM6 {
   char enabled;
   uint32_t lastsent;
   uint32_t numcnt;
   uint32_t idcnt0;
   uint32_t idcnt1;
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float noise;
   float bitlev;
   float lastu;
   char polarity;
   char cbit;
   char oldd;
   char plld;
   char lastmanch;
   char txok;
   uint32_t rxp;
   char rxbuf[264+6];
   AFIRTAB afirtab;
   uint32_t synword;
   char cb[56];
   char ch[56];
   char db1[104];
   char db2[104];
   char dh1[104];
   char dh2[104];
   uint32_t demodbaud;
   uint32_t configbaud;
   char id[12];
    int frnr;
    int sonde_typ;
    uint32_t SNL;
    uint32_t SNH;
    uint32_t SNT;
    uint32_t SN;
    uint32_t SN6;
    uint32_t SN9;
    uint32_t SN15;
    int yr; int mon; int day;
    int hr; int min; float sek;
    double lat; double lon; double alt;
    double newlat; double newlon; double newalt;
    double prevlat; double prevlon; double prevalt;
    double dir; double horiV; double vertV;
    float meas24[5];
    float status[2];
    char newsonde;
    int ok;

};



struct M10;


struct M10 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float bitlev;
   float noise;
   float lastu;
   char cbit;
   char oldd;
   char plld;
   char lastmanch;
   char txok;
   uint32_t rxb;
   uint32_t rxp;
   char rxbuf[101];
   AFIRTAB afirtab;
   uint32_t synword;
   uint32_t demodbaud;
   uint32_t configbaud;
};

struct M20;


struct M20 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float bitlev;
   float noise;
   float lastu;
   char cbit;
   char oldd;
   char plld;
   char lastmanch;
   char txok;
   uint32_t rxb;
   uint32_t rxp;
   char rxbuf[70];
   AFIRTAB afirtab;
   uint32_t synword;
   uint32_t demodbaud;
   uint32_t configbaud;
};


struct SCID;


struct SCID {
   CNAMESTR id;
   CNAMESTR idcheck;
   uint32_t idtime;
   uint32_t idcnt;
};

struct C34;
struct C34 {
   char enabled;
   struct SCID id34;
   struct SCID id50;
   uint32_t idtime;
   uint32_t idcnt;
   int32_t pllshift;
   int32_t baudfine;
   int32_t leveldcd;
   float sqmed[2];
   float afskhighpass;
   float freq;
   float left;
   float tcnt;
   float afskmidfreq;
   float afmid;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char c50;
   uint32_t rxbyte;
   uint32_t rxbytec;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[9+6];
   AFIRTAB afirtab;
   int32_t asynst[10];
   DFIRTAB dfirtab;
   DFIR dfir;
   uint32_t dfin;
   uint32_t confignyquist;
   uint32_t configafskshift;
   uint32_t configafskmid;
   uint32_t demodbaud;
   uint32_t configbaud;
   uint32_t txbaud;
   uint32_t dcdclock;
   float hipasscap;
   uint32_t tused;
};

struct IMET;
struct IMET {
   char enabled;
   struct SCID imet;
//   struct SCID id50;
   uint32_t idtime;
   uint32_t idcnt;
   int32_t pllshift;
   int32_t baudfine;
   int32_t leveldcd;
   float sqmed[2];
   float afskhighpass;
   float freq;
   float left;
   float tcnt;
   float afskmidfreq;
   float afmid;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
//   char c50;
   uint32_t rxbyte;
   uint32_t rxbytec;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[9+6];
   AFIRTAB afirtab;
   int32_t asynst[10];
   DFIRTAB dfirtab;
   DFIR dfir;
   uint32_t dfin;
   uint32_t confignyquist;
   uint32_t configafskshift;
   uint32_t configafskmid;
   uint32_t demodbaud;
   uint32_t configbaud;
   uint32_t txbaud;
   uint32_t dcdclock;
   float hipasscap;
   uint32_t tused;

};





struct CHAN;


struct CHAN {
   int32_t adcmax;
   int32_t adcmin;
   int32_t adcdc;
   float afir[32];
   int32_t configequalizer;
   pUDPTX udptx;
   uint32_t squelch;
   uint32_t mycallc;
   char myssid;
   struct R92 r92;
   struct R41 r41;
   struct DFM6 dfm6;
   struct C34 c34;
   struct PILS pils;                      //added for pilot sonde SQ2DK
   struct M10 m10;
   struct M20 m20;
   struct IMET imet;
   struct MP3 mp3;

   char nr;
   char freq[10];
   char pfreq[10];

};

static int32_t soundfd;

static int32_t debfd;

static char abortonsounderr;

static char verb;

static char verb2;

static uint32_t dfmidchg;

static uint32_t getst;

static uint32_t afin;
static uint32_t afinR41;
static uint32_t afinPS;
static uint32_t afinMP3;

static uint32_t soundbufs;

static uint32_t adcrate;

static uint32_t adcbuflen;

static uint32_t adcbufrd;

static uint32_t adcbufsamps;

static uint32_t fragmentsize;

static FILENAME soundfn;

static struct CHAN chan[64];

static uint32_t adcbufsampx;

static uint32_t maxchannels;

static uint32_t cfgchannels;

static short adcbuf[4096];

static uint32_t dfmnametyp;

static uint16_t CRCTAB[256];



static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void Hamming(float f[], uint32_t f_len)
{
   uint32_t i;
   uint32_t tmp;
   tmp = f_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      f[i] = f[i]*(0.54f+0.46f*osic_cos(3.1415926535898f*(X2C_DIVR((float)
                i,(float)(1UL+(f_len-1))))));
      if (i==tmp) break;
   } /* end for */
} /* end Hamming() */


static void initdfir(DFIRTAB dfirtab, uint32_t fg)
{
   uint32_t f;
   uint32_t i;
   float t[512];
   float e;
   float f1;
   uint32_t tmp;
   for (i = 0UL; i<=511UL; i++) {
      t[i] = 0.5f;
   } /* end for */
   f1 = X2C_DIVR((float)(fg*64UL),(float)adcrate);
   tmp = (uint32_t)X2C_TRUNCC(f1,0UL,X2C_max_longcard)+1UL;
   f = 1UL;
   if (f<=tmp) for (;; f++) {
      e = 1.0f;
      if (f==(uint32_t)X2C_TRUNCC(f1,0UL,X2C_max_longcard)+1UL) {
         e = f1-(float)(uint32_t)X2C_TRUNCC(f1,0UL,X2C_max_longcard);
      }
      for (i = 0UL; i<=511UL; i++) {
         t[i] = t[i]+e*osic_cos(X2C_DIVR(3.1415926535898f*(float)(i*f),512.0f));
      } /* end for */
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 512ul);
   for (i = 0UL; i<=511UL; i++) {
      t[i] = t[i]*(0.54f+0.46f*osic_cos(3.1415926535898f*(X2C_DIVR((float)i,512.0f))));
   } /* end for */
   for (i = 0UL; i<=511UL; i++) {
      dfirtab[511UL+i] = t[i];
      dfirtab[511UL-i] = t[i];
   } /* end for */
} /* end initdfir() */


static void initafir(AFIRTAB atab, uint32_t F0, uint32_t F1, float eq)
{
   uint32_t f;
   uint32_t i;
   float t[256];
   float f10;
   float f00;
   float e;
   uint32_t tmp;
   f00 = X2C_DIVR((float)(F0*32UL),(float)adcrate);
   f10 = X2C_DIVR((float)(F1*32UL),(float)adcrate);
   for (i = 0UL; i<=255UL; i++) {
      t[i] = 0.0f;
   } /* end for */
   tmp = (uint32_t)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL;
   f = (uint32_t)X2C_TRUNCC(f00,0UL,X2C_max_longcard);
   if (f<=tmp) for (;; f++) {
      e = 1.0f+eq*((X2C_DIVR((float)f,X2C_DIVR((float)((F0+F1)*32UL), (float)adcrate)))*2.0f-1.0f);
      /*
          e:=1.0 + eq*(FLOAT(f)/FLOAT((F0+F1)*AFIRLEN DIV adcrate)*2.0-1.0);
      */
      if (e<0.0f) e = 0.0f;
      if (f==(uint32_t)X2C_TRUNCC(f00,0UL,X2C_max_longcard)) {
         e = e*(1.0f-(f00-(float)(uint32_t)X2C_TRUNCC(f00,0UL,X2C_max_longcard)));
      }
      if (f==(uint32_t)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL) {
         e = e*(f10-(float)(uint32_t)X2C_TRUNCC(f10,0UL, X2C_max_longcard));
      }
      /*
      IF eq<>0 THEN IO.WrFixed(e,2,2);IO.WrLn; END;
      */
      if (f==0UL) {
         for (i = 0UL; i<=255UL; i++) {
            t[i] = t[i]+e*0.5f;
         } /* end for */
      }
      else {
         for (i = 0UL; i<=255UL; i++) {
            t[i] = t[i]+e*osic_cos(X2C_DIVR(3.1415926535898f*(float)(i*f),
                256.0f));
         } /* end for */
      }
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 256ul);
   for (i = 0UL; i<=255UL; i++) {
      atab[255UL+i] = t[i];
      atab[255UL-i] = t[i];
   } /* end for */
   if (F0>0UL) {
      /* make dc level zero */
      e = 0.0f;
      for (i = 0UL; i<=511UL; i++) {
         e = e+atab[i];
      } /* end for */
      e = X2C_DIVR(e,512.0f);
      for (i = 0UL; i<=511UL; i++) {
         atab[i] = atab[i]-e;
      } /* end for */
   }
/*
IO.WrLn;
FOR i:=0 TO HIGH(atab) DO IO.WrFixed(atab[i], 2,8) END;
IO.WrLn;
*/
/*
IF eq<>0.0 THEN
debfd:=FIO.Create("/tmp/ta.raw");
FOR i:=0 TO HIGH(atab) DO f:=VAL(INTEGER, atab[i]*1000.0);
                FIO.WrBin(debfd,f,2) END;
FIO.Close(debfd);
END;
*/
} /* end initafir() */


static void OpenSound(void)
{
   int32_t s;
   int32_t i;
   soundfd = osi_OpenRW(soundfn, 1024ul);
   if (soundfd>=0L) {
      if (maxchannels<2UL) {
         i = samplesize(soundfd, 16UL); /* 8, 16 */
         i = channels(soundfd, maxchannels+1UL); /* 1, 2  */
         i = setfragment(soundfd, fragmentsize); /* 2^bufsize * 65536*bufs*/
         if (i) {
            osi_WrStr("sound setfragment returns ", 27ul);
            osic_WrINT32((uint32_t)i, 1UL);
            osic_WrLn();
         }
         i = sampelrate(soundfd, adcrate); /* 8000..48000 */
         s = (int32_t)getsampelrate(soundfd);
         if (s!=(int32_t)adcrate) {
            osi_WrStr("sound device returns ", 22ul);
            osic_WrINT32((uint32_t)s, 1UL);
            osi_WrStrLn("Hz!", 4ul);
         }
      }
   }
   else if (abortonsounderr) {
      osi_WrStr(soundfn, 1024ul);
      Error(" open", 6ul);
   }
} /* end OpenSound() */


static char packcall(char cs[], uint32_t cs_len,
                uint32_t * cc, char * ssid)
{
   uint32_t s;
   uint32_t j;
   uint32_t i;
   char c;
   char packcall_ret;
   X2C_PCOPY((void **)&cs,cs_len);
   cs[cs_len-1] = 0;
   *cc = 0UL;
   s = 0UL;
   i = 0UL;
   for (j = 0UL; j<=5UL; j++) {
      *cc =  *cc*37UL;
      c = cs[i];
      if ((uint8_t)c>='A' && (uint8_t)c<='Z') {
         *cc += ((uint32_t)(uint8_t)c-65UL)+1UL;
         ++i;
      }
      else if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         *cc += ((uint32_t)(uint8_t)c-48UL)+27UL;
         ++i;
      }
      else if (c && c!='-') {
         packcall_ret = 0;
         goto label;
      }
   } /* end for */
   if (cs[i]=='-') {
      /* ssid */
      ++i;
      c = cs[i];
      if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         s += (uint32_t)(uint8_t)c-48UL;
         ++i;
         c = cs[i];
         if ((uint8_t)c>='0' && (uint8_t)c<='9') {
            s = (s*10UL+(uint32_t)(uint8_t)c)-48UL;
         }
      }
      if (s>15UL) {
         packcall_ret = 0;
         goto label;
      }
   }
   else if (cs[i]) {
      packcall_ret = 0;
      goto label;
   }
   *ssid = (char)s;
   packcall_ret = *cc>0UL;
   label:;
   X2C_PFREE(cs);
   return packcall_ret;
} /* end packcall() */


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * ip,
                uint32_t * port)
{
   uint32_t p;
   uint32_t n;
   uint32_t i;
   char ok0;
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((uint8_t)h[p]>='0' && (uint8_t)h[p]<='9') {
         ok0 = 1;
         n = (n*10UL+(uint32_t)(uint8_t)h[p])-48UL;
         ++p;
      }
      if (!ok0) {
         GetIp_ret = -1L;
         goto label;
      }
      if (i<3UL) {
         if (h[p]!='.' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
         *ip =  *ip*256UL+n;
      }
      else if (i==3UL) {
         *ip =  *ip*256UL+n;
         if (h[p]!=':' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port = n;
      ++p;
   } /* end for */
   GetIp_ret = openudp();
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static void Config(void)
{
   uint32_t c;
   uint32_t i;
   struct R92 * anonym;
   struct R41 * anonym0;
   struct DFM6 * anonym1;
   struct C34 * anonym2;
   struct PILS * anonym5;
   struct M10 * anonym6;
   struct M20 * anonym7;
   struct IMET * anonym8;
   struct MP3 * anonym9;

   for (c = 0UL; c<=63UL; c++) {
      { /* with */
         struct R92 * anonym = &chan[c].r92;
         anonym->configbaud = 4800UL;
         anonym->demodbaud = (2UL*anonym->configbaud*65536UL)/adcrate;
         initafir(anonym->afirtab, 0UL, 2800UL,X2C_DIVR((float)chan[c].configequalizer,100.0f));
	 //initafir(anonym->afirtab, 0UL,   12600UL,X2C_DIVR((float)chan[c].configequalizer,100.0f)); //12600
         anonym->baudfine = 0L;
         anonym->noise = 0.0f;
         anonym->bitlev = 0.0f;
         anonym->cbit = 0;
         anonym->rxp = 0UL;
         anonym->rxbitc = 0UL;
         anonym->manchestd = 0L;
         anonym->lastmanch = 0;
         anonym->rxbyte = 0UL;
         for (i = 0UL; i<=9UL; i++) {
            anonym->asynst[i] = 0L;
         } /* end for */
      }
      { /* with */
         struct R41 * anonym0 = &chan[c].r41;
         anonym0->configbaud = 4800UL;
         anonym0->demodbaud = (2UL*anonym0->configbaud*65536UL)/adcrate;
         initafir(anonym0->afirtab, 0UL, 2800UL, X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym0->baudfine = 0L;
         anonym0->noise = 0.0f;
         anonym0->bitlev = 0.0f;
         anonym0->cbit = 0;
         anonym0->rxp = 0UL;
         anonym0->rxbitc = 0UL;
         anonym0->rxbyte = 0UL;
         anonym0->synp = 0UL;
      }
      { /* with */
         struct MP3 * anonym9 = &chan[c].mp3;
         anonym9->configbaud = 2400UL;
         anonym9->demodbaud = (2UL*anonym9->configbaud*65536UL)/adcrate;
         initafir(anonym9->afirtab, 0UL, 2400UL, X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym9->baudfine = 0L;
         anonym9->noise = 0.0f;
         anonym9->bitlev = 0.0f;
         anonym9->cbit = 0;
         anonym9->rxp = 0UL;
         anonym9->rxbitc = 0UL;
         anonym9->rxbyte = 0UL;
         anonym9->synp = 0UL;
      }

      { // with
         struct PILS * anonym5 = &chan[c].pils;
         anonym5->configbaud = 4804UL;
         anonym5->demodbaud = (2UL*anonym5->configbaud*65536UL)/adcrate; //4800
//         initafir(anonym5->afirtab, 0UL, 2200UL, X2C_DIVR((float)(chan[c].configequalizer+120),100.0f)); //V9
	initafir(anonym5->afirtab, 0UL, 2200UL, X2C_DIVR((float)(chan[c].configequalizer+60),100.0f)); //v10	
//         initafir(anonym5->afirtab, 0UL, 9600UL, X2C_DIVR((float)(chan[c].configequalizer),100.0f));
         anonym5->baudfine = 0L;
         anonym5->noise = 0.0f;
         anonym5->bitlev = 0.0f;
         anonym5->cbit = 0;
         anonym5->rxp = 0UL;
         anonym5->rxbitc = 0UL;
         anonym5->rxbyte = 0UL;
         anonym5->synp = 0UL;
	 anonym5->poklat=-1;
	 anonym5->poklon=-1;

      }


      { /* with */
         struct DFM6 * anonym1 = &chan[c].dfm6;
         anonym1->configbaud = 2500UL;
         anonym1->demodbaud = (2UL*anonym1->configbaud*65536UL)/adcrate;
         initafir(anonym1->afirtab, 0UL, 1900UL, X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym1->baudfine = 0L;
         anonym1->noise = 0.0f;
         anonym1->bitlev = 0.0f;
         anonym1->cbit = 0;
         anonym1->rxp = 264UL; /* out of fram, wait for sync */
         anonym1->manchestd = 0L;
         anonym1->polarity = 0;
         anonym1->numcnt = 0UL;
         anonym1->txok = 0;
	 anonym1->newsonde=1;
      }

      { /* with */
         struct M10 * anonym6 = &chan[c].m10;
         anonym6->configbaud = 9600UL;
         anonym6->demodbaud = (2UL*anonym6->configbaud*65536UL)/adcrate;
         initafir(anonym6->afirtab, 0UL, 5200UL, X2C_DIVR((float)chan[c].configequalizer,100.0f)); //5200
         anonym6->baudfine = 0L;
         anonym6->noise = 0.0f;
         anonym6->bitlev = 0.0f;
         anonym6->cbit = 0;
         anonym6->rxp = 101UL; /* out of fram, wait for sync */
         anonym6->manchestd = 0L;
         anonym6->txok = 0;
      }

      { /* with */
         struct M20 * anonym7 = &chan[c].m20;
         anonym7->configbaud = 9600UL;
         anonym7->demodbaud = (2UL*anonym7->configbaud*65536UL)/adcrate;
         initafir(anonym7->afirtab, 0UL, 5200UL, X2C_DIVR((float)chan[c].configequalizer,100.0f)); //5200
         anonym7->baudfine = 0L;
         anonym7->noise = 0.0f;
         anonym7->bitlev = 0.0f;
         anonym7->cbit = 0;
         anonym7->rxp = 70UL; /* out of fram, wait for sync */
         anonym7->manchestd = 0L;
         anonym7->txok = 0;
      }

      { /* with */
         struct C34 * anonym2 = &chan[c].c34;
         anonym2->txbaud = (anonym2->configbaud*65536UL)/adcrate;
         anonym2->demodbaud = anonym2->txbaud*2UL;
         anonym2->afskmidfreq = X2C_DIVR((float)anonym2->configafskmid*2.0f,(float)adcrate);
         initafir(anonym2->afirtab,
		 (anonym2->configafskmid-anonym2->configafskshift/2UL)-anonym2->configbaud/4UL, anonym2->configafskmid+anonym2->configafskshift/2UL+anonym2->configbaud/4UL, X2C_DIVR((float)chan[c].configequalizer,100.0f));
         initdfir(anonym2->dfirtab,(anonym2->configbaud*anonym2->confignyquist)/100UL);
         anonym2->baudfine = 0L;
         anonym2->left = 0.0f;
         anonym2->tcnt = 0.0f;
         anonym2->freq = 0.0f;
         anonym2->dfin = 0UL;
         anonym2->cbit = 0;
         anonym2->rxp = 0UL;
         anonym2->rxbitc = 0UL;
      }
      { /* with */
         struct IMET * anonym2 = &chan[c].imet;
         anonym2->txbaud = (anonym2->configbaud*65536UL)/adcrate;
         anonym2->demodbaud = anonym2->txbaud*2UL;
         anonym2->afskmidfreq = X2C_DIVR((float)anonym2->configafskmid*2.0f,(float)adcrate);
         initafir(anonym2->afirtab,
		 (anonym2->configafskmid-anonym2->configafskshift/2UL)-anonym2->configbaud/4UL, anonym2->configafskmid+anonym2->configafskshift/2UL+anonym2->configbaud/4UL, X2C_DIVR((float)chan[c].configequalizer,100.0f));
         initdfir(anonym2->dfirtab,(anonym2->configbaud*anonym2->confignyquist)/100UL);
         anonym2->baudfine = 0L;
         anonym2->left = 0.0f;
         anonym2->tcnt = 0.0f;
         anonym2->freq = 0.0f;
         anonym2->dfin = 0UL;
         anonym2->cbit = 0;
         anonym2->rxp = 0UL;
         anonym2->rxbitc = 0UL;
      }
   } /* end for */
} /* end Config() */


static void Parms(void)
{
   char err;
   FILENAME mixfn;
   FILENAME h1;
   FILENAME h;
   uint32_t ch;
   uint32_t cnum;
   int32_t inum;
   uint32_t channel;
   pUDPTX utx;
   char chanset;
   char mycall[11];
   uint32_t myc;
   char mys;
   struct R92 * anonym;
   struct R41 * anonym0;
   struct DFM6 * anonym1;
   struct C34 * anonym2;
   struct CHAN * anonym3;
   /* set only 1 chan */
   struct CHAN * anonym4;
   struct PILS * anonym5;
   struct M10 * anonym6;
   struct M20 * anonym7;
   struct IMET * anonym8;
   struct MP3 * anonym9;
   err = 0;
   abortonsounderr = 0;
   adcrate = 22050UL;
   adcbuflen = 1024UL;
   fragmentsize = 11UL;
   maxchannels = 0UL;
   cfgchannels = 1UL; /* fix 1 channel */
   debfd = -1L;
   chanset = 0;
   dfmnametyp = 0UL;
   dfmidchg = 2UL; /* minutes no tx if dfm name change */
   for (channel = 0UL; channel<=63UL; channel++) {
      { /* with */
         struct R92 * anonym = &chan[channel].r92;
         anonym->enabled = 1;
         anonym->pllshift = 1024L;
      }
      { /* with */
         struct R41 * anonym0 = &chan[channel].r41;
         anonym0->enabled = 1;
         anonym0->pllshift = 1024L;
      }
      { /* with */
         struct MP3 * anonym0 = &chan[channel].mp3;
         anonym0->enabled = 1;
         anonym0->pllshift = 1024L;
      }

      //------pilot sonde
      { // with 
         struct PILS * anonym5 = &chan[channel].pils;
         anonym5->enabled = 1;
         anonym5->pllshift = 1024L;
      }
      //---pilot sonde^
      { /* with */
         struct DFM6 * anonym1 = &chan[channel].dfm6;
         anonym1->enabled = 1;
         anonym1->pllshift = 1024L;
         anonym1->idcnt0 = 0UL;
         anonym1->idcnt1 = 0UL;
      }
      { /* with */
         struct C34 * anonym2 = &chan[channel].c34;
         anonym2->enabled = 1;
         anonym2->pllshift = 4096L;
         anonym2->confignyquist = 65UL;
         anonym2->afskhighpass = 0.0f;
         anonym2->configbaud = 2400UL;
         anonym2->configafskshift = 1800UL;
         anonym2->configafskmid = 3800UL;
         anonym2->id34.id[0] = 0;
         anonym2->id34.idcheck[0] = 0;
         anonym2->id34.idtime = 0UL;
         anonym2->id34.idcnt = 0UL;
         anonym2->id50.id[0] = 0;
         anonym2->id50.idcheck[0] = 0;
         anonym2->id50.idtime = 0UL;
         anonym2->id50.idcnt = 0UL;
      }
      { /* with */
         struct IMET * anonym8 = &chan[channel].imet;
         anonym8->enabled = 1;
         anonym8->pllshift = 4096L;
         anonym8->confignyquist = 65UL;
         anonym8->afskhighpass = 0.0f;
         anonym8->configbaud = 2400UL;
         anonym8->configafskshift = 1800UL;
         anonym8->configafskmid = 3800UL;
         anonym8->imet.id[0] = 0;
         anonym8->imet.idcheck[0] = 0;
         anonym8->imet.idtime = 0UL;
         anonym8->imet.idcnt = 0UL;
         anonym8->imet.id[0] = 0;
         anonym8->imet.idcheck[0] = 0;
         anonym8->imet.idtime = 0UL;
         anonym8->imet.idcnt = 0UL;
      }
      { /* with */
         struct M10 * anonym6 = &chan[channel].m10;
         anonym6->enabled = 1;
         anonym6->pllshift = 4096L;
      }
      { /* with */
         struct M20 * anonym7 = &chan[channel].m20;
         anonym7->enabled = 1;
         anonym7->pllshift = 4096L;
      }

      { /* with */
         struct CHAN * anonym3 = &chan[channel];
         anonym3->configequalizer = 0L;
         anonym3->udptx = 0;
         anonym3->mycallc = 0UL;
      }

   } /* end for */
   channel = 0UL;
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='a') abortonsounderr = 1;
         else if (h[1U]=='3') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].c34.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].c34.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='9') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].r92.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].r92.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='4') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].r41.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].r41.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='6') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].dfm6.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].dfm6.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='1') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].m10.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].m10.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='2') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].m20.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].m20.enabled = 0;
               } /* end for */
            }
         }

         else if (h[1U]=='5') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].imet.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].imet.enabled = 0;
               } /* end for */
            }
         }

	 //---------------pilot sonde
	 else if (h[1U]=='8') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].pils.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].pils.enabled = 0;
               } /* end for */
            }
         }

	 //---------------pilot sonde^

	 else if (h[1U]=='A') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].mp3.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].mp3.enabled = 0;
               } /* end for */
            }
         }


         else if (h[1U]=='N') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmnametyp)) err = 1;
            dfmnametyp += 512UL;
         }
         else if (h[1U]=='n') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmnametyp)) err = 1;
            dfmnametyp += 256UL;
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=63UL) Error("maxchannels 0..max", 19ul);
            cfgchannels = cnum;
            if (cfgchannels>0UL) maxchannels = cfgchannels-1UL;
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>1UL) Error("channel 0 to max", 17ul);
            channel = cnum;
            chanset = 1;
         }
         else if (h[1U]=='D') {
            osi_NextArg(h1, 1024ul);
            debfd = osi_OpenWrite(h1, 1024ul);
         }
         else if (h[1U]=='e') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
            if (labs(inum)>999L) Error("equalizer -999..999", 20ul);
            chan[channel].configequalizer = inum;
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<8000UL || cnum>96000UL) {
               Error("sampelrate 8000..96000", 23ul);
            }
            adcrate = cnum;
         }
         else if (h[1U]=='F') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            chan[channel].c34.configafskmid = cnum;
         }
         else if (h[1U]=='G') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmidchg)) err = 1;
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
         }
         else if (h[1U]=='o') osi_NextArg(soundfn, 1024ul);
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 11ul);
            if (!packcall(mycall, 11ul, &myc, &mys)) {
               Error("-I illegall Callsign + ssid", 28ul);
            }
            if (chanset) {
               { /* with */
                  struct CHAN * anonym4 = &chan[channel];
                  anonym4->mycallc = myc;
                  anonym4->myssid = mys;
               }
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].mycallc = myc;
                  chan[ch].myssid = mys;
               } /* end for */
            }
         }
         else if (h[1U]=='u') {
            osi_NextArg(h, 1024ul);
            osic_alloc((char * *) &utx, sizeof(struct UDPTX));
            if (utx==0) Error("udp socket out of memory", 25ul);
            utx->udpfd = GetIp(h, 1024ul, &utx->ip, &utx->destport);
            if (utx->udpfd<0L) Error("cannot open udp socket", 23ul);
            if (chanset) {
               /* set only 1 chan */
               utx->next = chan[channel].udptx;
               chan[channel].udptx = utx;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  utx->next = chan[ch].udptx;
                  chan[ch].udptx = utx;
               } /* end for */
            }
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("oss Mono/Stereo up to 64 Channel RS92, RS41, C34, C50 Sonde Demodulator to raw Frames", 86ul);
               osi_WrStrLn("sent via UDP to \'sondemod\' decoder, more demodulators may send to same decoder", 79ul);
               osi_WrStrLn("Stereo used for 2 Rx for 2 Sondes or 1 Sonde with Antenna-Diversity", 68ul);
	       osi_WrStrLn(" -1             disable M10 decoding (use -C before to select 1 channel)", 73ul);
	       osi_WrStrLn(" -2             disable M20 decoding (use -C before to select 1 channel)", 73ul);
               osi_WrStrLn(" -3             disable SRSC34/50 decoding (use -C before to select 1 channel)", 79ul);
               osi_WrStrLn(" -4             disable RS41 decoding (use -C before to select 1 channel)", 74ul);
               osi_WrStrLn(" -5             disable IMET decoding (use -C before to select 1 channel)", 74ul);
               osi_WrStrLn(" -6             disable DFM06 decoding (use -C before to select 1 channel)", 75ul);
               osi_WrStrLn(" -8             disable PILOTSONDE decoding (use -C before to select 1 channel)", 74ul);
               osi_WrStrLn(" -9             disable RS92 decoding (use -C before to select 1 channel)", 74ul);
	       osi_WrStrLn(" -A             disable MP3 decoding (use -C before to select 1 channel)", 73ul);
               osi_WrStrLn(" -a             abort on sounddevice error else retry to open (USB audio...)", 77ul);
               osi_WrStrLn(" -c <num>       maxchannels, 0 for automatic channel number recognition", 72ul);
               osi_WrStrLn(" -C <num>       channel parameters follow (repeat for each channel)", 68ul);
               osi_WrStrLn(" -D <filename>  write raw soundcard input data to file or pipe", 63ul);
               osi_WrStrLn("                for debug or chaining demodulators (equalizer diversity)", 73ul);
               osi_WrStrLn(" -e <num>       demod equalizer (0) 100=6db/oct highpass (-999..999)", 69ul);
               osi_WrStrLn("                -C <n> before -e sets channel number", 53ul);
               osi_WrStrLn(" -f <num>       adcrate (22050) (8000..96000)", 46ul);
               osi_WrStrLn(" -G <minutes>   no tx if DFMxx Name changes (2)", 48ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -I <call>      mycall + ssid (use -C before to select 1 channel)", 66ul);
               osi_WrStrLn(" -l <num>       adcbuflen (256)", 32ul);
               osi_WrStrLn(" -N <num>       0..255 generate DFM-ID from serial no. in first frame (see -v)", 79ul);
               osi_WrStrLn("                enter first byte in decimal \"AC00070\" -N 172", 61ul);
               osi_WrStrLn(" -n <num>       same as -N but try old methode too", 51ul);
               osi_WrStrLn(" -o <filename>  oss devicename (/dev/dsp) or raw/wav audio file or pipe /dev/stdin", 83ul);
               osi_WrStrLn(" -u <x.x.x.x:destport> send rx data in udp (to sondemod), -C <n> before sets", 77ul);
               osi_WrStrLn("                channel number, maybe repeated for more destinations", 69ul);
               osi_WrStrLn(" -V             very verbous, with some hex dumps", 50ul);
               osi_WrStrLn(" -v             verbous, (CRC-checked Sondename)",  49ul);
               osi_WrStrLn("example: sondeudp -f 16000 -o /dev/dsp -c 2 -C 0 -e 50 -u 127.0.0.1:4000", 73ul);
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
   if (adcbuflen*(maxchannels+1UL)>4096UL) {
      adcbuflen = 4096UL/(maxchannels+1UL);
   }
   Config();
   OpenSound();
} /* end Parms() */


static void sendudp(char data[], uint32_t data_len, int32_t len,
                uint32_t ip, uint32_t destport, int32_t udpfd)
{
   int32_t i;
   X2C_PCOPY((void **)&data,data_len);
   i = udpsend(udpfd, data, len, destport, ip);
   X2C_PFREE(data);
} /* end sendudp() */


static void WrdB(int32_t volt)
{
   if (volt>0L) {
      osic_WrFixed(osic_ln((float)volt)*8.685889638f-96.4f, 1L, 6UL);
      osi_WrStr("dB", 3ul);
   }
} /* end WrdB() */


static void WrQ(float lev, float noise)
{
   if (lev>0.0f) {
      noise = X2C_DIVR(noise*200.0f,lev);
      if (noise>100.0f) noise = 100.0f;
      else if (noise<=0.0f) noise = 0.0f;
      osic_WrINT32((uint32_t)(100L-(int32_t)X2C_TRUNCI(noise,
                X2C_min_longint,X2C_max_longint)), 3UL);
      osi_WrStr("%", 2ul);
   }
} /* end WrQ() */


static void WrQuali(float q)
{
   if (q>0.0f) {
      q = 100.5f-q*200.0f;
      if (q<0.0f) q = 0.0f;
      osi_WrStr(" q:", 4ul);
      osic_WrINT32((uint32_t)osi_realint(q), 2UL);
   }
} /* end WrQuali() */


static void Wrtune(int32_t volt, int32_t max0)
{
   int32_t u;
   if (max0>0L && max0>labs(volt)) {
      u = (volt*100L)/max0;
      if (labs(u)>0L) {
         osi_WrStr(" f:", 4ul);
         osic_WrINT32((uint32_t)u, 2UL);
      }
      else osi_WrStr("     ", 6ul);
   }
} /* end Wrtune() */


static float noiselevel1(float bitlev, float noise)
/* 0.0 perfect, ~0.25 noise only*/
{
   if (bitlev==0.0f) return 0.0f;
   else return X2C_DIVR(noise,bitlev);
   return 0;
} /* end noiselevel() */



static float noiselevel(uint32_t channel)
/* 0.0 perfect, ~0.25 noise only*/
{
   struct C34 * anonym;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      if (anonym->sqmed[1]==anonym->sqmed[0]) return 0.0f;
      else {
         return X2C_DIVR(anonym->noise,(float)fabs(anonym->sqmed[1]-anonym->sqmed[0]));
      }
   }
   return 0;
} /* end noiselevel() */

static float noiselevelIMET(uint32_t channel)
/* 0.0 perfect, ~0.25 noise only*/
{
   struct IMET * anonym;
   { /* with */
      struct IMET * anonym = &chan[channel].imet;
      if (anonym->sqmed[1]==anonym->sqmed[0]) return 0.0f;
      else {
         return X2C_DIVR(anonym->noise,(float)fabs(anonym->sqmed[1]-anonym->sqmed[0]));
      }
   }
   return 0;
} /* end noiselevel() */


//            Fir(afin, 
//		(uint32_t)((anonym->baudfine&65535L)/4096L),
//		16UL, 
//		chan[m].afir, 
//		32ul, 
//		anonym->afirtab, 
//		512ul);
static float Fir(uint32_t in, uint32_t sub, uint32_t step, float fir[], uint32_t fir_len, float firtab[], uint32_t firtab_len)
{
   float s;
   uint32_t i;
   s = 0.0f;
   i = sub;
   uint32_t dfl=fir_len-1;
   uint32_t dftl=firtab_len-1;

   do {
      s = s+fir[in]*firtab[i];
      ++in;
      if (in>dfl) in = 0UL;
      i += step;
   } while (i<=dftl);
   return s;
} /* end Fir() */



static void alludp(pUDPTX utx, uint32_t len, char buf[], uint32_t buf_len)
{
   X2C_PCOPY((void **)&buf,buf_len);
   while (utx) {
      if (utx->udpfd>=0L) {
         sendudp(buf, buf_len, (int32_t)len, utx->ip, utx->destport,
                utx->udpfd);
      }
      utx = utx->next;
   }
   X2C_PFREE(buf);
} /* end alludp() */


static int32_t reedsolomon92(char buf[], uint32_t buf_len)
{
   uint32_t i;
   int32_t res;
   char b[256];
   uint32_t eraspos[24];
   for (i = 0UL; i<=255UL; i++) {
      b[i] = 0;
   } /* end for */
   for (i = 0UL; i<=209UL; i++) {
      b[230UL-i] = buf[i+6UL];
   } /* end for */
   for (i = 0UL; i<=23UL; i++) {
      b[254UL-i] = buf[i+216UL];
   } /* end for */
   res = decodersc(b, eraspos, 0L);
   if (res>0L && res<=12L) {
      for (i = 0UL; i<=209UL; i++) {
         buf[i+6UL] = b[230UL-i];
      } /* end for */
      for (i = 0UL; i<=23UL; i++) {
         buf[i+216UL] = b[254UL-i];
      } /* end for */
   }
   return res;
} /* end reedsolomon92() */


static char crcrs(const char frame[], uint32_t frame_len,
                int32_t from, int32_t to)
{
   uint16_t crc;
   int32_t i;
   int32_t tmp;
   crc = 0xFFFFU;
   tmp = to-3L;
   i = from;
   if (i<=tmp) for (;; i++) {
      crc = X2C_LSH(crc,16,-8)^CRCTAB[(uint32_t)((crc^(uint16_t)(uint8_t)frame[i])&0xFFU)];
      if (i==tmp) break;
   } /* end for */
   return frame[to-1L]==(char)crc && frame[to-2L]==(char)X2C_LSH(crc,
                16,-8);
} /* end crcrs() */

static uint16_t sondeudp_POLYNOM = 0x1021U;

void printCnDT(uint32_t m){

	int hours, minutes, seconds, day, month, year;
	time_t now;
	time(&now);
        struct tm *local = localtime(&now);

	printf("%02i: (%04d-%02d-%02d %02d:%02d:%02d):",m+1,local->tm_year + 1900,local->tm_mon + 1,local->tm_mday,local->tm_hour, local->tm_min, local->tm_sec);
}


static void decodeframe92(uint32_t m)
{
   uint32_t len;
   uint32_t p;
   uint32_t j;
   int32_t corr;
   struct CHAN * anonym;
   corr = reedsolomon92(chan[m].r92.rxbuf, 256ul);
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->mycallc>0UL) {
         chan[m].r92.rxbuf[0U] = (char)(anonym->mycallc/16777216UL);
         chan[m].r92.rxbuf[1U] = (char)(anonym->mycallc/65536UL&255UL);
         chan[m].r92.rxbuf[2U] = (char)(anonym->mycallc/256UL&255UL);
         chan[m].r92.rxbuf[3U] = (char)(anonym->mycallc&255UL);
         chan[m].r92.rxbuf[4U] = anonym->myssid;
      }
      alludp(anonym->udptx, 240UL, chan[m].r92.rxbuf, 256ul);
   }
   if (verb) {



      p = 6UL;
      if (chan[m].r92.rxbuf[6U]=='e') {
         ++p;
         len = (uint32_t)(uint8_t)chan[m].r92.rxbuf[7U]*2UL+2UL;
                /* +crc */
         ++p;
         if (maxchannels>0UL) {
	    printCnDT(m);
         }
         osi_WrStr("R92 ", 5ul);
         if (8UL+len>240UL || !crcrs(chan[m].r92.rxbuf, 256ul, 8L,
                (int32_t)(8UL+len))) osi_WrStr("----  crc err ", 15ul);
         else {
            j = 4UL;
            while ((uint8_t)chan[m].r92.rxbuf[8UL+j]>=' ') {
               osi_WrStr((char *) &chan[m].r92.rxbuf[8UL+j], 1u/1u);
               ++j;
            }
            osi_WrStr(" ", 2ul);
            osic_WrINT32((uint32_t)(uint8_t)
                chan[m].r92.rxbuf[8U]+(uint32_t)(uint8_t)
                chan[m].r92.rxbuf[9U]*256UL, 4UL);
         }
         /*      IF m>0 THEN WrStr("             ") END; */
         WrdB(chan[m].adcmax);
         WrQ(chan[m].r92.bitlev, chan[m].r92.noise);
         if (corr<0L) osi_WrStr(" -R", 4ul);
         else if (corr>0L && corr<=12L) {
            osi_WrStr(" +", 3ul);
            osic_WrINT32((uint32_t)corr, 1UL);
            osi_WrStr("R", 2ul);
         }
         Wrtune(chan[m].adcdc, chan[m].adcmax);
         osi_WrStrLn("", 1ul);
      }
   }
} /* end decodeframe92() */


static double latlong(uint32_t val, char c50)
{
   double hf;
   double hr;
   hr = (double)(val%0x080000000UL);
   if (c50) hr = X2C_DIVL(hr,1.E+7);
   else hr = X2C_DIVL(hr,1.E+6);
   hf = (double)(uint32_t)X2C_TRUNCC(hr,0UL,X2C_max_longcard);
   hr = hf+X2C_DIVL(hr-hf,0.6);
   if (val>=0x080000000UL) hr = -hr;
   return hr;
} /* end latlong() */


static char hex(uint32_t n)
{
   n = n&15UL;
   if (n<10UL) return (char)(n+48UL);
   else return (char)(n+55UL);
   return 0;
} /* end hex() */

/*------------------------------ RS41 */

static void sendrs41(uint32_t m)
{
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->mycallc>0UL) {
         chan[m].r41.rxbuf[0U] = (char)(anonym->mycallc/16777216UL);
         chan[m].r41.rxbuf[1U] = (char)(anonym->mycallc/65536UL&255UL);
         chan[m].r41.rxbuf[2U] = (char)(anonym->mycallc/256UL&255UL);
         chan[m].r41.rxbuf[3U] = (char)(anonym->mycallc&255UL);
         chan[m].r41.rxbuf[4U] = anonym->myssid;
         chan[m].r41.rxbuf[5U] = 0;
         chan[m].r41.rxbuf[6U] = 0;
      }
      alludp(anonym->udptx, 520UL, chan[m].r41.rxbuf, 520ul);
   }
} /* end sendrs41() */


static double atang2(double x, double y)
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
} /* end atang2() */

#define sondeudp_EARTHA 6.378137E+6

#define sondeudp_EARTHB 6.3567523142452E+6

#define sondeudp_E2 6.6943799901413E-3

#define sondeudp_EARTHAB 4.2841311513312E+4


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
      *long0 = atang2(xh, y)*2.0;
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
/*  lat:=atan(z/(rh*(1.0 - E2))); */
/*  heig:=sqrt(h + z*z) - EARTHA; */
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


/*---------------------- M10 */

static uint16_t crcm10(int32_t len, const char buf[], uint32_t buf_len)
{
   int32_t i;
   uint16_t s;
   uint16_t t;
   uint16_t b;
   uint16_t cs;
   int32_t tmp;
   cs = 0U;
   tmp = len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (uint16_t)(uint32_t)(uint8_t)buf[i];
      b = X2C_LSH(b,16,-1)|X2C_LSH(b&0x1U,16,7);
      b = b^X2C_LSH(b,16,-2)&0xFFU;
      t = cs&0x3FU|X2C_LSH((cs^X2C_LSH(cs,16,-2)^X2C_LSH(cs,16,-4))&0x1U,16,
                6)|X2C_LSH((X2C_LSH(cs,16,-1)^X2C_LSH(cs,16,-3)^X2C_LSH(cs,
                16,-5))&0x1U,16,7);
      s = X2C_LSH(cs,16,-7)&0xFFU;
      s = (s^X2C_LSH(s,16,-2))&0xFFU;
      cs = X2C_LSH(cs&0xFFU,16,8)|b^t^s;
      if (i==tmp) break;
   } /* end for */
   return (uint16_t)cs;
} /* end crcm10() */


static uint32_t m10card(const char b[], uint32_t b_len, int32_t pos, int32_t len)
{
   int32_t i;
   uint32_t n;
   int32_t tmp;
   n = 0UL;
   tmp = len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      n = n*256UL+(uint32_t)(uint8_t)b[pos+i];
      if (i==tmp) break;
   } /* end for */
   return n;
} /* end m10card() */

typedef uint32_t SET256[8];

static float sondeudp_DEGMUL = 8.3819036711397E-8f;

#define sondeudp_VMUL 0.005

static SET256 sondeudp_HSET = {0x03FFFFF0UL,0x00000003UL,0x00000000UL,
                0x00000018UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL}; /* not hexlist known bytes */

static SET256 _cnst1 = {0x03FFFFF0UL,0x00000003UL,0x00000000UL,0x00000018UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL};

static void WrChan(int32_t c)
{
   if (maxchannels>0UL) {
      printf("%02i:",c+1);
   }
} /* end WrChan() */



// Temperatur Sensor
// NTC-Thermistor Shibaura PB5-41E
//
float M10get_Temp(uint32_t m) {
// NTC-Thermistor Shibaura PB5-41E
// T00 = 273.15 +  0.0 , R00 = 15e3
// T25 = 273.15 + 25.0 , R25 = 5.369e3
// B00 = 3450.0 Kelvin // 0C..100C, poor fit low temps
// [  T/C  , R/1e3 ] ( [P__-43]/2.0 ):
// [ -50.0 , 204.0 ]
// [ -45.0 , 150.7 ]
// [ -40.0 , 112.6 ]
// [ -35.0 , 84.90 ]
// [ -30.0 , 64.65 ]
// [ -25.0 , 49.66 ]
// [ -20.0 , 38.48 ]
// [ -15.0 , 30.06 ]
// [ -10.0 , 23.67 ]
// [  -5.0 , 18.78 ]
// [   0.0 , 15.00 ]
// [   5.0 , 12.06 ]
// [  10.0 , 9.765 ]
// [  15.0 , 7.955 ]
// [  20.0 , 6.515 ]
// [  25.0 , 5.370 ]
// [  30.0 , 4.448 ]
// [  35.0 , 3.704 ]
// [  40.0 , 3.100 ]
// -> Steinhart–Hart coefficients (polyfit):
    float p0 = 1.07303516e-03,
          p1 = 2.41296733e-04,
          p2 = 2.26744154e-06,
          p3 = 6.52855181e-08;
// T/K = 1/( p0 + p1*ln(R) + p2*ln(R)^2 + p3*ln(R)^3 )

    // range/scale 0, 1, 2:                        // M10-pcb
    float Rs[3] = { 12.1e3 ,  36.5e3 ,  475.0e3 }; // bias/series
    float Rp[3] = { 1e20   , 330.0e3 , 3000.0e3 }; // parallel, Rp[0]=inf

    uint8_t  scT;     // {0,1,2}, range/scale voltage divider
    uint16_t ADC_RT;  // ADC12 P6.7(A7) , adr_0377h,adr_0376h
    uint16_t Tcal[2]; // adr_1000h[scT*4]

    float adc_max = 4095.0; // ADC12
    float x, R;
    float T = 0;    // T/Kelvin

    struct M10 * anonym = &chan[m].m10;

    scT     =  (unsigned char)anonym->rxbuf[0x3E]; // adr_0455h
    ADC_RT  = ((unsigned char)anonym->rxbuf[0x40] << 8) | (unsigned char)anonym->rxbuf[0x3F];
    ADC_RT -= 0xA000;
    Tcal[0] = ((unsigned char)anonym->rxbuf[0x42] << 8) | (unsigned char)anonym->rxbuf[0x41];
    Tcal[1] = ((unsigned char)anonym->rxbuf[0x44] << 8) | (unsigned char)anonym->rxbuf[0x43];

    x = (adc_max-ADC_RT)/ADC_RT;  // (Vcc-Vout)/Vout
    if (scT < 3) R =  Rs[scT] /( x - Rs[scT]/Rp[scT] );
    else         R = -1;

    if (R > 0)  T =  1/( p0 + p1*log(R) + p2*log(R)*log(R) + p3*log(R)*log(R)*log(R) );
/*
//    if (option_verbose >= 3 && csO) { // on-chip temperature
        uint16_t ADC_Ti_raw = (anonym->rxbuf[0x49] << 8) | anonym->rxbuf[0x48]; // int.temp.diode, ref: 4095->1.5V
        float vti, ti;
        // INCH1A (temp.diode), slau144
        vti = ADC_Ti_raw/4095.0 * 1.5; // V_REF+ = 1.5V, no calibration
        ti = (vti-0.986)/0.00355;      // 0.986/0.00355=277.75, 1.5/4095/0.00355=0.1032
        fprintf(stdout, "  (Ti:%.1fC)", ti);
        // SegmentA-Calibration:
        //ui16_t T30 = adr_10e2h; // CAL_ADC_15T30
        //ui16_t T85 = adr_10e4h; // CAL_ADC_15T85
        //float  tic = (ADC_Ti_raw-T30)*(85.0-30.0)/(T85-T30) + 30.0;
        //fprintf(stdout, "  (Tic:%.1fC)", tic);
//    }
*/
    return  T - 273.15; // Celsius
}
/*
frame[0x32]: adr_1074h
frame[0x33]: adr_1075h
frame[0x34]: adr_1076h
frame[0x35..0x37]: TBCCR1 ; relHumCap-freq
frame[0x38]: adr_1078h
frame[0x39]: adr_1079h
frame[0x3A]: adr_1077h
frame[0x3B]: adr_100Ch
frame[0x3C..3D]: 0
frame[0x3E]: scale_index ; scale/range-index
frame[0x3F..40] = ADC12_A7 | 0xA000, V_R+=AVcc ; Thermistor
frame[0x41]: adr_1000h[scale_index*4]
frame[0x42]: adr_1000h[scale_index*4+1]
frame[0x43]: adr_1000h[scale_index*4+2]
frame[0x44]: adr_1000h[scale_index*4+3]
frame[0x45..46]: ADC12_A5/4, V_R+=2.5V
frame[0x47]: ADC12_A2/16 , V_R+=2.5V
frame[0x48..49]: ADC12_iT, V_R+=1.5V (int.Temp.diode)
frame[0x4C..4D]: ADC12_A6, V_R+=2.5V
frame[0x4E..4F]: ADC12_A3, V_R+=AVcc
frame[0x50..54]: 0;
frame[0x55..56]: ADC12_A1, V_R+=AVcc
frame[0x57..58]: ADC12_A0, V_R+=AVcc
frame[0x59..5A]: ADC12_A4, V_R+=AVcc  // ntc2: R(25C)=2.2k, Rs=22.1e3 (relHumCap-Temp)
frame[0x5B]:
frame[0x5C]: adr_108Eh
frame[0x5D]: adr_1082h (SN)
frame[0x5E]: adr_1083h (SN)
frame[0x5F]: adr_1084h (SN)
frame[0x60]: adr_1080h (SN)
frame[0x61]: adr_1081h (SN)
*/
float M10get_Tntc2(uint32_t m) {
// SMD ntc
    float Rs = 22.1e3;          // P5.6=Vcc
//  float R25 = 2.2e3;
//  float b = 3650.0;           // B/Kelvin
//  float T25 = 25.0 + 273.15;  // T0=25C, R0=R25=5k
// -> Steinhart–Hart coefficients (polyfit):
    float p0 =  4.42606809e-03,
          p1 = -6.58184309e-04,
          p2 =  8.95735557e-05,
          p3 = -2.84347503e-06;
    float T = 0.0;              // T/Kelvin
    uint16_t ADC_ntc2;            // ADC12 P6.4(A4)
    float x, R;
    struct M10 * anonym = &chan[m].m10;

//    if (csOK)
//    {
        ADC_ntc2  = ((unsigned char)anonym->rxbuf[0x5A] << 8) | (unsigned char)anonym->rxbuf[0x59];
        x = (4095.0 - ADC_ntc2)/ADC_ntc2;  // (Vcc-Vout)/Vout
        R = Rs / x;
        //if (R > 0)  T = 1/(1/T25 + 1/b * log(R/R25));
        if (R > 0)  T =  1/( p0 + p1*log(R) + p2*log(R)*log(R) + p3*log(R)*log(R)*log(R) );
//    }
    return T - 273.15;
}

// Humidity Sensor
// U.P.S.I.
//
#define FREQ_CAPCLK (8e6/2)      // 8 MHz XT2 crystal, InputDivider IDx=01 (/2)
#define LN2         0.693147181
#define ADR_108A    1000.0       // 0x3E8=1000

float get_count_RH(uint32_t m) {  // capture 1000 rising edges
    struct M10 * anonym = &chan[m].m10;
    uint32_t TBCCR1_1000 = anonym->rxbuf[0x35] | (anonym->rxbuf[0x36]<<8) | (anonym->rxbuf[0x37]<<16);
    return TBCCR1_1000 / ADR_108A;
}
float get_TLC555freq(uint32_t m) {
    return FREQ_CAPCLK / get_count_RH(m);
}


static void decodeframe10(uint32_t m)
{
   uint32_t week;
   uint32_t tow;
   uint32_t cs;
   uint32_t i;
   int32_t ci;
   double dir;
   double v;
   double vv;
   double vn;
   double ve;
   double alt;
   double lon;
   double lat;
   float vbat;
   float temp1,temp2;
   uint32_t time0;
   uint32_t id;
   char ids[201];
   //char s[201+6];
   char s[400];
    float fq555;
   struct M10 * anonym;
   struct CHAN * anonym0; /* call if set */
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      cs = (uint32_t)crcm10(99L, anonym->rxbuf, 101ul);
      if (cs==m10card(anonym->rxbuf, 101ul, 99L, 2L)) {
         /* crc ok */
	 if((unsigned char)anonym->rxbuf[1]==0x9f && (unsigned char)anonym->rxbuf[2]==0x20){
	
    	     tow = m10card(anonym->rxbuf, 101ul, 10L, 4L);
    	     week = m10card(anonym->rxbuf, 101ul, 32L, 2L);
    	     time0 = tow/1000UL+week*604800UL+315964800UL;
    	     if (verb2) {
        	osi_WrStr(" ", 2ul);
        	aprsstr_DateToStr(time0, s, 201ul);
        	osi_WrStr(s, 201ul);
        	osi_WrStr(" ", 2ul);
             } 
	     temp1=M10get_Temp(m);
	     temp2=M10get_Tntc2(m);
	     fq555 = get_TLC555freq(m);
	     vbat=(float)((256*(unsigned char)anonym->rxbuf[70]+(unsigned char)anonym->rxbuf[69])*0.00668);
             lat = (double)m10card(anonym->rxbuf, 101ul, 14L,4L)*8.3819036711397E-8;
             lon = (double)m10card(anonym->rxbuf, 101ul, 18L,4L)*8.3819036711397E-8;
             alt = (double)m10card(anonym->rxbuf, 101ul, 22L, 4L)*0.001;
             ci = (int32_t)m10card(anonym->rxbuf, 101ul, 4L, 2L);
             if (ci>32767L) ci -= 65536L;
             ve = (double)ci*0.005;
             ci = (int32_t)m10card(anonym->rxbuf, 101ul, 6L, 2L);
             if (ci>32767L) ci -= 65536L;
             vn = (double)ci*0.005;
             ci = (int32_t)m10card(anonym->rxbuf, 101ul, 8L, 2L);
             if (ci>32767L) ci -= 65536L;
             vv = (double)ci*0.005;
             v = (double)osic_sqrt((float)(ve*ve+vn*vn));
                // hor speed
             dir = atang2(vn, ve)*5.7295779513082E+1;
             if (dir<0.0) dir = 360.0+dir;
	 }
	 else if((unsigned char)anonym->rxbuf[1]==0xAF && (unsigned char)anonym->rxbuf[2]==0x02){
	     int tim=m10card(anonym->rxbuf, 101ul, 0x15, 3L);
	     int dat=m10card(anonym->rxbuf, 101ul, 0x18, 3L);
    	     time0 = 2678400*(dat%10000)/100 +86400*dat/10000 +3600*(tim/10000)+60*((tim%10000)/100)+ ((tim%100)/1); //tow/1000UL+week*604800UL+315964800UL;
    	     if (verb2) {
        	osi_WrStr(" ", 2ul);
        	aprsstr_DateToStr(time0, s, 201ul);
        	osi_WrStr(s, 201ul);
        	osi_WrStr(" ", 2ul);
             } 
	     temp1=M10get_Temp(m);
	     temp2=M10get_Tntc2(m);
	     fq555 = get_TLC555freq(m);
	     vbat=(float)((256*(unsigned char)anonym->rxbuf[70]+(unsigned char)anonym->rxbuf[69])*0.00668);
             lat = (double)m10card(anonym->rxbuf, 101ul, 0x04,4L)*1e-6;
             lon = (double)m10card(anonym->rxbuf, 101ul, 0x08,4L)*1e-6;
             alt = (double)m10card(anonym->rxbuf, 101ul, 0x0c, 3L)*0.01;
             ci = (int32_t)m10card(anonym->rxbuf, 101ul, 0x0f, 2L);
             if (ci>32767L) ci -= 65536L;
             ve = (double)ci*0.005;
             ci = (int32_t)m10card(anonym->rxbuf, 101ul, 0x11, 2L);
             if (ci>32767L) ci -= 65536L;
             vn = (double)ci*0.005;
             ci = (int32_t)m10card(anonym->rxbuf, 101ul, 0x13, 2L);
             if (ci>32767L) ci -= 65536L;
             vv = (double)ci*0.005;
             v = (double)osic_sqrt((float)(ve*ve+vn*vn));
                // hor speed
             dir = atang2(vn, ve)*5.7295779513082E+1;
             if (dir<0.0) dir = 360.0+dir;
	 } 



    int i,j,k,l;
    unsigned int byte;
    unsigned char sn_bytes[5];
    char SN[12];

    for (i = 0; i < 11; i++) SN[i] = ' '; SN[11] = '\0';

    for (i = 0; i < 5; i++) {
        byte = anonym->rxbuf[0x5d + i];
        sn_bytes[i] = byte;
    }

    byte = sn_bytes[2];
    sprintf(SN, "%1X%02u", (byte>>4)&0xF, byte&0xF);
    j=atoi(SN);
    k=(int)(j/25);
    l=j-k*25;
    byte = sn_bytes[3] | (sn_bytes[4]<<8);
    sprintf(SN+3, "%1X%1u%04u", sn_bytes[0]&0xF, (byte>>13)&0x7, byte&0x1FFF);
    sprintf(ids,SN);
    ids[9U] = 0;
    
         /* get ID */
         if (verb) {
            //WrChan((int32_t)m);
	    printCnDT(m);
            osi_WrStr("M10 ", 5ul);
            osi_WrStr(ids, 201ul);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)lat, 5L, 1UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)lon, 5L, 1UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)alt, 1L, 1UL);
            osi_WrStr("m ", 3ul);
            osic_WrFixed((float)(v*3.6), 1L, 1UL);
            osi_WrStr("km/h ", 6ul);
            osic_WrFixed((float)dir, 0L, 1UL);
            osi_WrStr("deg ", 5ul);
            osic_WrFixed((float)vv, 1L, 1UL);
            osi_WrStr("m/s ", 5ul);
	    osic_WrFixed((float)vbat, 1L, 1UL);
            osi_WrStr("V T1:", 6ul);
	    osic_WrFixed((float)temp1, 1L, 1UL);
            osi_WrStr("C T2:", 6ul);
	    osic_WrFixed((float)temp2, 1L, 1UL);
	    osi_WrStr("C ", 3ul);
         }
	struct CHAN * anonym0 = &chan[m];
    	s[0U] = (char)(anonym0->mycallc/16777216UL);
        s[1U] = (char)(anonym0->mycallc/65536UL&255UL);
        s[2U] = (char)(anonym0->mycallc/256UL&255UL);
        s[3U] = (char)(anonym0->mycallc&255UL);
        if (anonym0->mycallc>0UL) s[4U] = anonym0->myssid;
        else s[4U] = '\020';
	s[5]=',';
	for(i=0;i<6;i++)			//qrg
	    s[i+6]=chan[m].freq[i];
	s[12]=',';
	for(i=0;i<9;i++)			//nazwa
	    s[i+13]=ids[i];
	s[22]=0;
	if( lat>-90.0 && lat<90.0 && lon>=-180.0 && lon<=180.0 && alt>0.0 && alt<45000.0 && dir>=0 && dir<361 && v>=0 && v<600 && 
		vv>-200 && vv<200 && vbat>0 && vbat<10 && temp1>-270.0 && temp1<100.0 && temp2>-270.0 && temp2<100.0){
	    sprintf(s,"%s,%012lu,%09.5f,%010.5f,%05.0f,%03.0f,%05.1f,%05.1f,%05.2f,%06.1f,%06.1f,%06.0f\n",s,time0,lat,lon,alt,dir,v,vv,vbat,temp1,temp2,fq555);
//	    printf("\nM10T:%s",s);	
	    alludp(chan[m].udptx, 105, s, 105);
	}
      }
      else if (verb) {
         /*build tx frame */
         WrChan((int32_t)m);
         osi_WrStr("M10 crc error", 14ul);
      }
      if (verb) {
         WrdB(chan[m].adcmax-chan[m].adcmin);
         WrQuali(noiselevel1(anonym->bitlev, anonym->noise));
         Wrtune(chan[m].adcmax+chan[m].adcmin, chan[m].adcmax-chan[m].adcmin);
      }
      if (verb2) {
         for (i = 0UL; i<=23UL; i++) {
            if (i%10UL==0UL) osi_WrStrLn("", 1ul);
            osic_WrINT32(m10card(anonym->rxbuf, 101ul,
                (int32_t)(48UL+i*2UL), 2L), 6UL);
            osi_WrStr(" ", 2ul);
         } /* end for */
         for (i = 0UL; i<=100UL; i++) {
            if (i%24UL==0UL) osi_WrStrLn("", 1ul);
            if (X2C_INL(i,256,_cnst1)) osi_WrStr(" . ", 4ul);
            else osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[i], 3UL);
         } /* end for */
      }
      if (verb) osi_WrStrLn("", 1ul);
   }
} /* end decodeframe10() */


static void demodbyte10(uint32_t m, char d)
{
   struct M10 * anonym;
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      /*WrInt(ORD(d),1); Flush(); */
      anonym->synword = anonym->synword*2UL+(uint32_t)d;
      if (anonym->rxp>=101UL) {
         if ((anonym->synword&16777215UL)==6594336UL) {
            anonym->rxp = 3UL;
            anonym->rxb = 0UL;
            anonym->rxbuf[0U] = 'd';
            anonym->rxbuf[1U] = '\237';
            anonym->rxbuf[2U] = ' ';
         }
         if ((anonym->synword&16777215UL)==6598402UL) {
            anonym->rxp = 3UL;
            anonym->rxb = 0UL;
            anonym->rxbuf[0U] = 0x64;
            anonym->rxbuf[1U] = 0xAF;
            anonym->rxbuf[2U] = 0x02;
         }
      }
      else {
         /*WrStr(" -syn- "); */
         ++anonym->rxb;
         if (anonym->rxb>=8UL) {
            anonym->rxbuf[anonym->rxp] = (char)(anonym->synword&255UL);
            anonym->rxb = 0UL;
            ++anonym->rxp;
            if (anonym->rxp==101UL) decodeframe10(m);
         }
      }
   }
} /* end demodbyte10() */

static void demodbit10(uint32_t m, float u, float u0)
{
   char bit;
   char d;
   float ua;
   struct M10 * anonym;
   /*IF manchestd>20000 THEN  */
   /*WrInt(VAL(INTEGER, u), 8); Flush; */
   d = u>=0.0f;
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      /*WrInt(manchestd DIV 256, 1); WrStr("("); WrInt(ORD(lastmanch),1);
                WrInt(ORD(d),1);WrStr(")");  Flush(); */
      /*WrInt(ORD(lastmanch<>d),1); Flush(); */
      /*END; */
      if (anonym->lastmanch==d) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      bit = d!=anonym->lastmanch;
      if (anonym->manchestd>0L) {
         demodbyte10(m, bit);
         /*quality*/
         ua = (float)fabs(u)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.02f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.05f;
      }
      /*quality*/
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
   }
} /* end demodbit10() */


static void demod10(float u, uint32_t m)
{
   char d;
   struct M10 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      d = u>=0.0f;
      if (anonym->cbit) {
         demodbit10(m, u, anonym->lastu);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod10() */


/*---------------------- M10 */

/*---------------------- M20 */

#define pos_SN20 18

float M20get_Temp(uint32_t m) {
    float p0 = 1.07303516e-03,
          p1 = 2.41296733e-04,
          p2 = 2.26744154e-06,
          p3 = 6.52855181e-08;
// T/K = 1/( p0 + p1*ln(R) + p2*ln(R)^2 + p3*ln(R)^3 )

    // range/scale 0, 1, 2:                        // M10-pcb
    float Rs[3] = { 12.1e3 ,  36.5e3 ,  475.0e3 }; // bias/series
    float Rp[3] = { 1e20   , 330.0e3 , 3000.0e3 }; // parallel, Rp[0]=inf

    uint8_t  scT;     // {0,1,2}, range/scale voltage divider
    uint16_t ADC_RT;  // ADC12 P6.7(A7) , adr_0377h,adr_0376h
    uint16_t Tcal[2]; // adr_1000h[scT*4]

    float adc_max = 4095.0; // ADC12
    float x, R;
    float T = 0;    // T/Kelvin

    struct M20 * anonym = &chan[m].m20;

    scT     =  0; //(unsigned char)anonym->rxbuf[0x3E]; // adr_0455h
    ADC_RT  = ((unsigned char)anonym->rxbuf[0x05] << 8) | (unsigned char)anonym->rxbuf[0x04];

    printf("TEST: %04x\n",ADC_RT);
//    ADC_RT -= 0xA000;
//    Tcal[0] = ((unsigned char)anonym->rxbuf[0x42] << 8) | (unsigned char)anonym->rxbuf[0x41];
//    Tcal[1] = ((unsigned char)anonym->rxbuf[0x44] << 8) | (unsigned char)anonym->rxbuf[0x43];

    x = (adc_max-ADC_RT)/ADC_RT;  // (Vcc-Vout)/Vout
    if (scT < 3) R =  Rs[scT] /( x - Rs[scT]/Rp[scT] );
    else         R = -1;

    if (R > 0)  T =  1/( p0 + p1*log(R) + p2*log(R)*log(R) + p3*log(R)*log(R)*log(R) );
    return  (T - 273.15); // Celsius
}

float M20get_Tntc2(uint32_t m) {
// SMD ntc
    float Rs = 22.1e3;          // P5.6=Vcc
  float R25 = 2.2e3;
  float b = 3650.0;           // B/Kelvin
  float T25 = 25.0 + 273.15;  // T0=25C, R0=R25=5k
// -> Steinhart–Hart coefficients (polyfit):
    float p0 =  4.42606809e-03,
          p1 = -6.58184309e-04,
          p2 =  8.95735557e-05,
          p3 = -2.84347503e-06;
    float T = 0.0;              // T/Kelvin
    uint16_t ADC_ntc2;            // ADC12 P6.4(A4)
    float x, R;
    struct M20 * anonym = &chan[m].m10;

//    if (csOK)
//    {
        ADC_ntc2  = ((unsigned char)anonym->rxbuf[0x07] << 8) | (unsigned char)anonym->rxbuf[0x06];
        x = (4095.0 - ADC_ntc2)/ADC_ntc2;  // (Vcc-Vout)/Vout
        R = Rs / x;
        if (R > 0)  T = 1/(1/T25 + 1/b * log(R/R25));
        //if (R > 0)  T =  1/( p0 + p1*log(R) + p2*log(R)*log(R) + p3*log(R)*log(R)*log(R) );
//    }
    return T - 273.15;
}


static void decodeframe20(uint32_t m)
{
   uint32_t week;
   uint32_t tow;
   uint32_t cs;
   uint32_t i;
   int32_t ci;
   double dir;
   double v;
   double vv;
   double vn;
   double ve;
   double alt;
   double lon;
   double lat;
   float vbat;
   float temp1,temp2;
   uint32_t time0;
   uint32_t id;
   int k;
   char ids[201];
   //char s[201+6];
   char s[400];
   struct M20 * anonym;
   struct CHAN * anonym0; /* call if set */
   { /* with */
      struct M20 * anonym = &chan[m].m20;
      cs = (uint32_t)crcm10(68L, anonym->rxbuf, 70ul);
//	printf("[0]:%02x:%02x CS:%04x\n",(unsigned char)anonym->rxbuf[68],(unsigned char)anonym->rxbuf[69],cs);
      

      if (((unsigned char)anonym->rxbuf[0]==0x45 && (unsigned char)anonym->rxbuf[1]==0x20) && cs==m10card(anonym->rxbuf, 70ul, 68L, 2L)) {
         /* crc ok */
	    printf("\n");
	    for(k=0;k<70;k++)
		printf("%02x",(unsigned char)anonym->rxbuf[k]);
	    printf("\n");

	
	     tow=0;
	     tow=(unsigned char)anonym->rxbuf[15];
	     tow<<=8;
	     tow|=(unsigned char)anonym->rxbuf[16];
	     tow<<=8;
	     tow|=(unsigned char)anonym->rxbuf[17];

	     unsigned char w[4];
	     week=0;
	     

	     week=  (((unsigned char)anonym->rxbuf[27]>>4)&0x0f)*1000 ;
	     week+= ((unsigned char)anonym->rxbuf[27])&0x0f;
	     week+= (((unsigned char)anonym->rxbuf[26]>>4)&0x0f)*100 ;
	     week+= (((unsigned char)anonym->rxbuf[26])&0x0f)*10;
/*
    	     time0 = tow/1000UL+week*604800UL+315964800UL;
    	     if (verb2) {
        	osi_WrStr(" ", 2ul);
        	aprsstr_DateToStr(time0, s, 201ul);
        	osi_WrStr(s, 201ul);
        	osi_WrStr(" ", 2ul);
             } 
*/
	    time0 = tow+week*604800000UL+315964800UL;
             if (verb2) {
                osi_WrStr(" ", 2ul);
                aprsstr_DateToStr(time0/1000, s, 201ul);
                osi_WrStr(s, 201ul);
                osi_WrStr(" ", 2ul);
             }


	     vbat=(float)3.3-(255-(unsigned char)anonym->rxbuf[38])*0.012654321;
             lat = (double)m10card(anonym->rxbuf, 70ul, 28L,4L)*0.000001;
             lon = (double)m10card(anonym->rxbuf, 70ul, 32L,4L)*0.000001;
             alt = (double)m10card(anonym->rxbuf, 70ul, 8L, 3L)*0.01;

             //ci = (int32_t)m10card(anonym->rxbuf, 70, 0x0b, 2L);

	     ci=0xff&anonym->rxbuf[0x0b]; ci<<=8;  ci|= 0xff& anonym->rxbuf[0x0c];
//	     printf("TEST: %04x\n",ci);
             if (ci>32767L) ci -= 65536L;
             ve = (double)ci/ 1e2;
             //ci = (int32_t)m10card(anonym->rxbuf, 70, 0x0d, 2L);
//             ci=(int32_t)anonym->rxbuf[0x0e]<<8 | anonym->rxbuf[0x0d];
	     ci=0xff&anonym->rxbuf[0x0d]; ci<<=8;  ci|=0xff& anonym->rxbuf[0x0e];

	     if (ci>32767L) ci -= 65536L;
             vn = (double)ci/ 1e2;
             //ci = (int32_t)m10card(anonym->rxbuf, 70, 0x18, 2L);
//	     ci=(int32_t)anonym->rxbuf[0x19]<<8 | anonym->rxbuf[0x18];
	     ci=0xff&anonym->rxbuf[0x18]; ci<<=8;  ci|=0xff& anonym->rxbuf[0x19];
//	     printf("TEST: %04x\n\n",ci);
             if (ci>32767L) ci -= 65536L;
             vv = (double)ci/ 1e2;
             v = (double)osic_sqrt((float)(ve*ve+vn*vn));
                // hor speed
             dir = atang2(vn, ve)*5.7295779513082E+1;
             if (dir<0.0) dir = 360.0+dir;


/*	    int i,j,k,l;
	    unsigned int byte;
	    unsigned char sn_bytes[5];
	    char SN[12];

	    for (i = 0; i < 11; i++) SN[i] = ' '; SN[11] = '\0';

	    for (i = 0; i < 5; i++) {
    		byte = anonym->rxbuf[46 + i];
    		sn_bytes[i] = byte;
	    }
*/
	    unsigned long bSN;
	    unsigned long sn1,sn2,sn3;
	    bSN=(anonym->rxbuf[pos_SN20]<<16) & 0xff0000 | (anonym->rxbuf[pos_SN20+1]<<8) &0xff00 | (anonym->rxbuf[pos_SN20+2]) & 0xff;
	    sn1 = (bSN>>17 & 0x0f)*100 + ((bSN>>22 &0x3) | (bSN>>19 &0x04));
	    sn3 = ((bSN >>10)&0x3f) |  (bSN & 0x1f)<<6;
	    sprintf(ids,"%03u2%05u\n",sn1,sn3);
	    ids[9U] = 0;
    
//	    temp1=M20get_Temp(m);
            temp2=M20get_Tntc2(m);
            if (verb) {
        	//WrChan((int32_t)m);
		printCnDT(m);
        	osi_WrStr("M20 ", 5ul);
        	osi_WrStr(ids, 201ul);
        	osi_WrStr(" ", 2ul);
        	osic_WrFixed((float)lat, 5L, 1UL);
        	osi_WrStr(" ", 2ul);
        	osic_WrFixed((float)lon, 5L, 1UL);
        	osi_WrStr(" ", 2ul);
        	osic_WrFixed((float)alt, 1L, 1UL);
        	osi_WrStr("m ", 3ul);
        	osic_WrFixed((float)(v*3.6), 1L, 1UL);
        	osi_WrStr("km/h ", 6ul);
        	osic_WrFixed((float)dir, 0L, 1UL);
        	osi_WrStr("deg ", 5ul);
        	osic_WrFixed((float)vv, 1L, 1UL);
        	osi_WrStr("m/s ", 5ul);
		osic_WrFixed((float)vbat, 1L, 1UL);
        	osi_WrStr("V T1:", 6ul);
		osic_WrFixed((float)temp1, 1L, 1UL);
        	osi_WrStr("C T2:", 6ul);
		osic_WrFixed((float)temp2, 1L, 1UL);
		osi_WrStr("C ", 3ul);
            }
	    struct CHAN * anonym0 = &chan[m];
    	    s[0U] = (char)(anonym0->mycallc/16777216UL);
    	    s[1U] = (char)(anonym0->mycallc/65536UL&255UL);
    	    s[2U] = (char)(anonym0->mycallc/256UL&255UL);
    	    s[3U] = (char)(anonym0->mycallc&255UL);
    	    if (anonym0->mycallc>0UL) s[4U] = anonym0->myssid;
    	    else s[4U] = '\020';
	    s[5]=',';
	    for(i=0;i<6;i++)			//qrg
		s[i+6]=chan[m].freq[i];
	    s[12]=',';
	    for(i=0;i<9;i++)			//nazwa
		s[i+13]=ids[i];
	    s[22]=0;

	    if( lat>-90.0 && lat<90.0 && lon>=-180.0 && lon<=180.0 && alt>0.0 && alt<45000.0){
		sprintf(s,"%s,%012lu,%09.5f,%010.5f,%05.0f,%03.0f,%05.1f,%05.1f,%05.2f,%06.1f,%06.1f\n",s,time0,lat,lon,alt,dir,v,vv,vbat,temp1,temp2);
		alludp(chan[m].udptx, 100, s, 100);
	    }
      }
      else if (verb2) {
         /*build tx frame */
         WrChan((int32_t)m);
         osi_WrStr("M20 crc error", 14ul);
      }
      if (verb2) {
         WrdB(chan[m].adcmax-chan[m].adcmin);
         WrQuali(noiselevel1(anonym->bitlev, anonym->noise));
         Wrtune(chan[m].adcmax+chan[m].adcmin, chan[m].adcmax-chan[m].adcmin);
      }
      if (verb2) {
         for (i = 0UL; i<=23UL; i++) {
            if (i%10UL==0UL) osi_WrStrLn("", 1ul);
            osic_WrINT32(m10card(anonym->rxbuf, 101ul,
                (int32_t)(48UL+i*2UL), 2L), 6UL);
            osi_WrStr(" ", 2ul);
         } /* end for */
         for (i = 0UL; i<=100UL; i++) {
            if (i%24UL==0UL) osi_WrStrLn("", 1ul);
            if (X2C_INL(i,256,_cnst1)) osi_WrStr(" . ", 4ul);
            else osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[i], 3UL);
         } /* end for */
      }
      if (verb2) osi_WrStrLn("", 1ul);
   }
} /* end decodeframe20() */



static void demodbyte20(uint32_t m, char d)
{
   struct M20 * anonym;
   { /* with */
      struct M20 * anonym = &chan[m].m20;
      /*WrInt(ORD(d),1); Flush(); */
      anonym->synword = anonym->synword*2UL+(uint32_t)d;
      if (anonym->rxp>=70UL) {
         if ((anonym->synword&0xffff)==0x4520) {
            anonym->rxp = 2UL;
            anonym->rxb = 0UL;
            anonym->rxbuf[0U] = 0x45;
            anonym->rxbuf[1U] = 0x20;
         }
      }
      else {
         /*WrStr(" -syn- "); */
         ++anonym->rxb;
         if (anonym->rxb>=8UL) {
            anonym->rxbuf[anonym->rxp] = (char)(anonym->synword&0xff);
            anonym->rxb = 0UL;
            ++anonym->rxp;
            if (anonym->rxp==70UL) decodeframe20(m);
         }
      }
   }
} /* end demodbyte10() */


static void demodbit20(uint32_t m, float u, float u0)
{
   char bit;
   char d;
   float ua;
   struct M20 * anonym;
   /*IF manchestd>20000 THEN  */
   /*WrInt(VAL(INTEGER, u), 8); Flush; */
   d = u>=0.0f;
   { /* with */
      struct M20 * anonym = &chan[m].m20;
      /*WrInt(manchestd DIV 256, 1); WrStr("("); WrInt(ORD(lastmanch),1);
                WrInt(ORD(d),1);WrStr(")");  Flush(); */
      /*WrInt(ORD(lastmanch<>d),1); Flush(); */
      /*END; */
      if (anonym->lastmanch==d) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      bit = d!=anonym->lastmanch;
      if (anonym->manchestd>0L) {
         demodbyte20(m, bit);
         /*quality*/
         ua = (float)fabs(u)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.02f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.05f;
      }
      /*quality*/
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
   }
} /* end demodbit10() */


static void demod20(float u, uint32_t m)
{
   char d;
   struct M20 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct M20 * anonym = &chan[m].m20;
      d = u>=0.0f;
      if (anonym->cbit) {
         demodbit20(m, u, anonym->lastu);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} 

/*---------------------- M20 */



//-----------------------------------------------------------------------------------------------------
//PILOTSONDE
//-----------------------------------------------------------------------------------------------------

#define EOF_INT  0x1000000
int sample_rate = 0, bits_sample = 0;
unsigned long sample_count = 0;
double bitgrenze = 0;
int wlen;
int *sample_buff = NULL;
int par=1, par_alt=1;

int read_signed_sample(FILE *fp) {  // int = i32_t
    int byte, i, sample, s=0;       // EOF -> 0x1000000

        byte = fgetc(fp);
        if (byte == EOF) return EOF_INT;
        if (i == 0) sample = byte;

        if (bits_sample == 16) {
            byte = fgetc(fp);
            if (byte == EOF) return EOF_INT;
            if (i == 0) sample +=  byte << 8;
        }


    if (bits_sample ==  8)  s = sample-128;   // 8bit: 00..FF, centerpoint 0x80=128
    if (bits_sample == 16)  s = (short)sample;

    sample_count++;

    return s;
}

int read_filter_sample(FILE *fp) {
    int i;                          // wenn sample_buff[] ein 8N1-byte umfasst,
    int s0, s, y;                   // mit (max+min)/2 Mittelwert bestimmen;
    static int min, max;            // Glaettung durch lowpass/moving average empfohlen

    s = read_signed_sample(fp);
    if (s == EOF_INT) return EOF_INT;

    sample_count--;

    s0 = sample_buff[sample_count % wlen];
    sample_buff[sample_count % wlen] = s;

    y = 0;
    if (sample_count >  wlen-1) {

        if (s < min)  min = s;
        else {
            if (s0 <= min) {
                min = sample_buff[0];
                for (i = 1; i < wlen; i++) {
                    if (sample_buff[i] < min)  min = sample_buff[i];
                }
            }
        }

        if (s > max)  max = s;
        else {
            if (s0 >= max) {
                max = sample_buff[0];
                for (i = 1; i < wlen; i++) {
                    if (sample_buff[i] > max)  max = sample_buff[i];
                }
            }
        }

        y = sample_buff[(sample_count+wlen-1)%wlen] - (min+max)/2;

    }
    else if (sample_count == wlen-1) {
        min = sample_buff[0];
        max = sample_buff[0];
        for (i = 1; i < wlen; i++) {
            if (sample_buff[i] < min)  min = sample_buff[i];
            if (sample_buff[i] > max)  max = sample_buff[i];
        }
        y = sample_buff[(sample_count+wlen-1)%wlen] - (min+max)/2;
    }

    sample_count++;

    return y;
}

int read_bits_fsk(FILE *fp, int *bit, int *len,float samples_per_bit,int option_dc,int option_res,int option_inv) {
    static int sample;
    int n, y0;
    float l, x1;
    static float x0;

    n = 0;
    do{
        y0 = sample;

        if (option_dc) sample = read_filter_sample(fp);
        else           sample = read_signed_sample(fp);

        if (sample == EOF_INT) return EOF;
        //sample_count++;
        par_alt = par;
        par =  (sample >= 0) ? 1 : -1;    // 8bit: 0..127,128..255 (-128..-1,0..127)
        n++;
    } while (par*par_alt > 0);

    if (!option_res) l = (float)n / samples_per_bit;
    else {                                 // genauere Bitlaengen-Messung
        x1 = sample/(float)(sample-y0);    // hilft bei niedriger sample rate
        l = (n+x0-x1) / samples_per_bit;   // meist mehr frames (nicht immer)
        x0 = x1;
    }

    *len = (int)(l+0.5);

    if (!option_inv) *bit = (1+par_alt)/2;  // oben 1, unten -1
    else             *bit = (1-par_alt)/2;  // sdr#<rev1381?, invers: unten 1, oben -1
// *bit = (1+inv*par_alt)/2; // ausser inv=0

    /* Y-offset ? */

    return 0;
}

/*
static void decPIL(){

    pos = FRAMESTART;

    while (!read_bits_fsk(fp, &bit, &len)) {

        if (len == 0) { // reset_frame();
            if (pos > (pos_GPSdate+7)*BITS) {
                for (i = pos; i < BITFRAME_LEN; i++) frame_bits[i] = 0x30 + 0;
                print_frame(pos);//byte_count
                header_found = 0;
                pos = FRAMESTART;
            }
            //inc_bufpos();
            //buf[bufpos] = 'x';
            continue;   // ...
        }

        for (i = 0; i < len; i++) {

            inc_bufpos();
            buf[bufpos] = 0x30 + bit;  // Ascii

            if (!header_found) {
                header_found = compare2();
                //if (header_found) fprintf(stdout, "[%c] ", header_found>0?'+':'-');
                if (header_found < 0) option_inv ^= 0x1;
                // printf("[%c] ", option_inv?'-':'+');
            }
            else {
                frame_bits[pos] = 0x30 + bit;  // Ascii
                pos++;

                if (pos == BITFRAME_LEN) {
                    print_frame(pos);//FRAME_LEN
                    header_found = 0;
                    pos = FRAMESTART;
                }
            }

        }
        if (header_found && option_b==1) {
            bitstart = 1;

            while ( pos < BITFRAME_LEN ) {
                if (read_rawbit(fp, &bit) == EOF) break;
                frame_bits[pos] = 0x30 + bit;
                pos++;
            }
            frame_bits[pos] = '\0';
            print_frame(pos);//FRAME_LEN

            header_found = 0;
            pos = FRAMESTART;
        }
    }

}
*/

static void sendpils(uint32_t m)
{
   uint32_t i;
   struct CHAN * anonym;
        
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->mycallc>0UL) {
         chan[m].pils.rxbuf[0U] = (char)(anonym->mycallc/16777216UL);
         chan[m].pils.rxbuf[1U] = (char)(anonym->mycallc/65536UL&255UL);
         chan[m].pils.rxbuf[2U] = (char)(anonym->mycallc/256UL&255UL);
         chan[m].pils.rxbuf[3U] = (char)(anonym->mycallc&255UL);
         chan[m].pils.rxbuf[4U] = anonym->myssid;
         chan[m].pils.rxbuf[5U] = 0;
         chan[m].pils.rxbuf[6U] = 0;

	 chan[m].pils.rxbuf[55U] = chan[m].freq[0];
	 chan[m].pils.rxbuf[56U] = chan[m].freq[1];
	 chan[m].pils.rxbuf[57U] = chan[m].freq[2];
	 chan[m].pils.rxbuf[58U] = chan[m].freq[3];
	 chan[m].pils.rxbuf[59U] = chan[m].freq[4];
	 chan[m].pils.rxbuf[60U] = chan[m].freq[5];
      }
      alludp(chan[m].udptx, 55UL+6UL, chan[m].pils.rxbuf, 66ul);
      ///osi_WrStrLn("sending UDP data...",19UL);
   }
} /* end sendpils() */

static int32_t getint32r(const char frame[], uint32_t frame_len, uint32_t p)
{
   uint32_t n;
   uint32_t i;
   n = 0UL;
   for (i = 0UL;; i++) {
      n = n*256UL+(uint32_t)(uint8_t)frame[p+i];
      if (i==3UL) break;
   } /* end for */
   return (int32_t)n;
} /* end getint32() */


static int32_t getint16r(const char frame[], uint32_t frame_len, uint32_t p)
{
   uint32_t n;
   n = (uint32_t)(uint8_t)frame[p+1UL]+256UL*(uint32_t)(uint8_t)
                frame[p];
   if (n>=32768UL) return (int32_t)(n-65536UL);
   return (int32_t)n;
} /* end getint16() */


/* -------------------------------------------------------------------------- */
//taken from https://github.com/rs1729/RS/blob/master/m10/pilotsonde/m12.c

int crc16poly = 0xA001;

unsigned int crc16rev(unsigned char bytes[], int len) {
    unsigned int rem = 0xFFFF; // init value: crc(0xAAAAAA;init:0xFFFF)=0x3FAF
    int i, j;
    for (i = 0; i < len; i++) {
        rem = rem ^ bytes[i];
        for (j = 0; j < 8; j++) {
            if (rem & 0x0001) {
                rem = (rem >> 1) ^ crc16poly;
            }
            else {
                rem = (rem >> 1);
            }
            rem &= 0xFFFF;
        }
    }
    return rem;
}

/* -------------------------------------------------------------------------- */


static void demodbytepilot(uint32_t m, char d)
{
 // -- - Modyfikacja na Pilot sonde ==
   uint32_t j;
   uint32_t i;
   uint32_t revc;
   uint32_t normc;
   uint32_t cz_1;
   unsigned int crc = 0x0000;
   
   uint16_t  offs;
   double   lat;
   double   long0;
   double   heig;
   int	    ok;

  
   struct PILS * anonym;
   
   { /* with */
   
   //Look for pilot sonde header 
   
      struct PILS * anonym = &chan[m].pils;
      if (anonym->rxp==0UL) {
         anonym->synbuf[anonym->synp] = d;
         i = anonym->synp;
         ++anonym->synp;
         if (anonym->synp>39UL) anonym->synp = 0UL; 
         j = 40UL;  //30 bytes in header (3*10)
         normc = 0UL;
         revc = 0UL;
         do {
            --j;
            if (("0010101011001010101100101010110100000001"[j]=='1')==anonym->synbuf[i]) ++normc;
            else ++revc;
            if (i==0UL) i = 39UL;
            else --i;
         } while (!(j==0UL || normc>4UL && revc>4UL));
         anonym->headok = normc==0UL || revc==0UL;
         anonym->rev = normc<revc;
         if (j==0UL) {
	        anonym->rxbuf[0UL]=0xAA;            //fill in first 3 bytes )as we lost it for header
		anonym->rxbuf[1UL]=0xAA;            //recovery of first bytes for crc calcs
		anonym->rxbuf[2UL]=0xAA;
		anonym->rxbuf[3UL]=0x01;
	        anonym->rxp = 4UL;
	 }	
         anonym->rxbitc = 0UL;
      }
      else { 
	//ignore first and last bytes (stop and start) and save remaining bites in reverse order to byte. 
         if ((anonym->rxbitc > 0UL) && (anonym->rxbitc < 9UL)) { 
	      anonym->rxbyte = (anonym->rxbyte>>1UL)+128UL*(uint32_t)(d!=anonym->rev);
	 }
	 ++anonym->rxbitc;                            //increase bit number
	 
         if (anonym->rxbitc>=10UL) {                       // when 10 bits (1+8+1) received
            anonym->rxbuf[anonym->rxp] = (anonym->rxbyte); //save received byte to buffer
            ++anonym->rxp;                                 // increase byte number 
            if (anonym->rxp>=50UL) {                        // if full sentence received
		 crc = (anonym->rxbuf[48UL]<<8UL) | anonym->rxbuf[49UL];
		 if (verb) {
/*		    if (maxchannels>0UL) {
			printf("%02i:",m+1);
                    }
*/
		    printCnDT(m);
		    osi_WrStr("PS ",4UL);
		 }	 

		if (verb) {                   //information on signal quality
		     WrdB(chan[m].adcmax);
                     WrQ(anonym->bitlev0, anonym->noise0);
		     Wrtune(chan[m].adcdc, chan[m].adcmax);
//		     osi_WrStrLn("",0UL);
		}	 
	        anonym->rxp = 0UL;

		
		offs=6UL; //offset - for first position
		lat=(double)getint32r(anonym->rxbuf,55UL,offs)*0.000001;
		long0=(double)getint32r(anonym->rxbuf,55UL,offs+4UL)*0.000001;
                heig=(double)getint32r(anonym->rxbuf,55UL,offs+8UL)*.01; 
                cnt++;



		if (crc == crc16rev(anonym->rxbuf, 48UL) && lat>0 && lat<89.9 && long0>0 && long0<179.9 && heig>1 && heig<35000) {
		    anonym->poklat=lat;
		    anonym->poklon=long0;
		    anonym->pokalt=heig;
		    anonym->lastfr=time(NULL);
  		    //if (verb) osi_WrStr(" CRC [OK+] ",12UL);
            	    if  (verb) {             //if more verbous print lat/long/h
			pok++;
			osi_WrStrLn("",0UL);
			osi_WrStr(" Lat=",5ul);
			osic_WrFixed((float)(lat), 6L, 1UL);
			osi_WrStr(" Long=", 6ul);
			osic_WrFixed((float)(long0), 6L, 1UL);
			osi_WrStr(" height=", 9ul);
			osic_WrFixed((float)heig, 1L, 1UL);
			osi_WrStrLn("m ", 3ul);
			printf("BR: %li:%li\r\n",anonym->configbaud,anonym->baudfine);
		    }
                    for (cz_1 = 49UL; cz_1>1UL; cz_1--) {anonym->rxbuf[cz_1+5UL] = anonym->rxbuf[cz_1];}   //move cells by 5 up
		    sendpils(m);    
		}   //crc ok
		else {
	    	    if (verb) osi_WrStrLn(" parity error",14UL);
		    //if position not too far from last good one....

		    ok=0;
		    if(time(NULL)-anonym->lastfr<=60){
			if ((fabs(anonym->poklat-lat)<0.003f) && (fabs(anonym->poklon-long0)<0.003f) && (abs(anonym->pokalt-heig)<1500)) 
			    ok=1;
		    }else if ((time(NULL)-anonym->lastfr>60) && (time(NULL)-anonym->lastfr<=1800)){
			if ((fabs(anonym->poklat-lat)<0.03f) && (fabs(anonym->poklon-long0)<0.03f) && (abs(anonym->pokalt-heig)<4500)) 
			    ok=1;
		    }
    
		    if (ok || (anonym->poklat==-1 && anonym->poklon==-1 && lat>0 && lat<89.9 && long0>0 && long0<179.9 && heig>1 && heig<35000)) {

			anonym->poklat=(lat+anonym->poklat)/2;
			anonym->poklon=(long0+anonym->poklon)/2;
			anonym->pokalt=(heig+anonym->pokalt)/2;
			anonym->lastfr=time(NULL);

			if (verb) {
			    nok++;
		    	    osi_WrStr(" !Lat=",6ul);
		    	    osic_WrFixed((float)(lat), 6L, 1UL);
		    	    osi_WrStr(" !Long=", 7ul);
		    	    osic_WrFixed((float)(long0), 6L, 1UL);
		    	    osi_WrStr(" !height=", 10ul);
		    	    osic_WrFixed((float)heig, 1L, 1UL);
		    	    osi_WrStr("m ", 3ul);
			    printf("***BR: %li:%li\r\n",anonym->configbaud,anonym->baudfine);
		        }

			for (cz_1 = 49UL; cz_1>1UL; cz_1--) {anonym->rxbuf[cz_1+5UL] = anonym->rxbuf[cz_1];}   //move cells by 5 up
			sendpils(m); //send data to sondemod
		   }
		}
//		printf("OK: %u, NOK:%u, CNT:%u  CAL:%u\r\n",pok,nok,cnt,(unsigned int)(100*pok/nok));
            }
	    
            if (anonym->rxp==48UL) {              
               anonym->bitlev0 = anonym->bitlev;
                /* save quality before end of shortst frame */
               anonym->noise0 = anonym->noise;
            }
            anonym->rxbitc = 0UL;
         }
      }
   }
} /* end demodbytepilot() */

static void demodbitpilot(uint32_t m, float u)
{
   char d;
   float ua;
   struct PILS * anonym;
   d = u>=0.0f;
   { /* with */
      struct PILS * anonym = &chan[m].pils;
      demodbytepilot(m, d);
      /*quality*/
      ua = (float)fabs(u)-anonym->bitlev;
      anonym->bitlev = anonym->bitlev+ua*0.005f;
      anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
   }
/*quality*/
} /* end demodbitpilot() */


//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//-----------------------------------------------------------------------------------------------------
//PILOTSONDE
//
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


static void posrs41(const char b[], uint32_t b_len, uint32_t p)
{
   double dir;
   double vu;
   double ve;
   double vn;
   double vz;
   double vy;
   double vx;
   double heig;
   double long0;
   double lat;
   double z;
   double y;
   double x;
   x = (double)getint32(b, b_len, p)*0.01;
   y = (double)getint32(b, b_len, p+4UL)*0.01;
   z = (double)getint32(b, b_len, p+8UL)*0.01;
   wgs84r(x, y, z, &lat, &long0, &heig);
   osi_WrStr(" ", 2ul);
   osic_WrFixed((float)(X2C_DIVL(lat,1.7453292519943E-2)), 5L, 1UL);
   osi_WrStr(" ", 2ul);
   osic_WrFixed((float)(X2C_DIVL(long0,1.7453292519943E-2)), 5L, 1UL);
   osi_WrStr(" ", 2ul);
   osic_WrFixed((float)heig, 0L, 1UL);
   osi_WrStr("m ", 3ul);
   /*speed */
   vx = (double)getint16(b, b_len, p+12UL)*0.01;
   vy = (double)getint16(b, b_len, p+14UL)*0.01;
   vz = (double)getint16(b, b_len, p+16UL)*0.01;
   vn = (-(vx*(double)osic_sin((float)lat)*(double)
                osic_cos((float)long0))-vy*(double)
                osic_sin((float)lat)*(double)osic_sin((float)
                long0))+vz*(double)osic_cos((float)lat);
   ve = -(vx*(double)osic_sin((float)long0))+vy*(double)
                osic_cos((float)long0);
   vu = vx*(double)osic_cos((float)lat)*(double)
                osic_cos((float)long0)+vy*(double)
                osic_cos((float)lat)*(double)osic_sin((float)
                long0)+vz*(double)osic_sin((float)lat);
   dir = X2C_DIVL(atang2(vn, ve),1.7453292519943E-2);
   if (dir<0.0) dir = 360.0+dir;
   osi_WrStr(" ", 2ul);
   osic_WrFixed(osic_sqrt((float)(vn*vn+ve*ve))*3.6f, 1L, 1UL);
   osi_WrStr("km/h ", 6ul);
   osic_WrFixed((float)dir, 0L, 1UL);
   osi_WrStr("deg ", 5ul);
   osic_WrFixed((float)vu, 1L, 1UL);
   osi_WrStr("m/s", 4ul);
} /* end posrs41() */


static int32_t reedsolomon41(char buf[], uint32_t buf_len,
                uint32_t len2)
{
   uint32_t i;
   int32_t res1;
   /*tb1, */
   int32_t res;
   char b1[256];
   char b[256];
   uint32_t eraspos[24];
   uint32_t tmp;
   for (i = 0UL; i<=255UL; i++) {
      b[i] = 0;
      b1[i] = 0;
   } /* end for */
   tmp = len2;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      b[230UL-i] = buf[i*2UL+56UL];
      b1[230UL-i] = buf[i*2UL+57UL];
      if (i==tmp) break;
   } /* end for */
   for (i = 0UL; i<=23UL; i++) {
      b[254UL-i] = buf[i+8UL];
      b1[254UL-i] = buf[i+32UL];
   } /* end for */
   /*tb1:=b; */
   res = decodersc(b, eraspos, 0L);
   /*FOR i:=0 TO HIGH(b) DO */
   /*IF tb1[i]<>b[i] THEN WrHex(ORD(tb1[i]),4);WrHex(ORD(b[i]),4);
                WrInt(i, 4); WrStr("=pos "); END; */
   /*END; */
   /*tb1:=b1; */
   res1 = decodersc(b1, eraspos, 0L);
   /*FOR i:=0 TO HIGH(b) DO */
   /*IF tb1[i]<>b1[i] THEN WrHex(ORD(tb1[i]),4);WrHex(ORD(b1[i]),4);
                WrInt(i, 4); WrStr("=pos1 "); END; */
   /*END; */
   if (res>0L && res<=12L) {
      tmp = len2;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         buf[i*2UL+56UL] = b[230UL-i];
         if (i==tmp) break;
      } /* end for */
      for (i = 0UL; i<=23UL; i++) {
         buf[i+8UL] = b[254UL-i];
      } /* end for */
   }
   if (res1>0L && res1<=12L) {
      tmp = len2;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         buf[i*2UL+57UL] = b1[230UL-i];
         if (i==tmp) break;
      } /* end for */
      for (i = 0UL; i<=23UL; i++) {
         buf[i+32UL] = b1[254UL-i];
      } /* end for */
   }
   if (res<0L || res1<0L) return -1L;
   else return res+res1;
   return 0;
} /* end reedsolomon41() */

static uint16_t sondeudp_POLYNOM0 = 0x1021U;


static void decode41(uint32_t m)
{
   uint32_t try0;
   uint32_t posok;
   uint32_t nameok;
   uint32_t len;
   uint32_t p;
   uint32_t i;
   char ch;
   char typ;
   char aux;
   char allok;
   int32_t repl;
   int32_t corr;
   struct R41 * anonym;
   uint32_t tmp;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      try0 = 0UL;
      do {
         allok = 1;
         nameok = 0UL;
         posok = 0UL;
         corr = 0L;
         repl = 0L;
         if (try0>0UL) {
            if (try0>1UL) {
               for (i = 0UL; i<=519UL; i++) {
                  if (anonym->fixcnt[i]>=10U) {
                     /* replace stable bytes */
                     anonym->rxbuf[i] = anonym->fixbytes[i];
                     ++repl;
                  }
               } /* end for */
            }
            corr = reedsolomon41(anonym->rxbuf, 520ul, 131UL);
                /* try short frame */
            if (corr<0L) {
               corr = reedsolomon41(anonym->rxbuf, 520ul, 230UL);
                /* may bo long frame */
            }
         }
         p = 57UL;
         aux = 0;
         for (;;) {
            if (p+4UL>=519UL) break;
            typ = anonym->rxbuf[p];
            ++p;
            len = (uint32_t)(uint8_t)anonym->rxbuf[p]+2UL;
            ++p;
            if (p+len>=519UL) break;
            /*
            WrStrLn("");
            FOR i:=0 TO len+1 DO WrHex(ORD(rxbuf[p+i-2]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;
            WrStrLn("");
            */
            if (!crcrs(anonym->rxbuf, 520ul, (int32_t)p,(int32_t)(p+len))) {
               /* crc error */
               allok = 0;
               break;
            }
            if (verb2) {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("", 1ul);
               osi_WrStr("start ID length data... crc [", 30ul);
               osi_WrHex((p-2UL)/256UL, 0UL);
               osi_WrHex(p-2UL, 3UL);
               osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[p-2UL], 3UL);
               osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[p-1UL], 0UL);
               osi_WrStrLn("]", 2ul);
               tmp = len-1UL;
               i = 0UL;
               if (i<=tmp) for (;; i++) {
                  osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[p+i], 3UL);
                  if ((i&15UL)==15UL) osi_WrStrLn("", 1ul);
                  if (i==tmp) break;
               } /* end for */
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("", 1ul);
            }
            tmp = (p+len)-1UL;
            i = p-2UL;
            if (i<=tmp) for (;; i++) {
               /* update fixbyte statistics */
               if (anonym->fixbytes[i]==anonym->rxbuf[i]) {
                  if (anonym->fixcnt[i]<255U) ++anonym->fixcnt[i];
               }
               else {
                  anonym->fixbytes[i] = anonym->rxbuf[i];
                  anonym->fixcnt[i] = 0U;
               }
               if (i==tmp) break;
            } /* end for */
            if (typ=='y') nameok = p;
            else if (typ=='{') {
               /*        ELSIF typ=CHR(7AH) THEN */
               /*             WrStrLn("7A frame"); */
               /*        ELSIF typ=CHR(7CH) THEN */
               /*             WrStrLn("7C frame"); */
               /*WrInt(getint32(rxbuf, p+2) DIV 1000 MOD 86400 , 10);
                WrStr("=gpstime "); */
               /*        ELSIF typ=CHR(7DH) THEN */
               /*             WrStrLn("7D frame"); */
               posok = p;
            }
            else if (typ=='~') aux = 1;
            /*        ELSIF typ=CHR(76H) THEN */
            /*             WrStrLn("76 frame"); */
            /*        ELSE EXIT END; */
            /*        WrInt(getint16(rxbuf, 3BH), 0); */
            /*        WrStr(" ");WrHex(ORD(typ), 0);WrStr(" ");
                WrHex(p DIV 256, 0);WrHex(p, 0); */
            /*        WrStr(" "); */
            if (typ=='v') break;
            p += len;
         }
         ++try0;
      } while (!(allok || try0>2UL));
      if (verb && nameok>0UL) {
         if (maxchannels>0UL) {
	      printCnDT(m);
         }
         osi_WrStr("R41 ", 5ul);
         for (i = 0UL; i<=7UL; i++) {
            ch = anonym->rxbuf[nameok+2UL+i];
            if ((uint8_t)ch>' ' && (uint8_t)ch<'\177') {
               osi_WrStr((char *) &ch, 1u/1u);
            }
         } /* end for */
         osi_WrStr(" ", 2ul);
         osic_WrINT32((uint32_t)getint16(anonym->rxbuf, 520ul, nameok),
                1UL);
         if (posok>0UL) posrs41(anonym->rxbuf, 520ul, posok);
         if (anonym->rxbuf[nameok+23UL]==0) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)(getcard16(anonym->rxbuf, 520ul,
                nameok+26UL)/64UL+40000UL)*0.01f, 2L, 1UL);
            osi_WrStr("MHz", 4ul);
         }
         if (aux) osi_WrStr(" +Aux", 6ul);
         if (!((allok || posok>0UL) || aux)) {
            osi_WrStr(" ----  crc err ", 16ul);
         }
         WrdB(chan[m].adcmax);
         WrQ(anonym->bitlev0, anonym->noise0);
         /*WrStrLn(""); */
         /*FOR i:=0 TO HIGH(rxbuf) DO WrHex(ORD(rxbuf[i]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;  */
         if (repl>0L) {
            osi_WrStr(" x", 3ul);
            osic_WrINT32((uint32_t)repl, 1UL);
         }
         if (corr<0L) osi_WrStr(" -R", 4ul);
         else if (corr>0L) {
            osi_WrStr(" +", 3ul);
            osic_WrINT32((uint32_t)corr, 1UL);
            osi_WrStr("R", 2ul);
         }
         Wrtune(chan[m].adcdc, chan[m].adcmax);
         osi_WrStrLn("", 1ul);
	// printf("BR: %li:%li\r\n",anonym->configbaud,anonym->baudfine);
      }
   }
  // printf("NAMEOK:%i\n",nameok);
   if (allok)
     sendrs41(m);
} /* end decode41() */

#define sondeudp_MAXHEADERR 4

static uint8_t _cnst[64] = {150U,131U,62U,81U,177U,73U,8U,152U,50U,5U,89U,
                14U,249U,68U,198U,38U,33U,96U,194U,234U,121U,93U,109U,161U,
                84U,105U,71U,12U,220U,232U,92U,241U,247U,118U,130U,127U,7U,
                153U,162U,44U,147U,124U,48U,99U,245U,16U,46U,97U,208U,188U,
                180U,182U,6U,170U,244U,35U,120U,110U,59U,174U,191U,123U,76U,
                193U};

static void demodbyte41(uint32_t m, char d)
{
   uint32_t j;
   uint32_t i;
   uint32_t revc;
   uint32_t normc;
   /*WrStr(CHR(ORD(d)+48)); */
   struct R41 * anonym;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      if (anonym->rxp==0UL) {
         anonym->synbuf[anonym->synp] = d;
         i = anonym->synp;
         ++anonym->synp;
         if (anonym->synp>63UL) anonym->synp = 0UL;
         j = 56UL;
         normc = 0UL;
         revc = 0UL;
         do {
            --j;
            if (("0000100001101101010100111000100001000100011010010100100000011111"[j]=='1')==anonym->synbuf[i]) ++normc;
            else ++revc;
            if (i==0UL) i = 63UL;
            else --i;
         } while (!(j==24UL || normc>4UL && revc>4UL));
         anonym->headok = normc==0UL || revc==0UL;
         anonym->rev = normc<revc;
         if (j==24UL) anonym->rxp = 7UL;
         anonym->rxbitc = 0UL;
      }
      else {
         anonym->rxbyte = anonym->rxbyte/2UL+128UL*(uint32_t)
                (d!=anonym->rev);
         ++anonym->rxbitc;
         if (anonym->rxbitc>=8UL) {
            anonym->rxbuf[anonym->rxp] = (char)((uint8_t)
                anonym->rxbyte^(uint8_t)_cnst[anonym->rxp&63UL]);
            ++anonym->rxp;
            if (anonym->rxp>=519UL) {
               decode41(m);
               anonym->rxp = 0UL;
            }
            if (anonym->rxp==200UL) {
               anonym->bitlev0 = anonym->bitlev;
                /* save quality before end of shortst frame */
               anonym->noise0 = anonym->noise;
            }
            anonym->rxbitc = 0UL;
         }
      }
   }
} /* end demodbyte41() */


static void demodbit41(uint32_t m, float u)
{
   char d;
   float ua;
   struct R41 * anonym;
   d = u>=0.0f;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      demodbyte41(m, d);
      /*quality*/
      ua = (float)fabs(u)-anonym->bitlev;
      anonym->bitlev = anonym->bitlev+ua*0.005f;
      anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
   }
/*quality*/
} /* end demodbit41() */



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

static void sendmp3(uint32_t m)
{
   uint32_t i;
   struct CHAN * anonym;
        
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->mycallc>0UL) {
         chan[m].mp3.rxbuf[103U] = (char)(anonym->mycallc/16777216UL);
         chan[m].mp3.rxbuf[104U] = (char)(anonym->mycallc/65536UL&255UL);
         chan[m].mp3.rxbuf[105U] = (char)(anonym->mycallc/256UL&255UL);
         chan[m].mp3.rxbuf[106U] = (char)(anonym->mycallc&255UL);
         chan[m].mp3.rxbuf[107U] = anonym->myssid;
         chan[m].mp3.rxbuf[108U] = 0;

	 chan[m].mp3.rxbuf[109U] = chan[m].freq[0];
	 chan[m].mp3.rxbuf[110U] = chan[m].freq[1];
	 chan[m].mp3.rxbuf[111U] = chan[m].freq[2];
	 chan[m].mp3.rxbuf[112U] = chan[m].freq[3];
	 chan[m].mp3.rxbuf[113U] = chan[m].freq[4];
	 chan[m].mp3.rxbuf[114U] = chan[m].freq[5];
      }
      alludp(chan[m].udptx, 115UL, chan[m].mp3.rxbuf, 115ul);
      ///osi_WrStrLn("sending UDP data...",19UL);
   }
} /* end sendpils() */



static void demodbytemp3(uint32_t m, char d)
{
   uint32_t j;
   uint32_t i;
   uint32_t revc;
   uint32_t normc;
   uint32_t cz_1;
   unsigned int crc = 0x0000;
   uint16_t  offs;
   double   lat;
   double   long0;
   double   heig;
   int	    ok;

   struct MP3 * anonym;
   { 
      struct MP3 * anonym = &chan[m].mp3;
      if (anonym->rxp==0UL) {
         anonym->synbuf[anonym->synp] = d;
         i = anonym->synp;
         ++anonym->synp;
         if (anonym->synp>47UL) anonym->synp = 0UL; 
         j = 48UL;  
         normc = 0UL;
         revc = 0UL;
         do {
            --j;
    	    if (("011001100110011001100110011001100110010101010101"[j]=='1')==anonym->synbuf[i]) ++normc;
            else ++revc;
            if (i==0UL) i = 47UL;
            else --i;
         } while (!(j==0UL || normc>1UL && revc>1UL));
         anonym->headok = normc==0UL || revc==0UL;
         anonym->rev = normc<revc;
         if (j==0UL) {
	        anonym->rxbuf[0UL]=0b10011001;   
		anonym->rxbuf[1UL]=0b10011001;   
		anonym->rxbuf[2UL]=0b10011001;
		anonym->rxbuf[3UL]=0b10011001;
		anonym->rxbuf[4UL]=0b10011010;
		anonym->rxbuf[5UL]=0b10101010;
	        anonym->rxp = 6UL;
	 }	
         anonym->rxbitc = 0UL;
      }
      else { 
	 uint8_t tmpD[155];
	 anonym->rxbyte = anonym->rxbyte/2UL+128UL*(uint32_t)
                (d!=anonym->rev);
	 ++anonym->rxbitc;                            
	 
         if (anonym->rxbitc>=8UL) {                       
            anonym->rxbuf[anonym->rxp] = (anonym->rxbyte);
            ++anonym->rxp;                                 
	    int ntsf=0;

            if (anonym->rxp>=102UL) {                     
		int s,q,p;
		uint16_t tmpb;
		p=0;
		    for(s=0;s<102;s+=2){
			tmpb=0;
			tmpb=(anonym->rxbuf[s]&0xff)<<8 | (anonym->rxbuf[s+1]&0xff);
			tmpD[p]=0;
			for(q=0;q<8;q++){
			    tmpD[p]<<=1;
			    if((tmpb & 0xC000) == 0x8000) {
				tmpD[p]|=1; 
			    }

			    switch(tmpb & 0xC000){
				case 0xC000: 
				case 0x0000: 
					anonym->rxp = 0UL; anonym->rxbitc = 0UL;
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
			sendmp3(m);
		    }
		    else
			printf("MP3 CRC ERR\n");
		    anonym->rxp = 0UL;
        	}
            if (anonym->rxp==50UL) {              
               anonym->bitlev0 = anonym->bitlev;
               anonym->noise0 = anonym->noise;
            }
            anonym->rxbitc = 0UL;
         }
      }
   }
} /* end demodbytemp3() */



static void demodbitmp3(uint32_t m, float u)
{
   char d;
   float ua;
   struct MP3 * anonym;
   d = u>=0.0f;
   { /* with */
      struct MP3 * anonym = &chan[m].mp3;
      demodbytemp3(m, d);
      /*quality*/
      ua = (float)fabs(u)-anonym->bitlev;
      anonym->bitlev = anonym->bitlev+ua*0.005f;
      anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
   }
/*quality*/
} /* end demodbitmp3() */








/*------------------------------ RS92 */
/*
PROCEDURE stobyte92(m:CARDINAL; b:CHAR);
VAR e:SET8;
BEGIN
  WITH chan[m].r92 DO
    rxbuf[rxp]:=b;
    IF rxp<5 THEN
      e:=CAST(SET8, b)/CAST(SET8, 2AH);
      WHILE e<>SET8{} DO
        IF e*SET8{0}<>SET8{} THEN INC(headerrs) END;
        e:=SHIFT(e, -1);
      END;
      IF headerrs>rxp THEN         (* allow 0 bit errors on first byte *) 
        headerrs:=0;
        rxp:=0;
      ELSE INC(rxp) END;
    ELSE INC(rxp) END;
    IF rxp>=240 THEN
      headerrs:=0;
      rxp:=0;
      decodeframe92(m);
    END;
  END;
END stobyte92;
*/

static void stobyte92(uint32_t m, char b)
{
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      anonym->rxbuf[anonym->rxp] = b;
      if (anonym->rxp>=5UL || b=='*') ++anonym->rxp;
      else anonym->rxp = 0UL;
      if (anonym->rxp>=240UL) {
         anonym->rxp = 0UL;
         decodeframe92(m);
      }
   }
} /* end stobyte92() */


static void demodbyte92(uint32_t m, char d)
{
   uint32_t maxi;
   uint32_t i;
   int32_t max0;
   int32_t n;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      anonym->rxbyte = anonym->rxbyte/2UL+256UL*(uint32_t)d;
      max0 = 0L;
      maxi = 0UL;
      for (i = 0UL; i<=9UL; i++) {
         n = anonym->asynst[i]-anonym->asynst[(i+1UL)%10UL];
         if (labs(n)>labs(max0)) {
            max0 = n;
            maxi = i;
         }
      } /* end for */
      if (anonym->rxbitc==maxi) {
         if (max0<0L) {
            anonym->rxbyte = (uint32_t)((uint32_t)anonym->rxbyte^0xFFUL);
         }
         stobyte92(m, (char)(anonym->rxbyte&255UL));
      }
      if (d) {
         anonym->asynst[anonym->rxbitc]
                += (32767L-anonym->asynst[anonym->rxbitc])/16L;
      }
      else {
         anonym->asynst[anonym->rxbitc]
                -= (32767L+anonym->asynst[anonym->rxbitc])/16L;
      }
      anonym->rxbitc = (anonym->rxbitc+1UL)%10UL;
   }
/*FOR i:=0 TO HIGH(asynst) DO WrInt(asynst[i], 8) END; WrStrLn(""); */
} /* end demodbyte92() */


static void demodbit92(uint32_t m, float u, float u0)
{
   char d;
   float ua;
   struct R92 * anonym;
   d = u>=u0;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      if (anonym->lastmanch==u0<0.0f) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
      /*WrInt(manchestd,8); */
      if (anonym->manchestd<0L) {
         demodbyte92(m, d);
         /*WrInt(VAL(INTEGER, u0*0.001), 4); WrStr("/");
                WrInt(VAL(INTEGER, u*0.001), 0); */
         /*quality*/
         ua = (float)fabs(u-u0)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.005f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
      }
   }
/*quality*/
} /* end demodbit92() */


static void demod92(float u, uint32_t m)
{
   char d;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      d = u>=0.0f;
      if (anonym->cbit) {
         if (anonym->enabled)  demodbit92(m, u, anonym->lastu);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod92() */


static void demod41(float u, uint32_t m)
{
   char d;
   struct R41 * anonym;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      d = u>=0.0f;
      if (anonym->cbit) {
         if (chan[m].r41.enabled) demodbit41(m, u);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod41() */


static void demodMP3(float u, uint32_t m)
{
   char d;
   struct MP3 * anonym;
   { /* with */
      struct MP3 * anonym = &chan[m].mp3;
      d = u>=0.0f;
      if (anonym->cbit) {
         if (chan[m].mp3.enabled) demodbitmp3(m, u);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demodmp3() */



static void demodPS(float u, uint32_t m)
{
   char d;
   struct PILS * anonym;
   { /* with */
      struct PILS * anonym = &chan[m].pils;
      d = u>=0.0f;
      if (anonym->cbit) {
	 if (chan[m].pils.enabled) demodbitpilot(m, u);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demodPS() */




static void Fsk(uint32_t m)
{
   float ff;
   int32_t lim;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
		//static float Fir(uint32_t in, uint32_t sub, uint32_t step, float fir[], uint32_t fir_len, float firtab[], uint32_t firtab_len)
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L),16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod92(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk() */


static void Fsk41(uint32_t m)
{
   float ff;
   int32_t lim;
   struct R41 * anonym;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afinR41, (uint32_t)((anonym->baudfine&65535L)/4096L),16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod41(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk() */

static void FskMP3(uint32_t m)
{
   float ff;
   int32_t lim;
   struct MP3 * anonym;
   { /* with */
      struct MP3 * anonym = &chan[m].mp3;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afinMP3, (uint32_t)((anonym->baudfine&65535L)/4096L),16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demodMP3(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end FskMP3() */


static void FskPS(uint32_t m)
{
   float ff;
   int32_t lim;
   struct PILS * anonym;
   { 
      struct PILS * anonym = &chan[m].pils;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afinPS, (uint32_t)((anonym->baudfine&65535L)/4096L),16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demodPS(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} // end Fsk() 


static void Fsk10(uint32_t m)
{
   float ff;
   int32_t lim;
   struct M10 * anonym;
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L), 16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod10(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk10() */

static void Fsk20(uint32_t m)
{
   float ff;
   int32_t lim;
   struct M20 * anonym;
   { /* with */
      struct M20 * anonym = &chan[m].m20;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L), 16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod20(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk10() */


/*------------------------------ DFM06 */

static void deinterleave(const char b[], uint32_t b_len,
                uint32_t base, uint32_t len, char db[],
                uint32_t db_len)
{
   uint32_t j;
   uint32_t i;
   uint32_t tmp;
   for (j = 0UL; j<=7UL; j++) {
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         db[i*8UL+j] = b[base+len*j+i];
         if (i==tmp) break;
      } /* end for */
   } /* end for */
} /* end deinterleave() */


static char hamcorr(char b[], uint32_t b_len, uint32_t d,
                uint32_t h)
{
   uint32_t e;
   e = (uint32_t)((b[d]==b[d+2UL])!=(b[h]==b[h+2UL]))+2UL*(uint32_t)
                ((b[d+1UL]==b[d+2UL])!=(b[h+1UL]==b[h+2UL]))+4UL*(uint32_t)
                ((b[d+3UL]==b[h])!=(b[h+1UL]==b[h+2UL]));
   /* hamming matrix multiplication */
   if (e>4UL) b[(h+e)-4UL] = !b[(h+e)-4UL];
   else if (e>0UL) b[(d+e)-1UL] = !b[(d+e)-1UL];
   /*  IF e<>0 THEN WrStr("<");WrInt(e, 1);WrStr(">") END; */
   e = (uint32_t)b[d]+(uint32_t)b[d+1UL]+(uint32_t)
                b[d+2UL]+(uint32_t)b[d+3UL]+(uint32_t)b[h]+(uint32_t)
                b[h+1UL]+(uint32_t)b[h+2UL]+(uint32_t)b[h+3UL];
   return !(e&1);
/* 1 bit checksum */
} /* end hamcorr() */


static char hamming(const char b[], uint32_t b_len, uint32_t len, char db[], uint32_t db_len)
{
   uint32_t j;
   uint32_t i;
   uint32_t tmp;
   tmp = db_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      db[i] = 0;
      if (i==tmp) break;
   } /* end for */
   tmp = len-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      for (j = 0UL; j<=3UL; j++) {
         db[i*4UL+j] = b[i*8UL+j];
      } /* end for */
      for (j = 0UL; j<=3UL; j++) {
         db[i*4UL+j+len*4UL] = b[i*8UL+j+4UL];
      } /* end for */
      if (!hamcorr(db, db_len, i*4UL, i*4UL+len*4UL)) return 0;
      if (i==tmp) break;
   } /* end for */
   return 1;
/*
0000 0000
0001 1110
0010 1101
0011 0011
0100 1011
0101 0101
0110 0110
0111 1000
1000 0111
1001 1001
1010 1010
1011 0100
1100 1100
1101 0010
1110 0001
1111 1111
*/
} /* end hamming() */


static uint32_t bits2val_org(const char b[], uint32_t b_len, uint32_t from, uint32_t len)
{
   uint32_t n;
   n = 0UL;
   while (len>0UL) {
      n = n*2UL+(uint32_t)b[from];
      ++from;
      --len;
   }
   return n;
} /* end bits2val() */

/*
static void wh(uint32_t x)
{
   char tmp;
   x = x&15UL;
   if (x<10UL) osi_WrStr((char *)(tmp = (char)(x+48UL),&tmp), 1u/1u);
   else osi_WrStr((char *)(tmp = (char)(x+55UL),&tmp), 1u/1u);
} 
*/
/* end wh() */

#define B 8 // codeword: 8 bit
#define S 4 // davon 4 bit data

char dat_str[9][13+1];

uint8_t H[4][8] =  // Parity-Check
             {{ 0, 1, 1, 1, 1, 0, 0, 0},
              { 1, 0, 1, 1, 0, 1, 0, 0},
              { 1, 1, 0, 1, 0, 0, 1, 0},
              { 1, 1, 1, 0, 0, 0, 0, 1}};
uint8_t He[8] = { 0x7, 0xB, 0xD, 0xE, 0x8, 0x4, 0x2, 0x1}; // Spalten von H:


uint32_t bits2val(uint8_t *bits, int len) { // big endian
    int j;
    uint32_t val;
    if ((len < 0) || (len > 32)) return -1;
    val = 0;
    for (j = 0; j < len; j++) {
        val |= (bits[j] << (len-1-j));
    }
    return val;
}
/*

void deinterleave(char *str, int L, uint8_t *block) {
    int i, j;
    for (j = 0; j < B; j++) {  // L = 7, 13
        for (i = 0; i < L; i++) {
            if (str[L*j+i] >= 0x30 && str[L*j+i] <= 0x31) {
                block[B*i+j] = str[L*j+i] - 0x30; // ASCII -> bit
            }
        }
    }
}
*/
int check(uint8_t code[8]) {
    int i, j;               // Bei Demodulierung durch Nulldurchgaenge, wenn durch Fehler ausser Takt,
    uint32_t synval = 0;      // verschieben sich die bits. Fuer Hamming-Decode waere es besser,
    uint8_t syndrom[4];       // sync zu Beginn mit Header und dann Takt beibehalten fuer decision.
    int ret=0;

    for (i = 0; i < 4; i++) { // S = 4
        syndrom[i] = 0;
        for (j = 0; j < 8; j++) { // B = 8
            syndrom[i] ^= H[i][j] & code[j];
        }
    }
    synval = bits2val(syndrom, 4);
    if (synval) {
        ret = -1;
        for (j = 0; j < 8; j++) {   // 1-bit-error
            if (synval == He[j]) {  // reicht auf databits zu pruefen, d.h.
                ret = j+1;          // (systematischer Code) He[0..3]
                break;
            }
        }
    }
    else ret = 0;
    if (ret > 0) code[ret-1] ^= 0x1;

    return ret;
}


char nib2chr(uint8_t nib) {
    char c = '_';
    if (nib < 0x10) {
        if (nib < 0xA)  c = 0x30 + nib;
        else            c = 0x41 + nib-0xA;
    }
    return c;
}

int dat_out(uint8_t *dat_bits,uint32_t m) {
    int i, ret = 0;
    static int fr_id;
    // int jahr = 0, monat = 0, tag = 0, std = 0, min = 0;
    int frnr = 0;
    int msek = 0;
    double lat = 0, lon = 0, alt = 0;
    int nib;
    int dvv;  // signed/unsigned 16bit

    fr_id = bits2val(dat_bits+48, 4);


    if (fr_id >= 0  && fr_id <= 8) {
        for (i = 0; i < 13; i++) {
            nib = bits2val(dat_bits+4*i, 4);
            dat_str[fr_id][i] = nib2chr(nib);
        }
        dat_str[fr_id][13] = '\0';
    }

    if (fr_id == 0) {
//        chan[m].dfm6.start = 1;
        frnr = bits2val(dat_bits+24, 8);
        chan[m].dfm6.frnr = frnr;
    }

    if (fr_id == 1) {
        // 00..31: ? GPS-Sats in Sicht?
        msek = bits2val(dat_bits+32, 16);
        chan[m].dfm6.sek = msek/1000.0;
//	chan[m].dfm6.frnr=sek+min*60+hr*3600+day*86400;
    }

    if (fr_id == 2) {
        lat = bits2val(dat_bits, 32);
	lat=lat/1e7;
        chan[m].dfm6.newlat = lat;
        dvv = (short)bits2val(dat_bits+32, 16);  // (short)? zusammen mit dir sollte unsigned sein
        chan[m].dfm6.horiV = dvv/1e2;
    }

    if (fr_id == 3) {
        lon = bits2val(dat_bits, 32);
        lon = lon/1e7;
	chan[m].dfm6.newlon = lon;
        dvv = bits2val(dat_bits+32, 16) & 0xFFFF;  // unsigned
        chan[m].dfm6.dir = dvv/1e2;
    }

    if (fr_id == 4) {
        alt = bits2val(dat_bits, 32);
        alt = alt/1e2;
	chan[m].dfm6.newalt = alt;
        dvv = (short)bits2val(dat_bits+32, 16);  // signed
        chan[m].dfm6.vertV = dvv/1e2;
    }

    if (fr_id == 5) {
    }

    if (fr_id == 6) {
    }

    if (fr_id == 7) {
    }

    if (fr_id == 8) {
        chan[m].dfm6.yr  = bits2val(dat_bits,   12);
        chan[m].dfm6.mon = bits2val(dat_bits+12, 4);
        chan[m].dfm6.day   = bits2val(dat_bits+16, 5);
        chan[m].dfm6.hr   = bits2val(dat_bits+21, 5);
        chan[m].dfm6.min   = bits2val(dat_bits+26, 6);

	if(chan[m].dfm6.newsonde==1){				// gdy nowa, przypisujemy jak leci
	    chan[m].dfm6.prevlat = chan[m].dfm6.newlat;
	    chan[m].dfm6.prevlon = chan[m].dfm6.newlon;
	    chan[m].dfm6.prevalt = chan[m].dfm6.newalt;
	    chan[m].dfm6.newsonde=0;
	    ret=-2;						// dane niepewne, więc oznaczamy jako bledne (z jednej ramki, nikomu korona nie spadnie a i tak czekamy na symbol sondy)
	}

	if(chan[m].dfm6.newalt<1 || chan[m].dfm6.newalt >40000 ||  	// gdy za duży błąd z usrednionych danych
	    (fabs(chan[m].dfm6.prevlat-chan[m].dfm6.newlat)>0.05)||
    	    (fabs(chan[m].dfm6.prevlon-chan[m].dfm6.newlon)>0.05)||
    	    (abs(chan[m].dfm6.prevalt-chan[m].dfm6.newalt)>500)){
	  ret=-2;                                                       // dane bledne
          chan[m].dfm6.sonde_typ = 0;
          printf("R1\n");          
        }
	else
	  ret = fr_id;

	chan[m].dfm6.prevlat = (chan[m].dfm6.newlat + chan[m].dfm6.lat)/2;	// usrednianie poprzednich i obecnych danych
	chan[m].dfm6.prevlon = (chan[m].dfm6.newlon + chan[m].dfm6.lon)/2;
	chan[m].dfm6.prevalt = (chan[m].dfm6.newalt + chan[m].dfm6.alt)/2;

	chan[m].dfm6.lat = chan[m].dfm6.newlat;		// odebrane jako aktualne
	chan[m].dfm6.lon = chan[m].dfm6.newlon;
	chan[m].dfm6.alt = chan[m].dfm6.newalt;

    }

    return ret;
}

// DFM-06 (NXP8)
float fl20(int d) {  // float20
    int val, p;
    float f;
    p = (d>>16) & 0xF;
    val = d & 0xFFFF;
    f = val/(float)(1<<p);
    return  f;
}
/*
float flo20(int d) {
    int m, e;
    float f1, f;
    m = d & 0xFFFF;
    e = (d >> 16) & 0xF;
    f =  m / pow(2,e);
    return  f;
}
*/

// DFM-09 (STM32)
float fl24(int d) {  // float24
    int val, p;
    float f;
    p = (d>>20) & 0xF;
    val = d & 0xFFFFF;
    f = val/(float)(1<<p);
    return  f;
}

// temperature approximation
float get_Temp(float *meas) { // meas[0..4]
// NTC-Thermistor EPCOS B57540G0502
// R/T No 8402, R25=Ro=5k
// B0/100=3450
// 1/T = 1/To + 1/B log(r) , r=R/Ro
// GRAW calibration data -80C..+40C on EEPROM ?
// meas0 = g*(R + Rs)
// meas3 = g*Rs , Rs: dfm6:10k, dfm9:20k
// meas4 = g*Rf , Rf=220k
    float BB0 = 3260.0;       // B/Kelvin, fit -55C..+40C
    float T0 = 25 + 273.15;  // t0=25C
    float R0 = 5.0e3;        // R0=R25=5k
    float Rf = 220e3;        // Rf = 220k
    float g = meas[4]/Rf;
    float R = (meas[0]-meas[3]) / g; // meas[0,3,4] > 0 ?
    float T = 0;                     // T/Kelvin
    if (R > 0)  T = 1/(1/T0 + 1/BB0 * log(R/R0));
    return  T - 273.15; // Celsius
//  DFM-06: meas20 * 16 = meas24
//      -> (meas24[0]-meas24[3])/meas24[4]=(meas20[0]-meas20[3])/meas20[4]
}

// temperature approximation
float get_Temp2(float *meas) { // meas[0..4]
// NTC-Thermistor EPCOS B57540G0502
// R/T No 8402, R25=Ro=5k
// B0/100=3450
// 1/T = 1/To + 1/B log(r) , r=R/Ro
// GRAW calibration data -80C..+40C on EEPROM ?
// meas0 = g*(R+Rs)+ofs
// meas3 = g*Rs+ofs , Rs: dfm6:10k, dfm9:20k
// meas4 = g*Rf+ofs , Rf=220k
    float f  = meas[0],
          f1 = meas[3],
          f2 = meas[4];
    float BB0 = 3260.0;      // B/Kelvin, fit -55C..+40C
    float T0 = 25 + 273.15; // t0=25C
    float R0 = 5.0e3;       // R0=R25=5k
    float Rf2 = 220e3;      // Rf2 = Rf = 220k
    float g_o = f2/Rf2;     // approx gain
    float Rs_o = f1/g_o;    // = Rf2 * f1/f2;
    float Rf1 = Rs_o;       // Rf1 = Rs: dfm6:10k, dfm9:20k
    float g = g_o;          // gain
    float Rb = 0.0;         // offset
    float R = 0;            // thermistor
    float T = 0;            // T/Kelvin

    if       ( 8e3 < Rs_o && Rs_o < 12e3) Rf1 = 10e3;  // dfm6
    else if  (18e3 < Rs_o && Rs_o < 22e3) Rf1 = 20e3;  // dfm9
    g = (f2 - f1) / (Rf2 - Rf1);
    Rb = (f1*Rf2-f2*Rf1)/(f2-f1); // ofs/g

    R = (f-f1)/g;                    // meas[0,3,4] > 0 ?
    if (R > 0)  T = 1/(1/T0 + 1/BB0 * log(R/R0));

    //printf("  (Rso: %.1f , Rb: %.1f)", Rs_o/1e3, Rb/1e3);

    return  T - 273.15;
//  DFM-06: meas20 * 16 = meas24
}

float get_Temp4(float *meas) { // meas[0..4]
// NTC-Thermistor EPCOS B57540G0502
// [  T/C  ,   R/R25   , alpha ] :
// [ -55.0 ,  51.991   ,   6.4 ]
// [ -50.0 ,  37.989   ,   6.2 ]
// [ -45.0 ,  28.07    ,   5.9 ]
// [ -40.0 ,  20.96    ,   5.7 ]
// [ -35.0 ,  15.809   ,   5.5 ]
// [ -30.0 ,  12.037   ,   5.4 ]
// [ -25.0 ,   9.2484  ,   5.2 ]
// [ -20.0 ,   7.1668  ,   5.0 ]
// [ -15.0 ,   5.5993  ,   4.9 ]
// [ -10.0 ,   4.4087  ,   4.7 ]
// [  -5.0 ,   3.4971  ,   4.6 ]
// [   0.0 ,   2.7936  ,   4.4 ]
// [   5.0 ,   2.2468  ,   4.3 ]
// [  10.0 ,   1.8187  ,   4.2 ]
// [  15.0 ,   1.4813  ,   4.0 ]
// [  20.0 ,   1.2136  ,   3.9 ]
// [  25.0 ,   1.0000  ,   3.8 ]
// [  30.0 ,   0.82845 ,   3.7 ]
// [  35.0 ,   0.68991 ,   3.6 ]
// [  40.0 ,   0.57742 ,   3.5 ]
// -> Steinhart–Hart coefficients (polyfit):
    float p0 = 1.09698417e-03,
          p1 = 2.39564629e-04,
          p2 = 2.48821437e-06,
          p3 = 5.84354921e-08;
// T/K = 1/( p0 + p1*ln(R) + p2*ln(R)^2 + p3*ln(R)^3 )
    float Rf = 220e3;    // Rf = 220k
    float g = meas[4]/Rf;
    float R = (meas[0]-meas[3]) / g; // meas[0,3,4] > 0 ?
    float T = 0; // T/Kelvin
    if (R > 0)  T = 1/( p0 + p1*log(R) + p2*log(R)*log(R) + p3*log(R)*log(R)*log(R) );
    return  T - 273.15; // Celsius
//  DFM-06: meas20 * 16 = meas24
//      -> (meas24[0]-meas24[3])/meas24[4]=(meas20[0]-meas20[3])/meas20[4]
}


#define SNbit 0x0100
int conf_out(uint8_t *conf_bits,uint32_t m) {
    int ret = 0;
    int val;
    uint8_t conf_id;
    uint8_t hl;
    uint32_t SN6, SN, snt;
    static int chAbit, chA[2];
    static int chCbit, chC[2];
    static int chDbit, chD[2];
    static int ch7bit, ch7[2];
    static uint32_t SN_A, SN_C, SN_D, SN_7;
    static uint8_t max_ch;
    static uint8_t nul_ch;
    static uint8_t sn2_ch, sn_ch;
    static uint32_t SN_X;
    static int chXbit, chX[2];
    static uint8_t dfm6typ;

    conf_id = bits2val(conf_bits, 4);

    printf(" ID:%02i\n",conf_id);

    if (conf_id > 4 && bits2val(conf_bits+8, 4*5) == 0) nul_ch = bits2val(conf_bits, 8);


    dfm6typ = ((nul_ch & 0xF0)==0x50) && (nul_ch & 0x0F);
    if (dfm6typ  && (chan[m].dfm6.sonde_typ & 0x0F) > 6)
    {   // reset if 0x5A, 0x5B (DFM-06)
        chan[m].dfm6.sonde_typ = 0;
        max_ch = conf_id;
	printf("R2\n");
    }

    if (conf_id > 4 && conf_id > max_ch) max_ch = conf_id; // mind. 5 Kanaele // reset? lower 0xsCaaaab?
    if (conf_id > 4 && conf_id == (nul_ch>>4)+1){  
	if(conf_id==6){
	    chan[m].dfm6.SN6=bits2val(conf_bits+4,24);
	    chan[m].dfm6.ok=1;
	    sprintf(chan[m].dfm6.id,"D6%08x", chan[m].dfm6.SN6);
	    dfm6typ=0x06;
	}else{
	    sn2_ch = bits2val(conf_bits, 8);
	    snt=bits2val(conf_bits+8, 16);
	    if(sn2_ch!=chan[m].dfm6.SNT) {chan[m].dfm6.SNT=sn2_ch; chan[m].dfm6.SNL=0;chan[m].dfm6.SNH=0; chan[m].dfm6.SN6=0; printf("R3\n");}		//jesli inny typ - reset numeru sondy
	    switch(bits2val(conf_bits+24, 4)){
		case 0:
		    if((snt<<16)!=chan[m].dfm6.SNH) {chan[m].dfm6.SNH=snt<<16; chan[m].dfm6.SN6=0; chan[m].dfm6.ok=0; printf("R0\n");}
		    else chan[m].dfm6.ok=1;
		    break;
		case 1:
		    if(snt!=chan[m].dfm6.SNL) {chan[m].dfm6.SNL=snt;chan[m].dfm6.SN6=0;  printf("R1\n");}
		    else chan[m].dfm6.ok=1;
		    break;
		default:
		    chan[m].dfm6.SNT=0; chan[m].dfm6.SNL=0; chan[m].dfm6.SNH=0; chan[m].dfm6.SN6=0; chan[m].dfm6.ok=0; printf("R2\n");		// reset
	    }

	    if ((sn2_ch & 0xFF) == 0xAC ) dfm6typ=0x9;				//DFM-9
	    if ((sn2_ch & 0xFF) == 0xCC ) dfm6typ=0xD;	//DFM17
	    if ((sn2_ch & 0x0F) == 0x0 ) dfm6typ=0xF;				//DFM15
	
	    chan[m].dfm6.sonde_typ=dfm6typ;


	    if(chan[m].dfm6.SNT!=0 && chan[m].dfm6.SNL!=0 && chan[m].dfm6.SNH!=0)
		{ chan[m].dfm6.SN6=chan[m].dfm6.SNH | chan[m].dfm6.SNL; sprintf(chan[m].dfm6.id,"D%1X%08u", dfm6typ,chan[m].dfm6.SN6);}
	    else{chan[m].dfm6.id[0]=0; chan[m].dfm6.SN6=0;}


        }
    }
	


    if (conf_id >= 0 && conf_id <= 4) {
        val = bits2val(conf_bits+4, 4*6);
        chan[m].dfm6.meas24[conf_id] = fl24(val);
        // DFM-09 (STM32): 24bit 0exxxxx
        // DFM-06 (NXP8):  20bit 0exxxx0
        //   fl20(bits2val(conf_bits+4, 4*5))
        //       = fl20(exxxx)
        //       = fl24(exxxx0)/2^4
        //   meas20 * 16 = meas24
    }

    // STM32-status: Bat, MCU-Temp
    if ((chan[m].dfm6.sonde_typ & 0xF) == 0xA) { // DFM-09 (STM32)
        if (conf_id == 0x5) { // voltage
            val = bits2val(conf_bits+8, 4*4);
            chan[m].dfm6.status[0] = val/1000.0;
        }
        if (conf_id == 0x6) { // T-intern (STM32)
            val = bits2val(conf_bits+8, 4*4);
            chan[m].dfm6.status[1] = val/100.0;
        }
    }

    int dig=0;
    int ii;

    for(ii=2;ii<strlen(chan[m].dfm6.id);ii++){
        if(chan[m].dfm6.id[ii]>57 || chan[m].dfm6.id[ii]<48) dig=1;
    }


    if(strlen(chan[m].dfm6.id)<5 || dig){
        time_t t = time(NULL);
	struct tm tm = *localtime(&t);
        int czas=tm.tm_mon + 1 + tm.tm_mday;

	chan[m].dfm6.id[0]='D';
        chan[m].dfm6.id[1]='6';
        chan[m].dfm6.id[2]='D';
        chan[m].dfm6.id[3]='X';
        chan[m].dfm6.id[4]=65+tm.tm_hour;
        chan[m].dfm6.id[5]=65+(int)(czas/25);
        chan[m].dfm6.id[6]=65+czas%25;
        chan[m].dfm6.id[7]=chan[m].freq[2];//rxb[58];
        chan[m].dfm6.id[8]=chan[m].freq[3];//rxb[59];
        chan[m].dfm6.id[9]=chan[m].freq[4];//rxb[60];
        chan[m].dfm6.id[10]=0;
    }


    return ret;
}

void print_gpx(uint32_t m) {
  int i, j;
    char typp;


	  if((chan[m].dfm6.id[0]!='D')||(chan[m].dfm6.id[1]!='6')&&(chan[m].dfm6.id[1]!='9')&&(chan[m].dfm6.id[1]!='F')&&(chan[m].dfm6.id[1]!='D'))
	    chan[m].dfm6.id[0]=0;

	  printf("%02d:DFM %s ",m+1,chan[m].dfm6.id);
	  printf("[%3d] [%04i-%02i-%02i %02i:%02i:%02i ]", chan[m].dfm6.frnr,  chan[m].dfm6.yr,chan[m].dfm6.mon,chan[m].dfm6.day, chan[m].dfm6.hr,chan[m].dfm6.min,(int)chan[m].dfm6.sek );
	  printf("lat: %.6f ", chan[m].dfm6.lat);
	  printf("lon: %.6f ", chan[m].dfm6.lon);
	  printf("alt: %.1f ", chan[m].dfm6.alt);
          printf("vH: %5.2f ", chan[m].dfm6.horiV);
          printf("D: %5.1f ", chan[m].dfm6.dir);
          printf("vV: %5.2f ", chan[m].dfm6.vertV);
          float t = get_Temp(chan[m].dfm6.meas24);
	  float t4 = get_Temp4(chan[m].dfm6.meas24);
          if (t > -270.0) printf("T=%.1fC ", t);
          if (t4 > -270.0) printf("T4=%.1fC ", t4);
          if ((chan[m].dfm6.sonde_typ & 0xFF) == 9) {
              printf("U: %.2fV ", chan[m].dfm6.status[0]);
              printf("Ti: %.2fC ", chan[m].dfm6.status[1]-270.0);
          }
      printf("\n");
}

static char sendDFM(uint32_t m){
    char s[200];
    char tmp[50];
    char ret=0;
    s[0]=0;
    char i;
    


    if (strlen(chan[m].dfm6.id)>5){

	sprintf(tmp, chan[m].dfm6.id);
	tmp[10]=0;

	if(chan[m].dfm6.ok==1) {tmp[0]='E'; printf("OK ****\n");}

	strcat(s,tmp);

	sprintf(tmp,"%04d", chan[m].dfm6.frnr);
	tmp[4]=0;
	strcat(s,tmp);
        sprintf(tmp,"%08.0f", chan[m].dfm6.lat*1000000);
	tmp[8]=0;
	strcat(s,tmp);
        sprintf(tmp,"%09.0f", chan[m].dfm6.lon*1000000);
	tmp[9]=0;
	strcat(s,tmp);
        sprintf(tmp,"%05.0f", chan[m].dfm6.alt);
	tmp[5]=0;
	strcat(s,tmp);
        sprintf(tmp,"%05.0f", chan[m].dfm6.horiV*100);
	tmp[5]=0;
	strcat(s,tmp);
        sprintf(tmp,"%04.0f", chan[m].dfm6.dir*10);
	tmp[4]=0;
	strcat(s,tmp);
        sprintf(tmp,"%05.0f", chan[m].dfm6.vertV*100);
	tmp[5]=0;
	strcat(s,tmp);

	tmp[0] = chan[m].freq[0];
        tmp[1] = chan[m].freq[1];
        tmp[2] = chan[m].freq[2];
        tmp[3] = chan[m].freq[3];
        tmp[4] = chan[m].freq[4];
        tmp[5] = chan[m].freq[5];
	tmp[6] = 0;

	if(strlen(tmp)>2)
	    strcat(s,tmp);
	else
	    strcat(s,"000000");

        float t = get_Temp(chan[m].dfm6.meas24);	
        if (t > -270.0) {
	    sprintf(tmp,"%04.0f", (t+273)*10);
	    tmp[4]=0;
	}
	else sprintf(tmp,"0000");
	strcat(s,tmp);

        if ((chan[m].dfm6.sonde_typ & 0xFF) == 9) {				
            sprintf(tmp,"%04.0f", chan[m].dfm6.status[0]*100);	
	    tmp[4]=0;
	    strcat(s,tmp);
            sprintf(tmp,"%05.0f", chan[m].dfm6.status[1]*100);	
	    tmp[5]=0;
        }
	else sprintf(tmp,"000000000");
	strcat(s,tmp);
	sprintf(tmp,"%4d%02d%02d%02d%02d%02.0f",chan[m].dfm6.yr, chan[m].dfm6.mon, chan[m].dfm6.day, chan[m].dfm6.hr, chan[m].dfm6.min, chan[m].dfm6.sek);
	
        tmp[16]=0;
        strcat(s,tmp);

        tmp[0] = (char)(chan[m].mycallc/16777216UL);
        tmp[1] = (char)(chan[m].mycallc/65536UL&255UL);
        tmp[2] = (char)(chan[m].mycallc/256UL&255UL);
        tmp[3] = (char)(chan[m].mycallc&255UL);
        if (chan[m].mycallc>0UL) tmp[4] = chan[m].myssid;
	else tmp[4] = '\020';
	strcat(s,tmp);

	alludp(chan[m].udptx, 88+5, s, 88+5);
	chan[m].dfm6.ok=0;

    }

    return ret;
}

static void decodeframe6(uint32_t m)
{
   uint32_t j;
   uint32_t i;
   uint32_t rt;

    char ret0,ret1,ret2;

   int frid = -1;

   char s[101+6];
   char tx;
   struct DFM6 * anonym;
   /* build tx frame */
   struct DFM6 * anonym0;
   struct CHAN * anonym1; /* my call if set */
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      deinterleave(anonym->rxbuf, 264ul, 0UL, 7UL, anonym->ch, 56ul);
      deinterleave(anonym->rxbuf, 264ul, 56UL, 13UL, anonym->dh1, 104ul);
      deinterleave(anonym->rxbuf, 264ul, 160UL, 13UL, anonym->dh2, 104ul);

      ret0=hamming(anonym->dh1, 104ul, 13UL, anonym->db1, 104ul);
      ret1=hamming(anonym->dh2, 104ul, 13UL, anonym->db2,104ul);
      ret2=hamming(anonym->ch, 56ul, 7UL, anonym->cb, 56ul);

      if((osic_time()-anonym->lastsent)>600){	//gdy za dlugo nie bylo ramki, kasowanie i liczenie od nowa
	chan[m].dfm6.newsonde=1;
	anonym->lastsent=osic_time();
	chan[m].dfm6.id[0]=0;
      }

      if(chan[m].dfm6.newsonde==0){
	for(i=0;i<6;i++)
	    if(chan[m].freq[0]!=chan[m].pfreq[0])
		chan[m].dfm6.newsonde=1;
      }

      if(chan[m].dfm6.newsonde){
	for(i=0;i<6;i++)
	    chan[m].pfreq[0]=chan[m].freq[0];
      }

      if(ret2>0)
          conf_out(anonym->cb,m);
      if(ret0>0){
          frid = dat_out(anonym->db1,m);
	  if (frid == 8){ 
		anonym->lastsent=osic_time();
	        print_gpx(m);
		sendDFM(m);
    	  }
	  else if(frid==-2){
	    printf("ERR:");
	    print_gpx(m);
	    chan[m].dfm6.id[0]=0;
	  }
      }
      if(ret1>0){
          frid = dat_out(anonym->db2,m);
	  if (frid == 8){ 
		anonym->lastsent=osic_time();
	        print_gpx(m);
		sendDFM(m);
          }
	  else if(frid==-2){
	    printf("ERR:");
	    print_gpx(m);
	    chan[m].dfm6.id[0]=0;
	  }

      }

   }
} /* end decodeframe6() */


static void demodbyte6(uint32_t m, char d)
{
   /*  WrInt(ORD(d),1); */
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      anonym->synword = anonym->synword*2UL+(uint32_t)d;
      if (anonym->rxp>=264UL) {
         if ((anonym->synword&65535UL)==17871UL) anonym->rxp = 0UL;
         else if ((anonym->synword&65535UL)==47664UL) {
            /* inverse start sequence found */
            anonym->polarity = !anonym->polarity;
            anonym->rxp = 0UL;
         }
      }
      else {
         anonym->rxbuf[anonym->rxp] = d;
         ++anonym->rxp;
         if (anonym->rxp==264UL) decodeframe6(m);
      }
   }
} /* end demodbyte6() */


static void demodbit6(uint32_t m, float u, float u0)
{
   char d;
   float ua;
   struct DFM6 * anonym;
   d = u>=u0;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      if (anonym->lastmanch==u0<0.0f) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
      /*WrInt(manchestd,8); */
      if (anonym->manchestd<0L) {
         /*=polarity*/
         demodbyte6(m, d!=anonym->polarity);
         /*quality*/
         ua = (float)fabs(u-u0)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.005f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
      }
   }
/*quality*/
} /* end demodbit6() */


static void demod6(float u, uint32_t m)
{
   char d;
   struct DFM6 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      d = u>=0.0f;
      if (anonym->cbit) {
         demodbit6(m, u, anonym->lastu);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod6() */


static void Fsk6(uint32_t m)
{
   float ff;
   int32_t lim;
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L),16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod6(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk6() */

/*------------------------------ C34 C50 */

static void demodframe34(uint32_t channel)
{
   uint32_t val;
   uint32_t sum2;
   uint32_t sum1;
   uint32_t i;
   double hr;
   char s[101+6];
   char ok0;
   struct C34 * anonym;
   struct CHAN * anonym0; /* call if set */
   char tmp;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      sum1 = 0UL;
      sum2 = 65791UL;
      for (i = 2UL; i<=6UL; i++) {
         sum1 += (uint32_t)(uint8_t)anonym->rxbuf[i];
         sum2 -= (uint32_t)(uint8_t)anonym->rxbuf[i]*(7UL-i);
      } /* end for */
      sum1 = sum1&255UL;
      sum2 = sum2&255UL;
      ok0 = sum1==(uint32_t)(uint8_t)
                anonym->rxbuf[7U] && sum2==(uint32_t)(uint8_t)
                anonym->rxbuf[8U];
      if (anonym->tused+3600UL<osic_time()) {
         anonym->id34.id[0U] = 0; /* timed out context */
         anonym->id50.id[0U] = 0;
      }
      if (verb && ok0 || verb2) {
         if (maxchannels>0UL) {
	    //printf("%02i:",channel+1);
	    printCnDT(channel);
         }
         if (anonym->c50) {
            osi_WrStr("C50 ", 5ul);
            osi_WrStr(anonym->id50.id, 9ul);
         }
         else {
            osi_WrStr("C34 ", 5ul);
            osi_WrStr(anonym->id34.id, 9ul);
         }
         WrdB(chan[channel].adcmax);
         WrQuali(noiselevel(channel));
         Wrtune(chan[channel].adcdc, chan[channel].adcmax);
         osi_WrStr(" [", 3ul);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[2U], 2UL);
         osi_WrStr(" ", 2ul);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[3U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[4U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[5U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[6U], 2UL);
         osi_WrStr(" ", 2ul);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[7U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[8U], 2UL);
         osi_WrStr("] ", 3ul);
      }
      if (ok0) {
         /* chksum ok */
         val = (uint32_t)(uint8_t)anonym->rxbuf[6U]+(uint32_t)
                (uint8_t)anonym->rxbuf[5U]*256UL+(uint32_t)(uint8_t)
                anonym->rxbuf[4U]*65536UL+(uint32_t)(uint8_t)
                anonym->rxbuf[3U]*16777216UL;
         hr = (double)*X2C_CAST(&val,uint32_t,float,float *);
         if (anonym->c50) {
            if (anonym->id50.idtime+3600UL<osic_time()) {
               anonym->id50.id[0U] = 0;
               anonym->id50.idcheck[0U] = 0;
               anonym->id50.idcnt = 0UL;
            }
            /* remove old id */
            switch ((unsigned)anonym->rxbuf[2U]) {
            case '\003':
               /*
                         CHR(02H): hr:=CAST(REAL, CAST(SET32,
                val)/SET32{0..31});
                                   IF verb THEN WrStr("baro ");
                WrFixed(hr, 2, 0); WrStr(""); END;
               */
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp1 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\004':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp2 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\005':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp3 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\020':
               if (hr<=100.0 && hr>=0.0) {
                  if (verb) {
                     osi_WrStr("hum ", 5ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("%", 2ul);
                  }
               }
               break;
            case '\024':
               if (verb) {
                  osi_WrStr("date", 5ul);
                  aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL,
                 s, 101ul);
                  s[0U] = ' ';
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\025':
               if (verb) {
                  aprsstr_TimeToStr((val/10000UL)*3600UL+((val%10000UL)
                /100UL)*60UL+val%100UL, s, 101ul);
                  osi_WrStr("time ", 6ul);
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\026':
               hr = latlong(val, anonym->c50);
               if (hr<89.9 && hr>(-89.9)) {
                  if (verb) {
                     osi_WrStr("lat  ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\027':
               hr = latlong(val, anonym->c50);
               if (hr<180.0 && hr>(-180.0)) {
                  if (verb) {
                     osi_WrStr("long ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\030':
               hr = (double)((float)val*0.1f);
               if (hr<50000.0) {
                  if (verb) {
                     osi_WrStr("alti ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("m", 2ul);
                  }
               }
               break;
            case 'd': /* 66H 67H 68H 89H 6BH seem to be fixed too */
               strncpy(s,"SC50",101u);
               s[4U] = hex(val/4096UL&7UL);
               s[5U] = hex(val/256UL);
               s[6U] = hex(val/16UL);
               s[7U] = hex(val);
               s[8U] = 0;
               s[9U] = 0;
               if (verb) {
                  osi_WrStr("numb ", 6ul);
                  osi_WrStr((char *) &s[4U], 1u/1u);
                  osi_WrStr((char *) &s[5U], 1u/1u);
                  osi_WrStr((char *) &s[6U], 1u/1u);
                  osi_WrStr((char *) &s[7U], 1u/1u);
                  osi_WrStr((char *) &s[8U], 1u/1u);
               }
               /* check name, if changed may be checksum error or 2 sondes on same frequency */
               if (aprsstr_StrCmp(anonym->id50.idcheck, 9ul, s, 101ul)) {
                  ++anonym->id50.idcnt; /* got same name again */
               }
               else {
                  /* new name so check if wrong */
                  aprsstr_Assign(anonym->id50.idcheck, 9ul, s, 101ul);
                  anonym->idcnt = 1UL;
               }
               if (anonym->id50.idcnt>2UL || anonym->id50.id[0U]==0) {
                  /* first name or safe new name */
                  memcpy(anonym->id50.id,anonym->id50.idcheck,9u);
                  anonym->id50.idtime = osic_time();
               }
               anonym->tused = osic_time();
               break;
            default:;
               if (verb2) {
                  /*WrStr("????");*/
                  osic_WrINT32(val, 12UL);
                  osic_WrINT32(val/65536UL, 7UL);
                  osic_WrINT32(val&65535UL, 7UL);
                  osic_WrFixed((float)hr, 2L, 10UL);
                  osi_WrStr(" ", 2ul);
                  for (i = 31UL;; i--) {
                     osi_WrStr((char *)(tmp = (char)
                (48UL+(uint32_t)X2C_IN(i,32,(uint32_t)val)),&tmp),
                1u/1u);
                     if (i==0UL) break;
                  } /* end for */
               }
               break;
            } /* end switch */
         }
         else {
            /* SC34 */
            if (anonym->id34.idtime+3600UL<osic_time()) {
               anonym->id34.id[0U] = 0;
               anonym->id34.idcheck[0U] = 0;
               anonym->id34.idcnt = 0UL;
            }
            /* remove old id */
            switch ((unsigned)anonym->rxbuf[2U]) {
            case '\003':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp1 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\007':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("dewp ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\024':
               if (verb) {
                  osi_WrStr("date", 5ul);
                  aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL,
                 s, 101ul);
                  s[0U] = ' ';
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\025':
               if (verb) {
                  aprsstr_TimeToStr((val/10000UL)*3600UL+((val%10000UL)
                /100UL)*60UL+val%100UL, s, 101ul);
                  osi_WrStr("time ", 6ul);
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\026':
               hr = latlong(val, anonym->c50);
               if (hr<89.9 && hr>(-89.9)) {
                  if (verb) {
                     osi_WrStr("lati ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\027':
               hr = latlong(val, anonym->c50);
               if (hr<180.0 && hr>(-180.0)) {
                  if (verb) {
                     osi_WrStr("long ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\030':
               hr = (double)((float)val*0.1f);
               if (hr<50000.0) {
                  if (verb) {
                     osi_WrStr("alti ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("m", 2ul);
                  }
               }
               break;
            case '\031':
               hr = (double)((float)val*0.1852f);
                /*1.609*/ /*1.852*/ /* guess knots or miles */
               if (hr<1000.0) {
                  if (verb) {
                     osi_WrStr("wind ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("km/h", 5ul);
                  }
               }
               break;
            case '\032':
               hr = (double)((float)val*0.1f);
               if (hr>=0.0 && hr<=360.0) {
                  if (verb) {
                     osi_WrStr("wdir ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("deg", 4ul);
                  }
               }
               break;
            case 'd':
               strncpy(s,"SC34",101u);
                /* build a name from seems like serial number */
               s[4U] = hex(val/65536UL);
               s[5U] = hex(val/4096UL);
               s[6U] = hex(val/256UL);
               s[7U] = hex(val/16UL);
               s[8U] = hex(val);
               s[9U] = 0;
               if (verb) {
                  osi_WrStr("numb ", 6ul);
                  osi_WrStr((char *) &s[4U], 1u/1u);
                  osi_WrStr((char *) &s[5U], 1u/1u);
                  osi_WrStr((char *) &s[6U], 1u/1u);
                  osi_WrStr((char *) &s[7U], 1u/1u);
                  osi_WrStr((char *) &s[8U], 1u/1u);
               }
               /* check name, if changed may be checksum error or 2 sondes on same frequency */
               if (aprsstr_StrCmp(anonym->id34.idcheck, 9ul, s, 101ul)) {
                  ++anonym->id34.idcnt; /* got same name again */
               }
               else {
                  /* new name so check if wrong */
                  aprsstr_Assign(anonym->id34.idcheck, 9ul, s, 101ul);
                  anonym->idcnt = 1UL;
               }
               if (anonym->id34.idcnt>3UL || anonym->id34.id[0U]==0) {
                  /* first name or safe new name */
                  memcpy(anonym->id34.id,anonym->id34.idcheck,9u);
                  anonym->id34.idtime = osic_time();
               }
               anonym->tused = osic_time();
               break;
            default:;
               if (verb2) {
                  osi_WrStr("????", 5ul);
                  osic_WrINT32(val, 12UL);
                  osic_WrFixed((float)hr, 2L, 10UL);
               }
               break;
            } /* end switch */
         }
         /* build tx frame */
         if ((anonym->c50 && anonym->id50.id[0U])
                && aprsstr_StrCmp(anonym->id50.idcheck, 9ul, anonym->id50.id,
                 9ul) || (!anonym->c50 && anonym->id34.id[0U])
                && aprsstr_StrCmp(anonym->id34.idcheck, 9ul, anonym->id34.id,
                 9ul)) {
            /* stop sending if ambigous id */
            if (anonym->c50) {
               for (i = 0UL; i<=8UL; i++) {
                  s[i] = anonym->id50.id[i];
               } /* end for */
            }
            else {
               for (i = 0UL; i<=8UL; i++) {
                  s[i] = anonym->id34.id[i];
               } /* end for */
            }
            s[9U] = 0;
            { /* with */
               struct CHAN * anonym0 = &chan[channel];
               s[10U] = (char)(anonym0->mycallc/16777216UL);
               s[11U] = (char)(anonym0->mycallc/65536UL&255UL);
               s[12U] = (char)(anonym0->mycallc/256UL&255UL);
               s[13U] = (char)(anonym0->mycallc&255UL);
               if (anonym0->mycallc>0UL) s[14U] = anonym0->myssid;
               else s[14U] = '\020';
            }
            for (i = 0UL; i<=6UL; i++) {
               s[i+15UL] = anonym->rxbuf[i+2UL]; /* payload */
            } /* end for */

         s[22] = chan[channel].freq[0];
         s[23] = chan[channel].freq[1];
         s[24] = chan[channel].freq[2];
         s[25] = chan[channel].freq[3];
         s[26] = chan[channel].freq[4];
         s[27] = chan[channel].freq[5];



            alludp(chan[channel].udptx, 22UL+6, s, 101ul+6);
         }
         else if (verb) {
            if (anonym->c50) {
               if (anonym->id50.id[0U]) {
                  osi_WrStr(" changing name ", 16ul);
                  osi_WrStr(anonym->id50.id, 9ul);
                  osi_WrStr("<->", 4ul);
                  osi_WrStr(anonym->id50.idcheck, 9ul);
               }
            }
            else if (anonym->id34.id[0U]) {
               osi_WrStr(" changing name ", 16ul);
               osi_WrStr(anonym->id34.id, 9ul);
               osi_WrStr("<->", 4ul);
               osi_WrStr(anonym->id34.idcheck, 9ul);
            }
         }
      }
      else if (verb2) {
         /*build tx frame */
         osi_WrStr("---- chksum ", 13ul);
         osi_WrHex(sum1, 2UL);
         osi_WrHex(sum2, 2UL);
      }
      if (verb2 || ok0 && verb) osi_WrStrLn("", 1ul);
   }
/* name(9) 0C call(5) playload(7) */
} /* end demodframe34() */


static void demodbit34(uint32_t channel, char d)
{
   /*IF NOT verb THEN WrInt(ORD(d),1); END; */
   struct C34 * anonym;
   d = !d;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      anonym->rxbytec = anonym->rxbytec*2UL+(uint32_t)d;
      if ((anonym->rxbytec&268435455UL)==234942462UL) {
         /* c34 1110 0000 0000 1110 1111 1111 1110*/
         anonym->c50 = 0;
         anonym->rxp = 2UL;
         anonym->rxbitc = 0UL;
      }
      else if ((anonym->rxbytec&2097151UL)==3070UL) {
         /*IF NOT verb THEN WrStrLn(""); END; */
         /* c50 0 0000 0000 1011 1111 1110 */
         anonym->c50 = 1;
         anonym->rxp = 2UL;
         anonym->rxbitc = 0UL;
      }
      /*IF NOT verb THEN WrStrLn(""); END; */
      if ((anonym->c50 || anonym->rxbitc) || !d) {
         if (anonym->rxbitc<=8UL) {
            /* databits */
            anonym->rxbyte = (anonym->rxbyte&255UL)/2UL;
            if (d) anonym->rxbyte += 128UL;
            ++anonym->rxbitc;
         }
         else if (anonym->rxp>0UL) {
            /* byte ready */
            anonym->rxbitc = 0UL;
            anonym->rxbuf[anonym->rxp] = (char)anonym->rxbyte;
            /*WrHex(rxbyte, 3); */
            ++anonym->rxp;
            if (anonym->rxp>8UL) {
               /*IF NOT verb THEN WrStr("*"); END; */
               demodframe34(channel);
               anonym->rxp = 0UL;
            }
         }
      }
   }
} /* end demodbit34() */


static void demod34(float u, uint32_t channel)
{
   char d;
   struct C34 * anonym;
   d = u>=0.0f;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      if (anonym->cbit) {
         demodbit34(channel, d);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         /*squelch*/
         anonym->sqmed[d] = anonym->sqmed[d]+(u-anonym->sqmed[d])*0.05f;
         anonym->noise = anonym->noise+((float)fabs(u-anonym->sqmed[d])
                -anonym->noise)*0.05f;
      }
      else {
         /*squelch*/
         anonym->plld = d;
      }
      anonym->cbit = !anonym->cbit;
   }
} /* end demod34() */


static void Afsk(uint32_t channel)
{
   float ff;
   float b;
   float a;
   float d;
   float mid;
   float right;
   struct C34 * anonym;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      right = Fir(afin, 0UL, 16UL, chan[channel].afir, 32ul, anonym->afirtab,512ul);
      if (anonym->left<0.0f!=right<0.0f) {
         d = X2C_DIVR(anonym->left,anonym->left-right);
         a = (float)(uint32_t)X2C_TRUNCC(d*16.0f+0.5f,0UL,X2C_max_longcard);
         b = a*0.0625f;
         if ((uint32_t)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)>0UL && (uint32_t)X2C_TRUNCC(a,0UL,X2C_max_longcard)<16UL) {
            mid = Fir(afin, 16UL-(uint32_t)X2C_TRUNCC(a,0UL, X2C_max_longcard), 16UL, chan[channel].afir, 32ul,anonym->afirtab, 512ul);
            if (anonym->left<0.0f!=mid<0.0f) {
               d = (X2C_DIVR(anonym->left,anonym->left-mid))*b;
            }
            else d = b+(X2C_DIVR(mid,mid-right))*(1.0f-b);
         }
         if (anonym->tcnt+d!=0.0f) {
            anonym->freq = X2C_DIVR(1.0f,anonym->tcnt+d);
         }
         anonym->tcnt = 0.0f-d;
      }
      anonym->tcnt = anonym->tcnt+1.0f;
      anonym->left = right;
      anonym->dfir[anonym->dfin] = anonym->freq-anonym->afskmidfreq;
      anonym->dfin = anonym->dfin+1UL&63UL;
      anonym->baudfine += (int32_t)anonym->demodbaud;
      if (anonym->baudfine>=65536L) {
         anonym->baudfine -= 65536L;
         if (anonym->baudfine<65536L) {
            /* normal alway true */
            ff = Fir(anonym->dfin, (uint32_t)(16L-anonym->baudfine/4096L), 16UL, anonym->dfir, 64ul, anonym->dfirtab, 1024ul);
            demod34(ff, channel);
         }
      }
   }
} /* end Afsk() */



static void demodbitIMET(uint32_t channel, char d)
{
   /*IF NOT verb THEN WrInt(ORD(d),1); END; */
   struct IMET * anonym;
   d = !d;
   { /* with */
      struct IMET * anonym = &chan[channel].imet;
      anonym->rxbytec = anonym->rxbytec*2UL+(uint32_t)d;
      if ((anonym->rxbytec&0xFFFFFFFF)==0x3FFFFD01) {
         /* IMET 111111111111111111110100000001*/
    //     anonym->imet = 1;
         anonym->rxp = 2UL;
         anonym->rxbitc = 0UL;
      }

      if (anonym->rxbitc || !d) {
         if (anonym->rxbitc<=8UL) {
            /* databits */
            anonym->rxbyte = (anonym->rxbyte&255UL)/2UL;
            if (d) anonym->rxbyte += 128UL;
            ++anonym->rxbitc;
         }
         else if (anonym->rxp>0UL) {
            /* byte ready */
            anonym->rxbitc = 0UL;
            anonym->rxbuf[anonym->rxp] = (char)anonym->rxbyte;
            /*WrHex(rxbyte, 3); */
            ++anonym->rxp;
            if (anonym->rxp>8UL) {
//               demodframeIMET(channel);
               anonym->rxp = 0UL;
            }
         }
      }
   }
} 


static void demodIMET(float u, uint32_t channel)
{
   char d;
   struct IMET * anonym;
   d = u>=0.0f;
   { /* with */
      struct IMET * anonym = &chan[channel].imet;
      if (anonym->cbit) {
         demodbitIMET(channel, d);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         /*squelch*/
         anonym->sqmed[d] = anonym->sqmed[d]+(u-anonym->sqmed[d])*0.05f;
         anonym->noise = anonym->noise+((float)fabs(u-anonym->sqmed[d])-anonym->noise)*0.05f;
      }
      else {
         /*squelch*/
         anonym->plld = d;
      }
      anonym->cbit = !anonym->cbit;
   }
}


static void AfskIMET(uint32_t channel)
{
   float ff;
   float b;
   float a;
   float d;
   float mid;
   float right;
   struct IMET * anonym;
   { /* with */
      struct IMET * anonym = &chan[channel].imet;
      right = Fir(afin, 0UL, 16UL, chan[channel].afir, 32ul, anonym->afirtab,512ul);
      if (anonym->left<0.0f!=right<0.0f) {
         d = X2C_DIVR(anonym->left,anonym->left-right);
         a = (float)(uint32_t)X2C_TRUNCC(d*16.0f+0.5f,0UL,X2C_max_longcard);
         b = a*0.0625f;
         if ((uint32_t)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)>0UL && (uint32_t)X2C_TRUNCC(a,0UL,X2C_max_longcard)<16UL) {
            mid = Fir(afin, 16UL-(uint32_t)X2C_TRUNCC(a,0UL, X2C_max_longcard), 16UL, chan[channel].afir, 32ul,anonym->afirtab, 512ul);
            if (anonym->left<0.0f!=mid<0.0f) {
               d = (X2C_DIVR(anonym->left,anonym->left-mid))*b;
            }
            else d = b+(X2C_DIVR(mid,mid-right))*(1.0f-b);
         }
         if (anonym->tcnt+d!=0.0f) {
            anonym->freq = X2C_DIVR(1.0f,anonym->tcnt+d);
         }
         anonym->tcnt = 0.0f-d;
      }
      anonym->tcnt = anonym->tcnt+1.0f;
      anonym->left = right;
      anonym->dfir[anonym->dfin] = anonym->freq-anonym->afskmidfreq;
      anonym->dfin = anonym->dfin+1UL&63UL;
      anonym->baudfine += (int32_t)anonym->demodbaud;
      if (anonym->baudfine>=65536L) {
         anonym->baudfine -= 65536L;
         if (anonym->baudfine<65536L) {
            /* normal alway true */
            ff = Fir(anonym->dfin, (uint32_t)(16L-anonym->baudfine/4096L), 16UL, anonym->dfir, 64ul, anonym->dfirtab, 1024ul);
            demodIMET(ff, channel);
         }
      }
   }
} /* end Afsk() */


static void getadc(void)
{
   int32_t sl;
   int32_t l;
   int32_t max0[64];
   int32_t min0[64];
   uint32_t ch;
   uint32_t c;
   unsigned int i;
   struct CHAN * anonym;
   uint32_t tmp,pos;
   int chno;
   char tmps[10];
   char mod=0;

    struct R92 * anonymz;
    struct R41 * anonym0;
    struct DFM6 * anonym1;
    struct C34 * anonym2;
    struct PILS * anonym5;
    struct M10 * anonym6;
    struct M20 * anonym7;
    struct IMET * anonym8;
    struct MP3 * anonym9;

   c = 0UL;
   mod=0;
   do {
      mod=0;
      if (adcbufrd>=adcbufsamps) {
         adcbufrd = 0UL;
         l = osi_RdBin(soundfd, (char *)adcbuf, 8192u/1u, adcbuflen*2UL);

	 sprintf(tmps,"%c%c%c%c\n",adcbuf[0],adcbuf[1],adcbuf[2],adcbuf[3]);
	 

	 if(tmps[0]=='9' && tmps[1]=='S' && tmps[2]=='K' && tmps[3]=='P'){ //freq table
	    //printf("Update channel table\n");
	    pos=4;
	    mod=1;
            chno=0;

	    while(adcbuf[pos]!=0){
		chno=(int)(((char)adcbuf[pos]-48)*10+((char)adcbuf[pos+1]-48)-1);
		if(chno<0) break;

		struct R92 * anonymz = &chan[chno].r92;
		anonymz->rxp = 0UL;
		anonymz->rxbitc = 0UL;
		anonymz->rxbyte = 0UL;
		struct R41 * anonym0 = &chan[chno].r41;
		anonym0->rxp = 0UL;
		anonym0->rxbitc = 0UL;
		anonym0->rxbyte = 0UL;
		struct MP3 * anonym9 = &chan[chno].mp3;
		anonym9->rxp = 0UL;
		anonym9->rxbitc = 0UL;
		anonym9->rxbyte = 0UL;
		struct PILS * anonym5 = &chan[chno].pils;
		anonym5->rxp = 0UL;
		anonym5->rxbitc = 0UL;
		anonym5->rxbyte = 0UL;
		struct DFM6 * anonym1 = &chan[chno].dfm6;
		anonym1->rxp = 264UL;
		struct M10 * anonym6 = &chan[chno].m10;
		anonym6->rxp = 101UL;
		struct M20 * anonym7 = &chan[chno].m20;
		anonym7->rxp = 70UL;
		struct C34 * anonym2 = &chan[chno].c34;
		anonym2->rxp = 0UL;
		struct IMET * anonym8 = &chan[chno].imet;
		anonym8->rxp = 0UL;

		chan[chno].freq[0]=adcbuf[pos+2];
		chan[chno].freq[1]=adcbuf[pos+3];
		chan[chno].freq[2]=adcbuf[pos+4];
		chan[chno].freq[3]=adcbuf[pos+5];
		chan[chno].freq[4]=adcbuf[pos+6];
		chan[chno].freq[5]=adcbuf[pos+7];
		chan[chno].freq[6]=0;
		pos+=8;
		//printf("CH:%i QRG:",chno);
		//printf(chan[chno].freq);printf("\n");
	    }
	 }else if(mod==0){
             adcbufsamps = 0UL;
	    if (l<0L) {
    		if (abortonsounderr) Error("Sounddevice Failure", 20ul);
        	else {
            	    osic_Close(soundfd);
            	    usleep(100000UL);
            	    OpenSound();
            	    return;
        	}
    	    }
            if (l<2L) return;
            adcbufsamps = (uint32_t)(l/2L);
            if (debfd>=0L) {
        	osi_WrBin(debfd, (char *)adcbuf, 8192u/1u, adcbufsamps*2UL);
            }
            tmp = maxchannels;
            ch = 0UL;
            if (ch<=tmp) for (;; ch++) {
        	chan[ch].adcdc += (max0[ch]+min0[ch])/2L-chan[ch].adcdc>>4;
        	chan[ch].adcmax += (max0[ch]-min0[ch])-chan[ch].adcmax>>4;
        	max0[ch] = -32768L;
        	min0[ch] = 32767L;
        	if (ch==tmp) break;
            } /* end for */
            adcbufsampx = X2C_max_longcard;
        }
      }
        sl = (int32_t)adcbuf[adcbufrd];
        if (cfgchannels==0UL && (sl&1)) {
         //auto channels channel 0 //WrInt(lastc, 1); WrStrLn(" ch1");
            if (adcbufsampx!=X2C_max_longcard) {
        	ch = (adcbufrd-adcbufsampx)-1UL;
        	/*WrInt(ch, 1); WrStrLn(" ch"); */
        	if (ch<63UL) {
/*            	    if (verb && maxchannels!=ch && mod==0) {
                	osi_WrStr("channels changed from ", 23ul);
                	osic_WrINT32(maxchannels+1UL, 0UL);
                	osi_WrStr(" to ", 5ul);
                	osic_WrINT32(ch+1UL, 0UL);
                	osi_WrStrLn("", 1ul);
                    }
*/
            	    maxchannels = ch;
        	}
            }
            adcbufsampx = adcbufrd;
            c = 0UL;
        }
        sl = (int32_t)((uint32_t)sl&0xFFFFFFFEUL);
        if (sl==0L) ++chan[c].squelch;
        else chan[c].squelch = 0UL;
        sl -= chan[c].adcdc;
        ++adcbufrd;
        chan[c].afir[afin] = (float)(sl-chan[c].adcdc);
        if (sl>max0[c]) max0[c] = sl;
        if (sl<min0[c]) min0[c] = sl;
        ++c;
   } while (c<=maxchannels);
   afin = afin+1UL&31UL;
   afinR41=afin;
   afinPS=afin;
   afinMP3=afin;
   tmp = maxchannels;
   c = 0UL;
   if (c<=tmp) for (;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         if (anonym->squelch<64UL) {
            /* squelch open */
            if (anonym->r92.enabled) Fsk(c); // || anonym->r41.enabled || anonym->pils.enabled) Fsk(c);
	    if (anonym->r41.enabled)  Fsk41(c);
	    if (anonym->mp3.enabled)  FskMP3(c);
	    if (anonym->pils.enabled) FskPS(c);
            if (anonym->c34.enabled) Afsk(c);
	//    if (anonym->imet.enabled) AfskIMET(c);
            if (anonym->dfm6.enabled) Fsk6(c);
	    if (anonym->m10.enabled) Fsk10(c);
	    if (anonym->m20.enabled) Fsk20(c);
         }
      }
      if (c==tmp) break;
   } /* end for */
} /* end getadc() */

static uint16_t sondeudp_POLY = 0x1021U;


static void Gencrctab(void)
{
   uint16_t j;
   uint16_t i;
   uint16_t crc;
   for (i = 0U; i<=255U; i++) {
      crc = (uint16_t)(i*256U);
      for (j = 0U; j<=7U; j++) {
         if ((0x8000U & crc)) crc = X2C_LSH(crc,16,1)^0x1021U;
         else crc = X2C_LSH(crc,16,1);
      } /* end for */
      CRCTAB[i] = X2C_LSH(crc,16,-8)|X2C_LSH(crc,16,8);
   } /* end for */
} /* end Gencrctab() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(CNAMESTR)!=9) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   Gencrctab();
   memset((char *)chan,(char)0,sizeof(struct CHAN [64]));
   Parms();
   initrsc();
   getst = 0UL;
   afin = 0UL;
   soundbufs = 0UL;
   /*  IF verb THEN WrStrLn("Frame ID       level-L qual level-R qual") END;
                */
   adcbufrd = 0UL;
   adcbufsamps = 0UL;
   adcbufsampx = X2C_max_longcard;

//test
/*
    memcpy(chan[0].m20.rxbuf, (char [])
			    {0x45,0x20,0x00,0x00,0x7a,0x08,0xf0,0x0f,0x00,0x25,0x43,0x00,0x00,0x00,0x00,0x04,0xf9,0x7e,0xf2,0x9c,0x00,0x69,0xbd,0x74,0x00,0x00
			    ,0x08,0x23,0x03,0x2d,0xe7,0x51,0x00,0xe4,0xf7,0x49,0xbc,0x3e,0xe0,0x21,0x00,0x00,0x00,0x00,0xff,0x02,0x22,0xef,0x50,0xf3,0x09,0x64
			    ,0x57,0xae,0x25,0x00,0xa4,0xb5,0x40,0x51,0x00,0x20,0x00,0x94,0x02,0x6f,0x00,0x04,0xe1,0xfb}, sizeof chan[0].m20.rxbuf);
   decodeframe20(0);
*/
/*
uint32_t i; char fullid[15];
char rxb[]={0x45,0x20,0x00,0x00,0x7a,0x08,0xf0,0x0f,0x00,0x25,0x43,0x00,0x00,0x00,0x00,0x04,0xf9,0x7e,0xf2,0x9c,0x00,0x69,0xbd,0x74,0x00,0x00
                            ,0x08,0x23,0x03,0x2d,0xe7,0x51,0x00,0xe4,0xf7,0x49,0xbc,0x3e,0xe0,0x21,0x00,0x00,0x00,0x00,0xff,0x02,0x22,0xef,0x50,0xf3,0x09,0x64
                            ,0x57,0xae,0x25,0x00,0xa4,0xb5,0x40,0x51,0x00,0x20,0x00,0x94,0x02,0x6f,0x00,0x04,0xe1,0xfb};

//char rxb[]={0x45,0x20,0x88,0x22,0x57,0x24,0x0b,0x0c,0x27,0xbf,0xfc,0xfd,0xc0,0xfd,0xdb,0x02,0x08,0xba,0xf2,0x9c,0x00,0xd8,0x67,0x3e,0x02,0x08,0x08,0x39,0x02,0xe8,0x0f,0xc6,0x01,0x01,0xce,0xf4,0x2f,0x01,0xaa,0xd7,
//                      0x25,0x00,0x00,0x00,0xfc,0x33,0x22,0xe2,0x50,0xd1,0x08,0x24,0xc7,0x91,0x05,0x00,0xe8,0x15,0x47,0x4d,0x00,0x18,0x00,0x3c,0x08,0x0f,0x01,0x04,0x95,0xfe};

unsigned int rxb_len=0x45;
i = m10card(rxb, rxb_len, 34L, 1L); // 002-2-xxxxx 911-2-xxxxx
         fullid[0U] = (char)((i&127UL)/12UL+48UL);
         fullid[1U] = (char)(((i&127UL)%12UL+1UL)/10UL+48UL);
         fullid[2U] = (char)(((i&127UL)%12UL+1UL)%10UL+48UL);
         fullid[3U] = '-';
         fullid[4U] = (char)(i/128UL+1UL+48UL);
         fullid[5U] = '-';
         i = m10card(rxb, rxb_len, 35L, 2L)/4UL;
//	 printf("%
         fullid[6U] = (char)((i/10000UL)%10UL+48UL);
         fullid[7U] = (char)((i/1000UL)%10UL+48UL);
         fullid[8U] = (char)((i/100UL)%10UL+48UL);
         fullid[9U] = (char)((i/10UL)%10UL+48UL);
         fullid[10U] = (char)(i%10UL+48UL);
         fullid[11U] = 0;

printf("SN:%s\n",fullid);
*/
   for (;;) getadc();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
