<html>
<head>
<meta http-equiv="Content-Type" content="text/html;">
<title>skpSCP</title>
<style type="text/css"><!-- body {background-color: #CCCCCC;} a:link {color: #000000;} a:hover {text-decoration: underline;} --> </style>
</head>
<body>
<div align="center">
<table width="700" border="1" cellpadding="3" cellspacing="0">
<tr><th> Name </th><th> Database </th><th> Latitude </th> <th> Longitude </th> <th> Altitude </th><th>Speed [m/s]/[km/h]</th> <th> Climb </th> <th> Dir </th><th> Freq </th><th>Lat frame </th><th>Dist[km]</th></tr>
<?php

include 'SET.php';

//echo $lat;

function getDistance($point1_lat, $point1_long, $point2_lat, $point2_long, $unit = 'km', $decimals = 2) {
        // Calculate the distance in degrees
        $degrees = rad2deg(acos((sin(deg2rad($point1_lat))*sin(deg2rad($point2_lat))) + (cos(deg2rad($point1_lat))*cos(deg2rad($point2_lat))*cos(deg2rad($point1_long-$point2_long)))));

        // Convert the distance in degrees to the chosen unit (kilometres, miles or nautical miles)
        switch($unit) {
            case 'km':
                $distance = $degrees * 111.13384; // 1 degree = 111.13384 km, based on the average diameter of the Earth (12,735 km)
                break;
            case 'mi':
                $distance = $degrees * 69.05482; // 1 degree = 69.05482 miles, based on the average diameter of the Earth (7,913.1 miles)
                break;
            case 'nmi':
                $distance =  $degrees * 59.97662; // 1 degree = 59.97662 nautic miles, based on the average diameter of the Earth (6,876.3 nautical miles)
        }
        return round($distance, $decimals);
}



$row = 1;
$cnt=0;
if (($handle = fopen("/tmp/sonde.csv", "r")) !== FALSE) {
  while (($data = fgetcsv($handle, 1000, ";")) !== FALSE) {
    $num = count($data);
    if($cnt)
	echo "</tr>";
    echo "<tr>";
    $row++;
    $speed=$data[4]." / ".intval($data[4]*3.6);
    $data[4]=$speed;
    echo"<td align=center><a href=\"https://www.google.pl/maps/place/".$data[1].",".$data[2]."\" target=\"_blank\">".$data[0]."</a></td><td> <a target=\"_blank\" href=\"https://skp.wodzislaw.pl/sondy/sinfo.php?n=".$data[0]."\">SKP</a>/".
	"<a target=\"_blank\" href=\"https://aprs.fi/#!call=a%2F".$data[0]."&timerange=3600&tail=3600\">APRS</a>/<a target=\"_blank\" href=\"https://radiosondy.info/sonde_archive.php?sondenumber=".$data[0]."\">KXY</a></td>";
    for ($c=1; $c < $num-1; $c++) {
        echo "<td align=center>" . $data[$c] . "</td>";
    }
    echo "<td>" . date("d/G:i:s",$data[$num-1]) . "</td>";
    echo "<td>" . getDistance($lat,$lon,$data[1],$data[2]) . "</td>";
    
  }
  fclose($handle);
}


?>
</table>
</div></body></html>