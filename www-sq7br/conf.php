<?php

if (array_key_exists("config",$_POST)){
    file_put_contents("conf/dxl_config.sh",str_replace("\r\n","\n",$_POST["config"]));
}
if (array_key_exists("restart",$_POST)){
    touch("/tmp/restart_dxl");
}
print("<form method='POST'>");
print("<textarea name='config' cols='80' rows='20'>");
print(file_get_contents("conf/dxl_config.sh"));
print("</textarea>");
print("<input type='submit' name='save_config' value='SAVE'>");
print("<input type='submit' name='restart' value='SAVE & RESTART'>");
print("</form>");
print("<pre>");
@print(file_get_contents("/tmp/restart_log"));
print("</pre>");
?>