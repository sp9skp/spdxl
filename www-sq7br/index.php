<html>
<head>
<meta http-equiv="Content-Type" content="text/html;">
<?php
$realIP = file_get_contents("http://ipecho.net/plain");
$hostname = gethostbyaddr($realIP);
print("<title>spdxl ".$realIP."</title>");

?>
<link rel="stylesheet" type="text/css" href="styl.css">

<script>
<?php
    $p=$_GET["p"];

    if($p=='0' || $p=='') echo "var page='sondy.php'; ";
    if($p=='s') echo "var page='sondy.php';";
    if($p=='A') echo "var page='wf.php';";
    if($p=='PS') echo "var page='processes.php';";
    if($p=='D') echo "var page='display.php';";
?>

function getData(){
    nocache = "&nocache="+ Math.random() * 1000000;
    var request = new XMLHttpRequest();
    request.onreadystatechange = function(){
	if (this.readyState == 4){
	    if (this.status == 200){
		if (this.responseText != null){
		    document.getElementById("switch_txt").innerHTML = this.responseText;
		}
	    }
	}
    }
    request.open("GET", page, true);
    request.send(null);
    setTimeout('getData()', 1000);
}


</script>

</head>
<body onload="getData()">
<div align="center">
<table width="800" border="0" cellpadding="4" cellspacing="0">
<tr><td colspan="2" bgcolor="#66CCFF">
<h1 align="center">
<?php 
 print("Sonde Control Panel");
 //print("<br>");
 print(" ".$realIP); 
 print(" ".$hostname);
?>
</h1>
</td></tr>
<tr><td valign="top" width="96" bgcolor="#006666">
<a href='mapa.php' target='_blank' style="text-decoration:none" > <input style="width:100px" type="button" value="MAPA" > </a>
<br><br>
<input style="width:100px" type="button" onClick="location.href='index.php?p=s';" value="SONDY">


<br>
<input style="width:100px" type="button" onClick="location.href='index.php?p=PS';" value="PROCESY">
<br>
<input style="width:100px" type="button" onClick="location.href='index.php?p=A';" value="SDR WF">
<br><br>
<input style="width:100px" type="button" onClick="location.href='index.php?p=C';" value="CONFIG">
<input style="width:100px" type="button" onClick="location.href='index.php?p=SDRTST1';" value="SDR #1 freq.">
<input style="width:100px" type="button" onClick="location.href='index.php?p=SDRTST2';" value="SDR #2 freq.">
<input style="width:100px" type="button" onClick="location.href='index.php?p=SDRTST3';" value="SDR #3 freq.">
<input style="width:100px" type="button" onClick="location.href='index.php?p=SDRTST4';" value="SDR #4 freq.">
<input style="width:100px" type="button" onClick="location.href='index.php?p=D';" value="OLED">


<br><br>
</td>
<td width="704" bgcolor="#CCCCFF">
<?php

if ($p=="C") require("conf.php");

if($p=='0' || $p=='' || $p=='s' || $p=='D' ) echo "<p id=\"switch_txt\"></p>";
if( substr($p,0,6)=="SDRTST"  ) include 'sdr.php'; 
if($p=='A') echo "<p id=\"switch_txt\"></p>";
if($p=='PS') echo "<p id=\"switch_txt\"></p>";

// include 'wf.php';
?>


</td></tr><tr><td colspan="2" bgcolor="#66CCFF">
<div align="center">
&copy; <a href="http://skp.wodzislaw.pl">SP9SKP</a>
mod by <a href="http://sq7br.pzk.pl">SQ7BR</a>
</div>
</td></tr></table></div></body></html>
