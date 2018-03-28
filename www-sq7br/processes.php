<?php
    print("<pre>");
    print("RUNNING:\n");
    $required=array("rtl_tcp","sdrtst","sondeudp","sondemod","udpgate4");
    $procs=array();
    exec("ps -C ".implode(",",$required)." -o cmd --noheaders",$procs);
    foreach($procs as $proc){
	$p=explode(" ",$proc,2);
	$cmd=basename($p[0]);
	print($cmd.":\n".$p[1]."\n");
	$required=array_diff($required,array($cmd));
    }
    print("\nABSENT:\n");
    foreach($required as $cmd){
	print($cmd."\n");
    }
    print("</pre>");

?>