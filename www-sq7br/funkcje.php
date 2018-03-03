<?php

function getDistance($point1_lat, $point1_long, $point2_lat, $point2_long, $unit = 'km', $decimals = 2) {
        // Calculate the distance in degrees
        $degrees = rad2deg(acos((sin(deg2rad($point1_lat))*sin(deg2rad($point2_lat))) + (cos(deg2rad($point1_lat))*cos(deg2rad($point2_lat))*cos(deg2rad($point1_long-$point2_long)))));#

        // Convert the distance in degrees to the chosen unit (kilometres, miles or nautical miles)
        switch($unit) {
            case 'km':
                $distance = $degrees * 111.13384; // 1 degree = 111.13384 km, based on the average diameter of the Earth (12,$
                break;
            case 'mi':
                $distance = $degrees * 69.05482; // 1 degree = 69.05482 miles, based on the average diameter of the Earth (7,$
                break;
            case 'nmi':
                $distance =  $degrees * 59.97662; // 1 degree = 59.97662 nautic miles, based on the average diameter of the E$
        }
        return round($distance, $decimals);
}

function readCSVFile( $file, $cutOff = 0 ){
#
$csvArray = array();
$row=0;
if (($handle = @fopen($file, "r")) !== FALSE) {
  while (($data = fgetcsv($handle, 1000, ";")) !== FALSE) {
    $num = count($data);
     for( $c = 0; $c < $num; $c++ ) {
        $csvArray[$row][] = $data[$c];
      }
     $row++;
  }
fclose($handle);
if( !empty( $csvArray ) ) {
    return array_splice($csvArray, $cutOff); //cut off the first row (names of the fields)
  } else {
    return false;
  }
 }
}


function procesRunning($program){
exec("pgrep ".$program, $output, $return);
if ($return == 0) return $output;
else return array();
}


function procesRunningPrint($program){
 $pids=procesRunning($program);
 print ("<td class='proces'>".$program."</td>");
 $pidy="";
 foreach ($pids as $lp => $pid)   {
     $pidy=$pidy.$pid.", ";
}
 $pidy=rtrim($pidy,", ");
 print("<td class='proces'>".$pidy."</td>");

}




?>
