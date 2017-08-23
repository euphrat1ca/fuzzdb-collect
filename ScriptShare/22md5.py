#!/usr/bin/env python
# coding=utf-8

#author : phantomer

import string
import sys,urllib,urllib2,re
print "本程序用来简易的转换22位MD5到规范的MD5\n"


name=raw_input("请输入22位MD5：")
if len(name)!=22:
    while(1):
        if name[-1]=="=":
            name[:-1]
        else:
            break
if len(name)!=22:
    print "输入的md5不符合要求"


table1={
    'A':'0','B':'1','C':'2','D':'3','E':'4','F':'5','G':'6','H':'7','I':'8','J':'9','K':'10','L':'11','M':'12','N':'13','O':'14','P':'15','Q':'16','R':'17','S':'18','T':'19','U':'20','V':'21','W':'22','X':'23','Y':'24','Z':'25'
}
table2={
    'a':'26','b':'27','c':'28','d':'29','e':'30','f':'31','g':'32','h':'33','i':'34','j':'35','k':'36','l':'37','m':'38','n':'39','o':'40','p':'41','q':'42','r':'43','s':'44','t':'45','u':'46','v':'47','w':'48','x':'49','y':'50','z':'51'
}
table3={
    '0':'52','1':'53','2':'54','3':'55','4':'56','5':'57','6':'58','7':'59','8':'60','9':'61','+':'62','/':'63'
}
global strings
strings=""
for i in name:
    if i in table1:
        a=bin(int(table1[i])).zfill(8)
        a=a[2:]
    elif i in table2:
        a=bin(int(table2[i])).zfill(8)
        a=a[2:]
    elif i in table3:
        a=bin(int(table3[i])).zfill(8)
        a=a[2:]
    if i==name[-1]:
        a=a[0:2]
    if 'b' in a:
        a=string.replace(a,"b","0")
    strings+=a

liststring = [strings[x:x+8] for x in range(0,len(strings),8)]

global strings2
strings2=""
for i in range(16):
    liststring[i]=hex(int(liststring[i],2))
    a=liststring[i]
    a=a[2:].upper()
    strings2+=a

print "生成的32位md5：",strings2
print "生成的32位md5（全小写）：",strings2.lower()

jiemi=raw_input( "是否需要在线解密？：y？n？")
def jiemionlie():
	MD5=strings2.lower()
	def myaddr():
		site = 'http://md5.my-addr.com/'
		rest = 'md5_decrypt-md5_cracker_online/md5_decoder_tool.php'
		para = urllib.urlencode({'md5':MD5})
		req = urllib2.Request(site+rest)
		fd = urllib2.urlopen(req, para)
		data = fd.read()
		match = re.search('(Hashed string</span>: )(\w+.\w+)', data)
		if match: print '[-] site: %s\t\t\tPassword: %s' %(site, match.group(2))
		else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
	myaddr()

	def victorov():
		try:
			site = 'http://www.victorov.su/'
			para = 'md5/?md5e=&md5d=%s' %MD5
			req = urllib2.Request(site+para)
			req.add_header
			opener = urllib2.urlopen(req)
			data = opener.read()
			match = re.search('(\w+)(</b>)', data)
			if match: print '\n[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(1))
			else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
		except urllib2.URLError,e: print '[+] site: %s \t\t\t seems to be down\n' %site
	victorov()

'''	def md5crack():
		site = 'http://www.md5crack.com/'
		rest = 'crackmd5.php'
		para = urllib.urlencode({'term':MD5})
		req = urllib2.Request(site+rest)
		try:
			fd = urllib2.urlopen(req, para)
			data = fd.read()
			match = re.search('(Found: md5)(..)(\w+.\w+)', data)
			if match: print '[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(3))
			else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
		except urllib2.HTTPError: print '[*] Check your connection\n'
	md5crack()
'''
'''	def rednoize():
		site = 'http://md5.rednoize.com/'
		para = 'p&s=md5&q=%s&_=' %MD5
		req = urllib2.urlopen(site+'?'+para)
		data = req.read()
		if not len(data): print '[-] site: %s\t\t\tPassword: Not found\n' %site
		else: print '[-] site: %s\t\t\tPassword: %s\n' %(site, data)
	rednoize()

	def md5pass():
		site = 'http://www.md5pass.info/'
		para = urllib.urlencode({'hash':MD5, 'get_pass':'Get+Pass'})
		req = urllib2.Request(site)
		fd = urllib2.urlopen(req, para)
		data = fd.read()
		match = re.search('(Password - <b>)(\w+)', data)
		if match: print '[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(2))
		else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
	md5pass()

	def md5decryption():
		site = 'http://md5decryption.com/'
		para = urllib.urlencode({'hash':MD5,'submit':'Decrypt+It!'})
		req = urllib2.Request(site)
		fd = urllib2.urlopen(req, para)
		data = fd.read()
		match = re.search(r'(Decrypted Text: </b>)(.+[^>])(</font><br/><center>)', data)
		if match: print '[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(2))
		else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
	md5decryption()


	def bigtrapeze():
		site = 'http://www.bigtrapeze.com/'
		rest = 'md5/index.php?query=%s' %MD5
		req = urllib2.Request(site+rest)
		req.add_header('User-Agent', 'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.2)\
		Gecko/20100316 AskTbSPC2/3.9.1.14019 Firefox/3.6.2')
		opener = urllib2.build_opener()
		data = opener.open(req).read()
		match = re.search('(=> <strong>)(\w+.\w+)', data)
		if match: print '[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(2))
		else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
	bigtrapeze()

	def cloudcracker():
		site = 'http://www.netmd5crack.com/'
		para = 'cgi-bin/Crack.py?InputHash=%s' %MD5
		req = urllib.urlopen(site+para)
		data = req.read()
		match = re.search(r'<tr><td class="border">[^<]+</td><td class="border">\(?P<hash>[^>]+)</td></tr></tbody></table>', data)
		if match: print '[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(hash))
		else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
	cloudcracker()

	def hashchecker():
		site = 'http://www.hashchecker.com/'
		para = urllib.urlencode({'search_field':MD5, 'Submit':'search'})
		req = urllib2.Request(site)
		fd = urllib2.urlopen(req, para)
		data = fd.read()
		match = re.search('(is <b>)(\w+)', data)
		if match: print '[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(2))
		else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
	hashchecker()

	def hashcracking():
		site = 'http://md5.hashcracking.com/'
		rest = 'search.php'
		para = 'md5=%s' %MD5
		req = urllib2.urlopen(site+rest+'?'+para)
		data = req.read()
		match = re.search('(is)(.)(\w+.\w+)', data)
		if match: print '[-] site: %s\t\t\tPassword: %s\n' %(site, match.group(3))
		else: print '[-] site: %s\t\t\tPassword: Not found\n' %site
	hashcracking()
'''
if jiemi=='y':
    jiemionlie()
else:
    sys.exit()
