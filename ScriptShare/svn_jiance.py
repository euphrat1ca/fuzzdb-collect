#!/usr/bin/env python
# coding=utf-8

#author:phantomer
#github:https://github.com/rootphantomer
#weibo:http://weibo.com/527819757

import requests

"'此工具是用来查看是否有svn源码泄露'"



url=raw_input("输入url:")

default_url="/.svn/entries"

try:
    if url[-1]=="/":
        url=url[:-1]

    r=requests.get(url,params=default_url)

    if r.status_code==200:
        print "存在该目录，可能有漏洞"

except:
    print "无漏洞"
