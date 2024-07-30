//______________________________________________________________________________________________________________________
//   Program wysyłający powiadomienie na telefon za pośrednictwem Push Bullet (https://www.pushbullet.com)              |
//   Jeśli sonda meteo znajdzie się w promieniu mniejszym niż wskazany i pozostaje nieruchoma przez czas większy        |
//   niż wskazany wysyłane jest powiadomienie na urządzenie określone przez token Push Bullet (np. telefon).            |
//   Do działania wymaga biblioteki libcurl.                                                                            |
//   Na początku pliku należy wpisać swoje dane - koordynaty (np. Google Maps)                                          |
//    i token z www.pushbullet.com (Settings->Account->access Tokens). 						        |
//   Kompilacja do programu wykonywalnego w RPI:                                                                        |
//     gcc -Wall program.c -o program.out -lcurl -lm                                                                    |
//   Wynikowy plik wykonywalny program.out można np. wywoływać co określony czas z Crona.                               |
//															|
//       INSTALACJA: 													|
//   -------------------												|
//   sudo apt update && sudo apt -y upgrade										|
//   sudo apt install libcurl4												|
//   cd ~														|
//   mkdir dist														|
//   cd dist														|
//   wget https://raw.githubusercontent.com/SP5MI/spdxl/a344a31a9280edb396bcbc280cfb5b0b337233dc/program.c		|
//   nano program.c													|
//   ----------!!!!!!!!!!!!!!!!!!tutaj należy uzupełnić swoje dane!!!!!!!!!!!!!!--------				|
//   gcc -Wall program.c -o program.out -lcurl -lm										|
//   crontab -l > mycron												|
//   echo "*/2  * * * * ~/dist/program.out 2>&1 | /usr/bin/logger -t DIST" >> mycron					|
//   crontab mycron													|
//   rm mycron														|
//______________________________________________________________________________________________________________________|

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>


//-------------!! Tu wpisz swoje dane !!------------------------------------------------------------------

double lat1 = 50.000000; 				        //szerokość geograficzna stacji
double long1 = 20.000000; 			        	//długość geograficzna stacji
char * push_id = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; 	//token wygenerowany na https://www.pushbullet.com
int qth_odl = 15;						//odległość od podanych wyżej koordynat poniżej której przychodzi powiadomienie (km)
int secs = 150;							//czas "nieruchomości", żeby wykluczyć lecące i powiadamiać tylko o leżących (sekundy). Ustawienie 0 będzie powiadamiać o wszystkich.

//---------------------------------------------------------------------------------------------------------

char *tablica[9];


double pomiar(float s_lon, float s_lat, double lat1, double long1)
{
    float earth_radius = 6365.045;
    double lat2, long2;
    double dLat, dLong, a, c;

    lat1  = (lat1 * (M_PI / 180));
    long1 = (long1 * (M_PI / 180));
    lat2 = (s_lon * (M_PI / 180));
    long2 = (s_lat * (M_PI / 180));

    dLat  = lat2 - lat1;
    dLong = long2 - long1;

    a = sin(dLat/2) * sin(dLat/2) +
        cos(lat1) * cos(lat2) * sin(dLong/2) * sin(dLong/2);
    c = 2 * atan2(sqrt(a), sqrt(1-a));

    double odleglosc = (earth_radius * c);
    return odleglosc;
}


unsigned long czas(unsigned long s_time)
{
    unsigned long b_czas = time(NULL);
    unsigned long roznica = (b_czas - s_time);
    return roznica;
}


int libcurl(char *seria, double s_odleglosc, char * push_id)
{
   CURL *curl;
   CURLcode res;
   curl = curl_easy_init();
   if(curl) {
     struct curl_slist *chunk = NULL;
     char data[300] = "{\"body\":\"Sonda meteorologiczna numer ";
     const char * s_data = seria;
     strcat(data, s_data);
     char data1[200] = " w pobliżu twojego QTH! przybliżony dystans: ";
     strcat(data, data1);
     char c_odleglosc[20];
     snprintf(c_odleglosc, 50, "%f", s_odleglosc);
     strcat(data, c_odleglosc);
     char data2[] =" km.\",\"title\":\"Uwaga, sonda!\",\"type\":\"note\"}";
     strcat(data, data2);
     chunk = curl_slist_append(chunk, "Accept:");
     char acces_token[] = "Access-Token: ";
     strcat(acces_token, push_id);
     printf("alces token ------------------------- %s\n", acces_token);
     chunk = curl_slist_append(chunk, acces_token);
     chunk = curl_slist_append(chunk, "Content-Type: application/json");
     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
     curl_easy_setopt(curl, CURLOPT_URL, "https://api.pushbullet.com/v2/pushes");
     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
     curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
     res = curl_easy_perform(curl);
      if(res != CURLE_OK)
       fprintf(stderr, "curl_easy_perform() failed: %s\n",
               curl_easy_strerror(res));

      curl_easy_cleanup(curl);

      curl_slist_free_all(chunk);
   }
   return 0;
}

void check(char **numer, double dystans, unsigned long czas, int qth_odl, int secs)
{
   FILE *fp;
   fp = fopen("/tmp/sondy", "a+");
   if(dystans < qth_odl && czas > secs)
   {
      int c = 0;
      printf("\n\n Numer: %s\n\n", *numer);
//      printf(" dystans: %f.3\n", dystans);
      char s_row[20];
      while(fgets(s_row, sizeof(s_row), fp))
      {
       printf("Linia z pliku sondy: %s\n", s_row);
       size_t len = sizeof(*numer);
       int comp = strncmp(s_row, *numer, len);
       printf("wynik porownania: %d\n", comp);
       if(comp == 0){c = 1; break;}
       else{c = 0;}
       }
printf("%d\n", c);
if(c==0){
fputs(*numer, fp);
fputs("\n", fp);
libcurl(*numer, dystans, push_id);
}


}
fclose(fp);
}

int main()
{
	FILE *fp;
	char row[200];
	fp = fopen("/tmp/sonde.csv", "r"); 
	if (fp==NULL)
	{
		perror("Nie mozna otworzyc pliku");
		return 1;
	}
	else
	{
        //printf("Plik sonde.csv załadowany.\n");
	}

while(fgets(row, sizeof(row), fp))
          {
          int i = 0;
          char *ptr;
          ptr = strtok(row, ";");
          while(ptr != NULL)
          {
           tablica[i++] = ptr;
           ptr = strtok(NULL, ";");
          }

	float s_lon = atof(tablica[1]);
	float s_lat = atof(tablica[2]);
	unsigned long s_time = atof(tablica[8]);
	double s_odleglosc = pomiar(s_lon, s_lat, lat1, long1);
	unsigned long dawno = czas(s_time);
	check(&tablica[0], s_odleglosc, dawno, qth_odl, secs);
}
fclose(fp);

printf("=^..^= Program uruchomiono =^..^=\n");
}
//
