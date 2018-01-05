<html>
<head>
<meta http-equiv="Content-Type" content="text/html;">
<title>skpSCP</title>
<style type="text/css"><!-- body {background-color: #CCCCCC;} a:link {color: #000000;} a:hover {text-decoration: underline;} --> </style>
</head>
<body>
<div align="center">
<table width="700" border="1" cellpadding="5" cellspacing="0">
<tr><th> Name </th><th> Latitude </th> <th> Longitude </th> <th> Altitude </th> <th> Speed </th> <th> Climb </th> <th> Dir </th><th> Freq </th></tr>
<?php

$row = 1;
$cnt=0;
if (($handle = fopen("/tmp/sonde.csv", "r")) !== FALSE) {
  while (($data = fgetcsv($handle, 1000, ";")) !== FALSE) {
    $num = count($data);
    if($cnt)
	echo "</tr>";
    echo "<tr>";
    $row++;
    echo"<td><a href=\"https://www.google.pl/maps/place/".$data[1].",".$data[2]."\">".$data[0]."</a> &nbsp;<a target=\"_blank\" href=\"https://skp.wodzislaw.pl/sondy/sinfo.php?n=".$data[0]."\">(SKP)</a></td>";
    for ($c=1; $c < $num; $c++) {
        echo "<td>" . $data[$c] . "</td>\n";
    }
  }
  fclose($handle);
}


?>
</table>
</div></body></html>