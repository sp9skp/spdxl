<html>
<head>
<meta http-equiv="Content-Type" content="text/html;">
<title>skpSCP</title>
<style type="text/css"><!-- body {background-color: #CCCCCC;} a:link {color: #000000;} a:hover {text-decoration: underline;} --> </style>
<script>
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
    request.open("GET", "sondy.php", true);
    request.send(null);
    setTimeout('getData()', 2000);
}
function getDataWf(){
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
    request.open("GET", "wf.php", true);
    request.send(null);
    setTimeout('getDataWf()', 1000);
}

</script>

</head>
<body 
<?php
    $p=$_GET["p"];

    if($p=='0' || $p=='') echo " onload=\"getData()\" ";
    if($p=='A') echo " onload=\"getDataWf()\" ";
?>
>
<div align="center">
<table width="900" border="0" cellpadding="4" cellspacing="0">
<tr><td colspan="2" bgcolor="#66CCFF"><h1 align="center">skpSondeControlPanel</h1></td></tr>
<tr><td valign="top" width="96" bgcolor="#006666">
<input style="width:120px" type="button" onClick="location.href='index.php';" value="SONDY">
<br><br>
<input style="width:120px" type="button" onClick="location.href='index.php?p=A';" value="SDR WF">
<br><br>

<?php
    $i=0;
    $fil = scandir('CFG');
    foreach ($fil as $key => $value) { 
	if (!in_array($value,array(".",".."))){ 
	    echo '<input style="width:120px" type="button" onClick="location.href=\'index.php?p='.$i.'\';" value="CFG: '.$value.' ">';
	}
	$i++;
    }

?>


<br><br>
</td>
<td width="704" bgcolor="#CCCCFF">
<?php


if($p=='0' || $p=='') echo "<p id=\"switch_txt\"></p>";
if($p>1) include 'sdr.php'; 
if($p=='A') echo "<p id=\"switch_txt\"></p>";

?>


</td></tr><tr><td colspan="2" bgcolor="#66CCFF"><div align="center">&copy; <a href="http://skp.wodzislaw.pl">SP9SKP</a></div></td></tr></table></div></body></html>