<?php

if (array_key_exists("config",$_POST)){
    file_put_contents("conf/dxl_config.sh",str_replace("\r\n","\n",$_POST["config"]));
}

print("<form method='POST'>");
print("<textarea name='config' cols='80' rows='20'>");
print(file_get_contents("conf/dxl_config.sh"));
print("</textarea>");
print("<input type='submit' name='save_config' value='SAVE'>");
print("</form>");
?>