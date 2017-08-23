#coding=utf-8

import threading
import requests
import argparse
import time
import re


PORTS = (80,81,82,83,84,85,86,87,88,89,90,443,801,873,881,1900,2601,2604,3128,4440,4848,5001,5275,6379,7001,7002,7003,7005,7008,7033,7080,7090,7778,7901,8000,8001,8002,8003,8010,8011,8020,8080,8081,8082,8083,8084,8085,8086,8087,8088,8089,8090,8199,8443,8886,8888,9000,9043,9060,9080,9090,9200,9437,9440,9956,11211,16082,16907,18100,20000,27017,28017,41080,50000)
PATHS = ('/robots.txt',
         '/admin/',
         '/console/',
         '/uddiexplorer/',
         '/manager/html/',
         '/jmx-console/',
         '/web-console/',
         '/jonasAdmin/',
         '/manager/',
         '/install/',
         '/ibm/console/logon.jsp',
         '/axis2/axis2-admin/',
         '/CFIDE/administrator/index.cfm',
         '/FCKeditor/',
         '/fckeditor/',
         '/fck/',
         '/FCK/',
         '/HFM/',
         '/WEB-INF/',
         '/ckeditor/',
         '/phpMyAdmin/',
         '/Struts2/index.action',
         '/index.action',
         '/phpinfo.php',
         '/info.php',
         '/1.php',
         '/CHANGELOG.txt',
         '/LICENSE.txt',
         '/readme.html',
         '/cgi-bin/',
         '/invoker/',
         '/.svn/',
         '/test/',
         '/CFIDE/',
         '/.htaccess',
         '/.git/'
)

HTML_LOG_TEMPLATE="""
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Bannerscan Report</title>
<style type="text/css">
%s
</style>
</head>
<body>
<center><h2>%s</h2></center>
<div class="report-content">
%s
</div>
</body>
</html>
"""
css = """
body{background-color:#FFF;color:#444;font-family:"Droid Serif",Georgia,"Times New Roman",STHeiti,serif;font-size:100%;}
a{color:#3354AA;text-decoration:none;}
a:hover,a:active{color:#444;}
pre,code{background:#F3F3F0;font-family:Menlo,Monaco,Consolas,"Lucida Console","Courier New",monospace;font-size:.92857em;padding:2px 4px;}
code{color:#B94A48;}
pre{overflow:auto;max-height:400px;padding:8px;}
pre code{color:#444;padding:0;}
h1,h2,h3{font-family:"Helvetica Neue",Helvetica,Arial,sans-serif;}
textarea{resize:vertical;}.report-meta a,.report-content a,.widget a,a{border-bottom:1px solid#EEE;}.report-meta a:hover,.report-content a:hover,.widget a:hover,a{border-bottom-color:transparent;}#header{padding-top:35px;border-bottom:1px solid#EEE;}#logo{color:#333;font-size:2.5em;}.description{color:#999;font-style:italic;margin:.5em 0 0;}.report{border-bottom:1px solid#EEE;padding:15px 0 20px;}.report-title{font-size:1.4em;margin:.83em 0;}.report-meta{margin-top:-.5em;color:#999;font-size:.92857em;padding:0;}.report-meta li{display:inline-block;padding-left:12px;border-left:1px solid#EEE;margin:0 8px 0 0;}.report-meta li:first-child{margin-left:0;padding-left:0;border:none;}.report-content{line-height:1.5;}.report-content hr,hr{margin:2em auto;width:100px;border:1px solid#E9E9E9;border-width:2px 0 0 0;}
"""

ipPattern = "^([1]?\d\d?|2[0-4]\d|25[0-5])\." \
            "([1]?\d\d?|2[0-4]\d|25[0-5])\." \
            "([1]?\d\d?|2[0-4]\d|25[0-5])\." \
            "([1]?\d\d?|2[0-4]\d|25[0-5])$"

iprangePattern = "^([1]?\d\d?|2[0-4]\d|25[0-5])\." \
                 "([1]?\d\d?|2[0-4]\d|25[0-5])\." \
                 "([1]?\d\d?|2[0-4]\d|25[0-5])\." \
                 "([1]?\d\d?|2[0-4]\d|25[0-5])-([1]?\d\d?|2[0-4]\d|25[0-5])$"

ua = "Mozilla/5.0 (X11; U; Linux; en-US) AppleWebKit/527+ (KHTML, like Gecko, Safari/419.3) Arora/0.6"

headers = dict()
result = dict()


class bannerscan(threading.Thread):
    def __init__(self, ip, timeout, headers):
        self.ip = ip
        self.req = requests
        self.timeout = timeout
        self.headers = headers
        self.per = 0
        threading.Thread.__init__(self)

    def run(self):
        result[self.ip] = dict()
        for port in PORTS:
            url_pre = "https://" if port == 443 else "http://"
            site = url_pre + self.ip + ":" + str(port)
            try:
                print ("[*] %s\r" % (site[0:60].ljust(60, " "))),
                resp = requests.head(site,
                                     allow_redirects = False,
                                     timeout=self.timeout,
                                     headers=self.headers
                )
                result[self.ip][port] = dict()

            except Exception, e:
                pass

            else:
                result[self.ip][port]["headers"] = resp.headers
                result[self.ip][port]["available"] = list()

                for path in PATHS:
                    try:
                        url = site + path
                        print ("[*] %s\r" % (url[0:60].ljust(60, " "))),
                        resp = self.req.get(url,
                                            allow_redirects = False,
                                            timeout=self.timeout,
                                            headers=self.headers
                        )

                    except Exception, e:
                        pass
                    else:
                        if resp.status_code in [200, 406, 401, 403, 500]:
                            r = re.findall("<title>([\s\S]+?)</title>", resp.content)
                            title = lambda r : r and r[0] or ""
                            result[self.ip][port]["available"].append((title(r), url, resp.status_code))

