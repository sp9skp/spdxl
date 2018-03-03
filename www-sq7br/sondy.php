<div align="center">
<table class='sondy'>
<tbody class='sondy'>
<tr class='sonda'><th> Name </th><th> Latitude </th> <th> Longitude </th> <th> Altitude </th><th>Speed km/h(m/s)</th> <th> Climb </th> <th> Dir </th><th> Freq </th><th>Lat frame </th><th>Dist[km]</th><th> links </th></tr>
<?php
include 'config.php';
include 'funkcje.php';

function printLinks($sondeId,$latitude,$longitude,$linksFormat){
   
    echo"<td class='sondeMap'>";

    foreach ($linksFormat as $key => $linkFormat){
      echo "<a target=\"_blank\" href='". sprintf($linkFormat,$sondeId,$latitude,$longitude)."'>".$key."</a>";
      echo "&nbsp";
    }

   echo "</td>";
}

function printSonde($sondeId,$latitude,$longitude,$alt,$speed,$climb,$dir,$freq,$timeFr,$dist){
    echo "<td class='sondeId'>".$sondeId."</td>"; 
    echo "<td class='latitude'>".$latitude."</td>";
    echo "<td class='longitude'>".$longitude."</td>";
    if ($alt<$GLOBALS["altLimits"]["alert"]) {$idAlt="alert";
    }elseif ($alt<$GLOBALS["altLimits"]["warn"]) {$idAlt="warning";
    }else $idAlt="off";
    echo "<td class='altitude' id='".$idAlt."'>".$alt."</td>";
    echo "<td class='speed'>".$speed."</td>";
    if ($climb<=0) {$idClimb="alert";}
    else $idClimb="off";
    echo "<td class='climb' id='".$idClimb."'>".$climb."</td>";
    echo "<td class='dir' >".$dir."</td>";
    echo "<td class='freq'>".$freq."</td>";
    $tdif=time()-$timeFr;
    if ( $tdif <180*60 )  {$idTimeFr="alert"; }
    else $idTimeFr="off";
    
    echo "<td class='timeSonde' id='".$idTimeFr."'>" . date("d G:i:s",$timeFr) . "</td>";

    if ($dist<$GLOBALS["rangeLimits"]["alert"]) {$idDist="alert";}
    elseif ($dist<$GLOBALS["rangeLimits"]["warn"]) {$idDist="warning";}
    else $idDist="off";
    echo "<td class='distance' id='".$idDist."' name='".$idDist."'>" .$dist. "</td>";

}

$csvData = readCSVFile($sondeCSV);
foreach ($csvData as $key => $row) {
    $sondeId[$key]  = $row['0'];
    //$lat[$key] = $row['8'];
    $freq[$key] = $row[7];
    $last[$key] = $row[8];

}
array_multisort($last, SORT_DESC, $sondeId, SORT_ASC, $csvData);


echo "<tr class='sonda' >";
   foreach($csvData as $key => $data) { 
   // $num=count($data);
   
    $speed="".intval($data[4]*3.6)." (".$data[4].")";
    $data[4]=$speed;
   
    $sondeId=$data[0];
    $latitude=$data[1];
    $longitude=$data[2];
    $altitude=$data[3];
    $speed=$data[4];
    $climb=$data[5];
    $dir=$data[6];
    $freq=$data[7];
    $czas=$data[8];
    $dist=getDistance($lat,$lon,$latitude,$longitude);
    printSonde($sondeId,$latitude,$longitude,$altitude,$speed,$climb,$dir,$freq,$czas,$dist);
    printLinks($sondeId,$latitude,$longitude,$linksFormat);
  
   echo "</tr>";    

}


?>
</tbody>
</table>
</div>
