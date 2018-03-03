<?php
include 'config.php';
include 'funkcje.php';


$csvData = readCSVFile($sondeCSV);
foreach ($csvData as $key => $row) {
    $sondeId[$key]  = $row['0'];
    //$lat[$key] = $row['8'];
    $freq[$key] = $row[7];
    $last[$key] = $row[8];

}
array_multisort($last, SORT_DESC, $sondeId, SORT_ASC, $csvData);

foreach($csvData as $nrlinii => $row){
  foreach ($row as $key => $value)
    print("$value;");
  print "\n";

};

