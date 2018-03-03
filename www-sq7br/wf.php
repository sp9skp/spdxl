<?php
include 'config.php';

#function GreenYellowRedO($number) {

#  if ($number < 30) {
#    $r = floor(255 * ($number / 30));
#    $g = 105;

#  } else {
#    $r = 105;
#    $g = floor(255 * ((30-$number%30) / 30));
#  }
#  $b = 0;
#  return sprintf("%02X%02X%02X",$r,$g,$b);
#
#}


function GreenYellowRed($number) {
  if($number>99) $number=99;
  if ($number < 50) {
    // blue to green
    $g = floor(255 * ($number / 50));
    $b = 120;
    $r = 0;

  } else {
    // green to yelow
    $r = 255;
    $g = floor(255 * ((50-$number%50) / 50));
    $b = 0;
  }
  return sprintf("%02X%02X%02X",$r,$g,$b);
}

function printFreqPpmGain($freq,$ppm,$gain){
 echo "<table class='fpg'><tr>";
 echo "<th class='freq'>Freq:</th><td class='freq'>".$freq."</td><td class='freq'>MHz</td>";
 echo "<th class='ppm'>PPM:</th><td class='ppm'>".$ppm."</td>";
 echo "<th class='gain'>Gain:</th><td class='gain'>".$gain."</td>";
 echo "</tr></table>"; 
}

function printBar($db){
for($i=0;$i<$GLOBALS["wfDbSize"];$i++)
    echo '<td bgcolor="#'.GreenYellowRed(ord($db{$i})*1.33).'">&nbsp;</td>';
}

function printListenFreq($position,$freq,$dbLevel,$afc){
  $afcT=""; if ( is_numeric($afc) ) $afcT=" ".$afc." Hz";
  echo "<th class=''>".$position." : ".$freq." MHz".$afcT." (".$dbLevel." dB)</th>";
}

function printSDR($fil){
    if (($handle = @fopen($fil, "r")) !== FALSE) {
	$line = fgets($handle);
	$freq=$line{2}.$line{3}.$line{4}.".".$line{5}.$line{6}.$line{7};
        $line = fgets($handle);
        $ppm=$line{2}.$line{3};
        $line = fgets($handle);
        $gain=$line{2}.$line{3};
        printFreqPpmGain($freq,$ppm,$gain);
        echo "<br>";
	while (($line = fgets($handle)) !== false) {
	    $values=explode(";",$line);
	    echo "<table class='waterfall'><tr>";
	    $p=ord($line{10});
            $position=$values[$GLOBALS["wfMapping"]["lp"]-1];
            $freq=$values[$GLOBALS["wfMapping"]["freq"]-1];
            $freq=substr($freq,0,3).".".substr($freq,3,7);
            $afc=$values[$GLOBALS["wfMapping"]["afc"]-1];
            $db=$values[$GLOBALS["wfMapping"]["db"]-1];
            $dbLevel=ord($db{0});
	    printListenFreq($position,$freq,$dbLevel,$afc);
	    printBar($db);
	    echo "</tr></table>";
	}
    }
    else echo "<b>NO SDR</b>";
}

echo "<table class='sdrWaterfalls'><tbody class='sdrWaterfalls'";
foreach ($sdrWTF as $i => $value) {
    echo "<tr class='sdrWaterfall'><th class='sdrWaterfallId'>$i</th><td class='sdrWaterfallCell'>";
    printSDR($value);
    echo "</td></tr>";
}
echo "</tbody></table>";
?>
