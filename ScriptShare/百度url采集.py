#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author: Lcy
# @Date:   2016-07-13 16:04:29
# @Last Modified by:   Lcy
# @Last Modified time: 2016-07-14 10:22:23
import requests
import logging
from bs4 import BeautifulSoup
from urllib import quote
 
def BaiduSpider(keyword):
    retVal = [] 
    paths = []
    headers = {
        "User-Agent":" Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.94 Safari/537.36"
    }
    gpage = 1
    while True: 
        logging.info("using search result page #%d" % gpage) 
        url = "https://m.baidu.com/s?"
        url += "word=%s&" % quote(keyword) 
        url += "pn=%d" % gpage
        print url
        try:
            r = requests.get(url,headers=headers)
            if r.text.find("class=\"new-nextpage\"") == -1 and r.text.find('class="new-nextpage-only"') == -1:
                break
            soup = BeautifulSoup(r.text,"html.parser")
            div = soup.find_all(tpl='www_normal')
            for l in div:
                url = l.get('data-log', '')
                a = eval(url)
                paths.append(a['mu'])
        except KeyboardInterrupt,e:
            retVal = set(paths)
            print retVal
            return retVal
        except:
            pass
        gpage = gpage + 10
    retVal = set(paths)
    print retVal
    return retVal
 
if __name__ == "__main__":
    keyword = raw_input(u"输入关键字:".encode('gbk'))
    url = BaiduSpider(keyword)
    f = open("url.txt","a")
    for line in url:
        f.write(line+"\r\n")
    f.close()
