<?php
print("<pre>\n");
$sondes=array();
$lines=@file("/tmp/sonde.csv");
if ($lines) {
    foreach($lines as $line) $sondes[]=explode(";",$line);

    //sort by altitude, descending
    usort($sondes, function($a, $b) {
	return $b[3] - $a[3];
    });
    
    //print actual low or lowest
    foreach($sondes as $l) {
	if ( ($l[8]+60 > time()) && (((int)$l[3]) < 3000) ){
	    break;
	}
    }
	    print($l[0]." ".$l[7]."\n");
	    print($l[1]." ".$l[2]."\n");
	    print($l[3]."m ".$l[5]."m/s\n");
}else {
    print("---\n");
    print("---\n");
    print("---\n");
}
    print("\n");
    print("\n");

print("</pre>\n");

?>