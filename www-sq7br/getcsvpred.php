<?php
include 'config.php';
include 'funkcje.php';


$csvData = readCSVFile($predictCSV);
foreach ($csvData as $key => $row) {
    $pointType[$key]  = $row['0'];
    //$lat[$key] = $row['8'];
    $spa[$key] = $row[2];
    $last[$key] = $row[3];

}
array_multisort($last, SORT_DESC, $pointType, SORT_ASC, $csvData);

foreach($csvData as $nrlinii => $row){
  foreach ($row as $key => $value)
    print("$value;");
  print "\n";

};

