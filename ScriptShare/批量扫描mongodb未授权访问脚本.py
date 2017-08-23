
需要 pymongo库
easy_install pymongo
脚本：
import socket
import sys
import pymongo
 
ipcons = []
def Scanner(ip):
    global ipcons
    sk = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sk.settimeout(0.3) 
    try:
        sk.connect((ip,27017))
        ipcons.append(ip)
        sk.close()
    except Exception:
        pass
 
def ip2num(ip):
    ip=[int(x) for x in ip.split('.')]
    return ip[0] <<24 | ip[1]<<16 | ip[2]<<8 |ip[3]
     
def num2ip(num):
    return '%s.%s.%s.%s' %( (num & 0xff000000) >>24,
                                (num & 0x00ff0000) >>16,
                                (num & 0x0000ff00) >>8,
                                num & 0x000000ff )
     
def get_ip(ip):
    start,end = [ip2num(x) for x in ip.split(' ') ]
    return [ num2ip(num) for num in range(start,end+1) if num & 0xff ]
     
startIp = sys.argv[1]
endIp = sys.argv[2]
iplist = get_ip(sys.argv[1]+" "+sys.argv[2])
for i in iplist:
    Scanner(i)
 
def connMon(ip_addr):
    print ' Connect mongodb: ' + ip_addr + ':27017'
    try:
        conn = pymongo.MongoClient(ip_addr,27017,socketTimeoutMS=3000)
        dbname = conn.database_names()
        print "success"
    except Exception as e:
        print "error"
 
print ipcons    
for ipaddr in ipcons:
    connMon(ipaddr)
    print "================="
