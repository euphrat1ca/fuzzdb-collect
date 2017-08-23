# -*- coding: cp936 -*-
from socket import *
from threading import Thread
from Queue import Queue

import signal
import sys,os
import time
import string

'''

Python 端口扫描工具  Code by msx2009

'''

setdefaulttimeout(2)#设置超时时间2s 比较合适

def scan(ip,port):
    s=socket(AF_INET,SOCK_STREAM)
    result=s.connect_ex((ip,port))
    if(result==0):
        print '%s \t %d \t Open' % (ip,port) 
        s.close()
    
        
class Worker(Thread):
    def __init__(self,taskQueue):
        Thread.__init__(self)
        self.setDaemon(True)
        self.taskQueue=taskQueue
        self.start()
    def run(self):
        while 1:
            try:
                callable,args,kwds=self.taskQueue.get(block=False)
                callable(*args,**kwds)
            except:
                break
            
class ThreadPool:
    def __init__(self,ip):
        self.threads=[]
        self.taskQueue=Queue()#创建一个队列
        self.threadNum=num_thread #线程数
        self.__create_taskqueue(ip)
        self.__create_threadpool(self.threadNum)
    def __create_taskqueue(self,ip):
        if is_iparry:
            for i in ip_arry:
                self.add_task(scan,ip,string.atoi(i)) #增加任务到队列中
        else:
            for i in range(start_port,end_port):
                self.add_task(scan,ip,i) #增加任务到队列中
                
    def __create_threadpool(self,threadNum):
        for i in range(threadNum):
            thread=Worker(self.taskQueue)
            self.threads.append(thread)#增加到线程池中
    def add_task(self,callable,*args,**kwds):
        self.taskQueue.put((callable,args,kwds))
    '''不用此方法，因为会阻塞ctrl+c命令
    def waitfor_complete(self):
        while len(self.threads):
            thread=self.threads.pop()
            thread.join()#等待线程结束
            if thread.isAlive() and not self.taskQueue.empty(): #再次判断线程是否结束
                self.threads.append(thread)
        print 'scaning is over!'
    '''
    def new_complete(self):
        #global alive
        while 1:
            time.sleep(0.1) #防止cpu占用100%
            alive = False
            for i in range(num_thread):
                alive = alive or self.threads[i].isAlive()
            if not alive:
                break
           # catch Error:
        
        
#信号处理函数
def handler(signum, frame):
    global is_exit
    print "CTRL+C Is Pressed"
    sys.exit(0)

def usage():
    print "%s StartIP[-EndIP] Ports Threads" % os.path.basename(sys.argv[0])
    print "%s 10.10.10.1 21,80 10" % os.path.basename(sys.argv[0])
    print "%s 10.10.10.1 1-65535 10" % os.path.basename(sys.argv[0])
    print "%s 10.10.10.1-10.10.10.254 1-65535 10" % os.path.basename(sys.argv[0])
    print "%s 10.10.10.1-10.10.10.254 21,80,1433,3389 10" % os.path.basename(sys.argv[0])
    sys.exit(0)
if __name__=='__main__':
    if len(sys.argv) !=4:
        usage()
        
    #当端口参数为21,80这种形式时，保存到数组
    is_iparry = False
    ip_arry = []
        
    #信号量，用于捕获Ctrl+C
    signal.signal(signal.SIGINT, handler)
    signal.signal(signal.SIGTERM, handler)

    try:
        #处理ip参数
        if sys.argv[1].find("-") != -1:
            start_ip = sys.argv[1].split("-")[0]
            end_ip = sys.argv[1].split("-")[1]
        else:
            end_ip = sys.argv[1]
            start_ip= sys.argv[1]
        print "StartIP: %s \t EndIP: %s" % (start_ip,end_ip)

        #处理端口参数
        if sys.argv[2].find(",") != -1:
            ip_arry = sys.argv[2].split(",")
            is_iparry = True
        elif sys.argv[2].find("-") != -1:
            start_port = string.atoi(sys.argv[2].split("-")[0])
            end_port = string.atoi(sys.argv[2].split("-")[1])+1
        else:
            start_port = string.atoi(sys.argv[2])
            end_port = string.atoi(sys.argv[2])+1
             

        #检测数组里边是否是数字，不是的话，触发异常
        for i in ip_arry:
            string.atoi(i)
            
        #处理线程参数
        num_thread = string.atoi(sys.argv[3])
    
        #ip地址第一位的范围
        start_ip1 = string.atoi(start_ip.split(".")[0])
        end_ip1 = string.atoi(end_ip.split(".")[0])+1 #range最后一个不包括所以加1

        #ip地址第二位的范围
        start_ip2 = string.atoi(start_ip.split(".")[1])
        end_ip2 = string.atoi(end_ip.split(".")[1])+1 #range最后一个不包括所以加1

        #ip地址第三位的范围
        start_ip3 = string.atoi(start_ip.split(".")[2])
        end_ip3 = string.atoi(end_ip.split(".")[2])+1 #range最后一个不包括所以加1

        #ip地址第四位的范围
        start_ip4 = string.atoi(start_ip.split(".")[3])
        end_ip4 = string.atoi(end_ip.split(".")[3])+1 #range最后一个不包括所以加1
        
    except Exception,ex: #捕获所有异常
        #print ex
        usage()
              
    #判断ip地址合法性 因为前边endip加1了，所以范围到256
    if start_ip1 not in range(1,256) or end_ip1 not in range(1,256) or start_ip2 not in range(0,256) or end_ip2 not in range(0,256) or start_ip3 not in range(1,256) or end_ip3 not in range(1,256) or start_ip4 not in range(1,256) or end_ip4 not in range(1,256):
        usage()
    #一层
    for i in range(start_ip1,end_ip1):
        if i == start_ip1 and i!=end_ip1-1:
            start2 = start_ip2
            end2 = 255
        elif i == end_ip1-1 and i!=start_ip1:
            start2 = 0
            end2 = end_ip2
        elif i == start_ip1 and i == end_ip1-1:
            start2 = start_ip2
            end2 = end_ip2
        else:
            start2 = 0
            end2 = 255
        #print "%d %d" % (start2,end2)
        #二层   
        for j in range(start2,end2):
            if j == start2 and j!=end2-1:
                start3 = start_ip3
                end3 = 255
            elif j == end2-1 and j!=start2:
                start3 = 0
                end3 = end_ip3
            elif j == start2 and j == end2-1:
                start3 = start_ip3
                end3 = end_ip3
            else:
                start3 = 0
                end3 = 255
            #print "%d %d" % (start3,end3)
            #三层
            for k in range(start3,end3):
                if k == start3 and k!=end3-1:
                    start4 = start_ip4
                    end4 = 255
                elif k == end3-1 and k!=start3:
                    start4 = 0
                    end4 = end_ip4
                elif k ==start3 and k == end3-1:
                    start4 = start_ip4
                    end4 = end_ip4
                else:
                    start4 = 0
                    end4 = 255
                #print "%d %d" % (start4,end4)
                #四层
                for l in range(start4,end4):
                    targetIP = str(i)+"."+str(j)+"."+str(k)+"."+str(l)
                    #print targetIP
                    tp=ThreadPool(targetIP)
                    tp.new_complete()
