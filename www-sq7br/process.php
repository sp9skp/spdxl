<?php
function procesRunning($program){
exec("pgrep sdrtst", $output, $return);
if ($return == 0) return $output;
else return array();
}


function procesRunningPrint($program){
 $pids=procesRunning($program);
 print ("<td>".$program."</td>");
 print("<td>");
 foreach ($pids as $lp => $pid)   {
     print($pid."<br>"); 
}
 print("</td>"); 

}

procesRunningPrint("sdrtst");
?>
