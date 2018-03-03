<?php


function GreenYellowRedO($number) {

  if ($number < 30) {
    $r = floor(255 * ($number / 30));
    $g = 105;

  } else {
    $r = 105;
    $g = floor(255 * ((30-$number%30) / 30));
  }
  $b = 0;
  return sprintf("%02X%02X%02X",$r,$g,$b);

}


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


function printSDR($fil){
    if (($handle = fopen($fil, "r")) !== FALSE) {
	$line = fgets($handle);
	echo "<table width=380 cellspacing=0><tr><th bgcolor=#FFFFAA>Freq:</th><td bgcolor=#FFFFAA>".$line{2}.$line{3}.$line{4}.".".$line{5}.$line{6}.$line{7}." MHz</td>";
	$line = fgets($handle);
	echo "<th bgcolor=#AAFFAA>PPM:</th><td bgcolor=#AAFFAA>".$line{2}.$line{3}."</td>";
	$line = fgets($handle);
	echo "<th bgcolor=#11FFFF>Gain:</th><td bgcolor=#11FFFF>".$line{2}.$line{3}."</td></tr></table><br>";

	echo "<table>";
	while (($line = fgets($handle)) !== false) {
	    $arr = explode(";",$line);
	    echo "<tr>";
	    $p=ord($line{16});
	    echo "<th>".$line{0}.$line{1}." : ".$line{3}.$line{4}.$line{5}.".".$line{6}.$line{7}.$line{8}." MHz</td><td align=right>".intval($arr[2])."kHz</td><th> (".$p."dB)</th>";
	    for($i=0;$i<16;$i++){
		$p=ord($line{$i+16});
		echo '<td bgcolor="#'.GreenYellowRed($p*1.33).'">&nbsp;</td>';
	    }
	    echo "</tr>";
	}
	echo "</table>";
    }
    else echo "<b>NO SDR</b>";
}

echo "<table width=650 cellpadding=6><tr><th>SDR #1</th><th>SDR #2</th></tr>";
echo "<tr><td valign=top>";
printSDR("/tmp/sdr1.bin");
echo "</td><td valign=top>";
printSDR("/tmp/sdr2.bin");
echo "</td></tr><tr><td><br><br></td><td></td></tr><tr><th>SDR #3</th><th>SDR#4</th></tr><tr><td valign=top>";
printSDR("/tmp/sdr3.bin");
echo "</td><td valign=top>";
printSDR("/tmp/sdr4.bin");
echo "</td></tr></table>";

?>