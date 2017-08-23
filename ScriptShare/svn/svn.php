#!/usr/bin/php -q

<?php

/**

 * 本脚本用于下载.svn目录未作权限限制的并且svn版本小于1.7的网站源码.

 * 请用php5.3+来运行本脚本.想支持更低版本,请自行修改源码.不必通知我.

 * 作者：小雨@乌云

 * http://蛋疼.com

 */

#错误报告级别,只报告错误

error_reporting(E_ERROR);

#要显示错误

ini_set('display_errors','On');

define('VERSION', '1.0');

ini_set('user_agent','svn_clone(svn_clone v'.VERSION.'; by 小雨@乌云 email:z@zt.vc; http://蛋疼.com)');

#缓存目录，最好放在tmpfs上，我没做过缓存期的设置,所以想真正重新抓一次就必须手工删缓存目录

define('CACHE_DIR', '/tmp/cache');

#代码要保存到的路径，不同域名会自动分目录存放的

define('DATA_DIR',  '/data/src');

#调试信息级别

define('NONE',    0);#无条件报告

define('ERROR',   1);#错误

define('WARNING', 2);#警告

define('ALL',     3);#全部

define('EGGACHE', 4);#蛋疼

#获取参数

$opts = getopt('u:chv',array('url:','color','help','verbose'));

#获取传入的URL地址

$url = $opts['url']?:($opts['u']?:null);

#是否显示帮助

$help = isset($opts['h'])+isset($opts['help']);

#是否使用颜色

define('USECOLOR', isset($opts['c'])+isset($opts['color']));

#调试信息级别,v越多越详细,最多接受3个v, 函数内用，懒得写global,定义成常量吧.

define('VERBOSE', count($opts['v'])+count($opts['verbose']));

#本程序的名字,额，我不知道这个写法是否兼容别的shell.反正bash下用它判断是没错的

$cmd = basename($_SERVER['_'])=='php'?'php '.$_SERVER['PHP_SELF']:$_SERVER['_'];

if($help or !$url) {

    die("Usage:\t$cmd option [url]\n".

        "\t-u  --url\turl\t您想要通过svn克隆的网站url\n".

        "\t-c  --color\t\t使用控制台色彩输出\n".

        "\t-v  --verbose\t\t打印更多的详细信息,v越多越详细\n".

        "\t-h  --help\t\t本帮助信息\n".

        "Examples:\n".

        "\t$cmd -u http://localhost\n".

        "\t$cmd -u http://localhost -cvv\n".

        "\t$cmd -vu http://localhost\n".

        "\t$cmd -cvvu http://localhost\n".

        "\t$cmd --url http://localhost --color --verbose --verbose --verbose 有人勤奋到使用这种格式咩?! Orz.\n"

    );

}



#我真是蛋疼...写这行干啥呢...

debug("蛋疼是一种病,要淡定,不要蛋疼...\n", EGGACHE); 

svn_clone($url);





#本程序的主函数

function svn_clone($url) {

    #去除多余的url结尾多余的斜杠

    $url=trim($url,'/');

    $entries_url = $url.'/.svn/entries';

    $content = get($entries_url);

    if(!$content) {

        return debug("$url 不是一个合法的svn工作副本!\n", ERROR);

    } elseif(strlen($content)<10) {

        return debug("某个东西太短了,需要蓝色小药丸么？\n", ERROR);  

    }

    #匹配出entries中的文件和目录名

    preg_match_all('/\f\n([^\n]+?)\s(\w+)\s/s', $content, $m) or debug("$entries_url 不包含文件或子目录\n", WARNING);

    $files = array_combine($m[1], $m[2]);

    foreach($files as $file=>$type) {

        if($type=='dir') {

            debug(">>> 进入 $file 目录\n", ALL);

            svn_clone($url.'/'.$file); 

            debug("<<< 退出 $file 目录\n", ALL);

        } elseif($type=='file') {

            debug("*** 下载 $file 文件\n", ALL);

            fetch($url.'/.svn/text-base/'.$file.'.svn-base');

        }

    }

}



#抓取并保存

function fetch($text_base){

    put($text_base, get($text_base));

}



#带缓存的抓取

function get($url) {

    $file = CACHE_DIR.'/'.chunk_split(substr(md5($url),0,6),2,'/').urlencode($url);

    $dir = dirname($file);

    if(!is_dir($dir)) {

        mkdir($dir,0777,true);

    }

    if(!file_exists($file)) {

        $content = file_get_contents($url) or debug("读取 {$url} 内容为空\n", WARNING);

        if($content)

        {

            file_put_contents($file, $content) or debug("写入 {$file} 内容为空\n", WARNING);

        }

    } else {

        $content = file_get_contents($file) or debug("读缓存 {$file} 内容为空\n", WARNING);

    }

    return $content;

}



#保存到数据目录

function put($url, $content='')

{

    $file = DATA_DIR.substr(strchr($url,'://'),2);

    $dir  = dirname(dirname(dirname($file)));

    $file = basename($file,'.svn-base');

    #看看你那什么有多长?

    $len  = strlen($content);

    if(!is_dir($dir)) {

        mkdir($dir,0777,true);

    }

    debug("写入 $file 到 $dir ($len bytes)\n", ALL);

    file_put_contents($dir.'/'.$file, $content) or debug("写入 {$file} 内容为空\n", WARNING);

}



#打印调试信息

function debug($msg, $level=0) {

    #颜色定义 0:灰, 1:红, 2:绿, 3:黄, 4:蓝, 5:粉, 6:青, 7:白

    static $colors = array(NONE=>0, ERROR=>1, WARNING=>2, ALL=>3, EGGACHE=>4);

    VERBOSE>=$level && (USECOLOR?printf("\033[1;3{$colors[$level]}m$msg\033[m", $color, $msg):print $msg);

}
