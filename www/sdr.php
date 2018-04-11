<?php
$addn=0;
$save=0;

    $i=0;
    $fil = scandir('CFG');
    foreach ($fil as $key => $value) {
        if (!in_array($value,array(".",".."))){
	    if($p==$i)
		$fil="CFG/".$value;		
		$fname=$value;
        }
        $i++;
    }

if($_POST["dodaj"]=="Dodaj")
    $addn=1;

if($_POST["zapisz"]=="Zapisz")
    $sav=1;

if ($sav){

    $myfile = fopen($fil, "wa+");
    $en=$_POST['e'];
    $f=$_POST['f'];
    $afc=$_POST['afc'];
    $afo=$_POST['afo'];
    $sq=$_POST['sq'];
    $lp=$_POST['lp'];
    $ifw=$_POST['ifw'];
    $com=$_POST['com'];

    fwrite($myfile, "p 5 ".$_POST['ppm']."\n");
    fwrite($myfile, "p 8 ".$_POST['gain']."\n");

    $row=0;
    while($f[$row]!=""){
	$r=0;
	$et="#";
	while($en[$r]!=""){
	    if($row==$en[$r]-1)
		$et="";
	    $r++;
	}

	if($afc[$row]=="") $afc[$row]=10;
    	if($afo[$row]=="") $afo[$row]=0;
	if($sq[$row]=="") $sq[$row]=0;
	if($lp[$row]=="") $lp[$row]=0;
	if($ifw[$row]=="") $ifw[$row]=12000;

	if ($com[$row]!="DEL"){
            $txt=$et."f ".$f[$row]." ".$afc[$row]." ".$afo[$row]." ".$sq[$row]." ".$lp[$row]." ".$ifw[$row]." #".$com[$row]."\n";
	    fwrite($myfile, $txt);
	}
	$row++;
    }
    fclose($myfile);
}

?>


<form  method="post">


<?php


if (($handle = fopen($fil, "r")) !== FALSE) {
  while (($data = fgetcsv($handle, 1000, " ")) !== FALSE) {
    if ($data[0]=='p' && $data[1]=='5')
	$ppm=$data[2];
    if ($data[0]=='p' && $data[1]=='8')
	$gain=$data[2];
  }
}
fclose($handle);

echo '<table><tr><th>PPM:</th><td><input type="text" name="ppm" maxlength="2" size="1" value='.$ppm.'></td></tr>';
echo '<tr><th>GAIN:</th><td><input type="text" name="gain" maxlength="2" size="1" value='.$gain.'></td></tr></table>';
?>

<table width="700" border="1" cellpadding="3" cellspacing="0">
  <tr>
    <th>No</th>
    <th>En.</th>
    <th>Mod.</th>
    <th>Frequency</th>
    <th>AFC [kHz]</th>
    <th>AFC offs [Hz]</th>
    <th>Squelch</th>
    <th>Lowpass</th>
    <th>IF width</th>
    <th>Comment</th>
  </tr>

<?php
$row = 1;
if (($handle = fopen($fil, "r")) !== FALSE) {
  while (($data = fgetcsv($handle, 1000, " ")) !== FALSE) {
    $num = count($data);
    if ($data[0]=='f' || $data[0]=='#f'){
	if($data[0]=='f') 
	    $enf=" checked ";
	else
	    $enf="";
        echo '<tr><td>'.$row.'</td><td><input type="checkbox" name="e[]" value="'.$row.'"'.$enf.'><br></td><td>FM</td><td><input type="text" name="f[]" maxlength=7 size=4 value="'.$data[1].'"></td>';
	echo '<td><input type="text" name="afc[]" maxlength="2" size="1" value='.$data[2].'></td><td><input type="text" name="afo[]" maxlength="6" size="4" value='.$data[3].'></td>';
	echo '<td><input type="text" name="sq[]" maxlength="3" size="2" value='.$data[4].'></td><td><input type="text" name="lp[]" maxlength="3" size="3" value='.$data[5].'></td>';
	echo '<td><input type="text" name="ifw[]" maxlength="6" size="4" value='.$data[6].'></td><td><input type="text" name="com[]" maxlength="25" size="10" value='.substr($data[7],1).'></td></tr>';
	$row++;
    }
  }
  fclose($handle);
  copy($fil,'/tmp/'.$fname);
}
if($addn){
        echo '<tr bgcolor=#FFAAAA><td>'.$row.'</td><td><input type="checkbox" name="e[]" value="e0"><br></td><td>FM</td><td><input type="text" name="f[]" maxlength=7 size=4 value="'.$_POST["nqrg"].'"></td>';
	echo '<td><input type="text" name="afc[]" maxlength="2" size="1" value=10></td><td><input type="text" name="afo[]" maxlength="6" size="4" value=0></td>';
	echo '<td><input type="text" name="sq[]" maxlength="3" size="2" value=0></td><td><input type="text" name="lp[]" maxlength="3" size="3" value=0></td>';
	echo '<td><input type="text" name="ifw[]" maxlength="6" size="4" value=12000></td><td><input type="text" name="com[]" maxlength="25" size="10" value="nowa QRG"></td></tr>';
}


?>
</table>

<br>
<table style="width:100%">
<tr><td>
<input type="text" name="nqrg" value="403.000"><input name="dodaj" type="submit" value="Dodaj" >
</td><td align="right">
<input type="submit" name="zapisz" value="Zapisz">
</td></tr>
</table>
</form>