def getiplst(host, start=1, end=255):
    iplst = []
    ip_pre = ""
    for pre in host.split('.')[0:3]:
        ip_pre = ip_pre + pre + '.'
    for i in range(start, end):
        iplst.append(ip_pre + str(i))
    return iplst

def retiplst(ip):
    iplst = []
    if ip:
        if re.match(ipPattern, ip):
            print "[*] job: %s \r" % ip
            iplst = getiplst(ip)
            return iplst
        else:
            print "[!] not a valid ip given."
            exit()

def retiprangelst(iprange):
    iplst = []
    if re.match(iprangePattern, iprange):
        ips = re.findall(iprangePattern, iprange)[0]
        ip = ips[0] + "." + ips[1] + "." + ips[2] + "." + "1"
        ipstart = int(ips[3])
        ipend = int(ips[4]) + 1
        print "[*] job: %s.%s - %s" % (ips[0] + "." + ips[1] + "." + ips[2], ipstart, ipend)
        iplst = getiplst(ip, ipstart, ipend)
        return iplst
    else:
        print "[!] not a valid ip range given."
        exit()

def ip2int(s):
    l = [int(i) for i in s.split('.')]
    return (l[0] << 24) | (l[1] << 16) | (l[2] << 8) | l[3]

def log(out, path):
    logcnt = ""
    centerhtml = lambda ips: len(ips)>1  and str(ips[0]) + "  -  " + str(ips[-1]) or str(ips[0])
    titlehtml = lambda x : x and "<strong>" + str(x) + "</strong><br />" or ""
    ips = out.keys()
    ips.sort(lambda x, y: cmp(ip2int(x), ip2int(y)))
    for ip in ips:
        titled = False
        if type(out[ip]) == type(dict()):
            for port in out[ip].keys():
                if not titled:
                    if len(out[ip][port]['headers']):
                        logcnt += "<center><h2>%s</h2></center>" % ip
                        logcnt += "<hr />"
                        titled = True
                logcnt += "PORT: %s <br />" % port
                logcnt += "Response Headers:<pre>"
                for key in out[ip][port]["headers"].keys():
                    logcnt += key + ":" + out[ip][port]["headers"][key] + "\n"
                logcnt += "</pre>"
                for title, url, status_code in out[ip][port]["available"]:
                    logcnt += titlehtml(title) + \
                             "<a href=\"" + url + "\">" + url + " </a> "+ \
                             "Status Code:<code>" + str(status_code) + "</code><br />"
                logcnt += "<br />"
    center = centerhtml(ips)
    logcnt = HTML_LOG_TEMPLATE % ( css, center, logcnt)
    outfile = open(path, "a")
    outfile.write(logcnt)
    outfile.close()

def scan(iplst, timeout, headers, savepath):
    global result
    start = time.time()
    threads = []

    for ip in iplst:
        t = bannerscan(ip,timeout,headers)
        threads.append(t)

    for t in threads:
        t.start()

    for t in threads:
        t.join()

    log(result, savepath)
    result = dict()
    print

def main():
    parser = argparse.ArgumentParser(description='banner scanner. by DM_ http://x0day.me')
    group = parser.add_mutually_exclusive_group()

    group.add_argument('-i',
                        action="store",
                        dest="ip",
    )
    group.add_argument('-r',
                        action="store",
                        dest="iprange",
                        type=str,
    )
    group.add_argument('-f',
                        action="store",
                        dest="ipfile",
                        type=argparse.FileType('r')
    )
    parser.add_argument('-s',
                        action="store",
                        required=True,
                        dest="savepath",
                        type=str,
    )
    parser.add_argument('-t',
                        action="store",
                        required=False,
                        type = int,
                        dest="timeout",
                        default=5
    )

    args = parser.parse_args()
    savepath = args.savepath
    timeout = args.timeout
    iprange = args.iprange
    ipfile = args.ipfile
    ip = args.ip

    headers['user-agent'] = ua

    print "[*] starting at %s" % time.ctime()

    if ip:
        iplst = retiplst(ip)
        scan(iplst, timeout, headers, savepath)

    elif iprange:
        iplst = retiprangelst(iprange)
        scan(iplst, timeout, headers, savepath)

    elif ipfile:
        lines = ipfile.readlines()
        for line in lines:
            if re.match(ipPattern, line):
                iplst = retiplst(line)
                scan(iplst, timeout, headers, savepath)
            elif re.match(iprangePattern, line):
                iplst = retiprangelst(line)
                scan(iplst, timeout, headers, savepath)

    else:
        parser.print_help()
        exit()

if __name__ == '__main__':
    main()

