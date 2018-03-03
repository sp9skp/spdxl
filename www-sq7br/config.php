<?php
//polozenie stacji monitorujacej 
// mierzone sa od tej pozycji odleglosci
// mapa centruje sie na tej pozycji
$lat=51.3483;
$lon=19.8733;

//polozenie plikow czestotliwosciami sdr dla sdrtst
$sdrtstCFG=array(
                "SDRTST1" => "/tmp/sdrtst0.cfg"
               ,"SDRTST2" => "/tmp/sdrtst1.cfg"
               ,"SDRTST3" => "/tmp/sdrtst2.cfg"
               ,"SDRTST4" => "/tmp/sdrtst3.cfg"
              );

//polozenie plikow z waterfallem z sdrtst
$sdrWTF=array(
              "SDR:1" => "/tmp/sdr1.bin"
             ,"SDR:2" => "/tmp/sdr2.bin"
             ,"SDR:3" => "/tmp/sdr3.bin"
             ,"SDR:4" => "/tmp/sdr4.bin"
             );

//polozenie plik z wykazem sond z sondemonitora
$sondeCSV="/tmp/sonde.csv";

//plik z predictem burst i land
$predictCSV="/tmp/predict";


//format linkow do stron
$linksFormat=array(
                    "geo"       => 'geo:0,0?q=%2$s,%3$s(%1$s)'
                   ,"KXY"       => 'https:\/\/radiosondy.info/sonde_archive.php?sondenumber=%1$s'
                   ,"SKP"       => 'https:\/\/skp.wodzislaw.pl/sondy/sinfo.php?n=%1$s'
                   ,"aprs.fi"   => 'https:\/\/aprs.fi/#!call=%1$s%%2Ca%%2F%1$s_'
                   ,"Google"    => 'https:\/\/www.google.pl/maps/place/%2$s,%3$s'
                   ,"OSM"       => 'https:\/\/www.openstreetmap.org/?map=13&mlat=%2$s&mlon=%3$s?m'
                  );

//wartosci dla alertu odleglosci
$rangeLimits=array(
                   "warn"  => 150
                  ,"alert" => 60
                  );

//wartosci dla alertu wysokosci
$altLimits=array(
                 "warn"  => 8000
                ,"alert" => 2000
                );


//pozycje w plikach z waterfallem
$wfMapping=array(
                 "lp"   => 1
                ,"freq" => 2
                ,"afc"  => 3 //dla starej wersji ustaw na ciag 'N/A' 
                ,"db"   => 4 //dla starej wersji ustaw na 3
                );

//liczba pozycji z wartoscia sily sygnalu w plikach z waterfallem
$wfDbSize=16;


?>
