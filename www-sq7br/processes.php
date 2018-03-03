<div align="center">
<table class='procesy'>
<tbody class='procesy'>
<tr class='proces'><th> Name </th><th> PID </th></tr>
<?php
include 'config.php';
include 'funkcje.php';
echo "<tr class='proces'>";
procesRunningPrint("rtl_tcp");
echo "</tr>";
echo "<tr class='proces'>";
procesRunningPrint("sdrtst");
echo "</tr>";
echo "<tr class='proces'>";
procesRunningPrint("sondeudp");
echo "</tr>";
echo "<tr class='proces'>";
procesRunningPrint("sondemod");
echo "</tr>";
echo "<tr class='proces'>";
procesRunningPrint("udpgate4");
echo "</tr>";


?>
</tbody>
</table>
</div>
