
import poplib,imaplib,smtplib
import re,os,threading
semaphore = threading.Semaphore(1)
 
print '''if you input 1,it works with pop3,elseif you input 2,works with imap
and it needs 'emal.txt' where writes $email,$password in line.
!notice:it works with opening pop3&imap,hotmail hasn't imap and make sure you can visit gmail.com,
or you'll get some wrong data'''
 
 
num = raw_input()
filehandle = open('email.txt','r')
flist = open('email.txt').readlines()
 
 
def getPopMail( mailDomain ):
    if mailDomain in ['163','gmali','126','qq','sina']:
        temp1 = 'pop.'+mailDomain+'.com'
        mail = poplib.POP3_SSL(temp1)   
    elif ( mailDomain == 'hotmail' ):
        mail = poplib.POP3_SSL('pop3.live.com')
    elif ( mailDomain == 'yahoo' ):
        mail = poplib.POP3_SSL('pop.mail.yahoo.com')
    return mail
 
def getImapMail( mailDomain ):
    if mailDomain in ['163','gmali','126','qq','sina']:
        temp2 = 'imap.'+mailDomain+'.com'
        mail = imaplib.IMAP4_SSL(temp2)
    elif ( mailDomain == 'hotmail' ):
        mail = ''
    elif ( mailDomain == 'yahoo' ):
        mail = imaplib.IMAP4_SSL('imap.mail.yahoo.com')
    return mail
 
def isValidEmail(email,password):
    emailparts = email.split('@')
    str1 = str(emailparts[1])
    global subDomain
    subDomain=''
    if( len( emailparts ) != 2 ):
        print "Email Fomat Error ";
        return False  
    for la1 in ['gmail','163','yahoo','qq','126','sina']:
        regm1 = re.compile(la1)
        if(regm1.match(str1)):
            subDomain = la1
            return
    for la2 in ['outlook','hotmail','live']:
        regm2 = re.compile(la2)
        if(regm2.match(str1)):
            subDomain = 'hotmail'
            return
    
 
def choose_pop3(email,password):
    isValidEmail(email,password)
    ret = ""
    try:
        mail = getPopMail( subDomain );
        mail.user(str(email))
        mail.pass_(str(password))
        ret = mail.stat()
    except:
        pass
    retR = re.compile('\(\d*,\s*\d*');
    line = email+password
    if( retR.match(str(ret)) ):
        writeResultFile1('s',line)
        print 'Success Login';
        return True
    else:
        writeResultFile1('f',line)
        print 'Failed Login';
        return False
    
def choose_imap(email,password):
    isValidEmail(email,password)
    line = email+password
    try:
        mail = getImapMail(subDomain)
        mail.login(str(email), str(password))
        writeResultFile2('s',line)
        print 'Success Login'
        return True
    except:
        writeResultFile2('f',line)
        print 'Failed Login'
        return False
    
def writeResultFile1(result,line):
    if(result == 's'):
        fileHS = open("successMail1.txt",'a+')
    if(result == 'f'):
        fileHS = open('failedMail1.txt','a+')
    fileHS.write(line)
    
def writeResultFile2(result,line):
    if(result == 's'):
        fileHS = open("successMail2.txt",'a+')
    if(result == 'f'):
        fileHS = open('failedMail2.txt','a+')
    fileHS.write(line)
    
def mutithread( filepathlist,targetn ):
    task_threads=[] #存储线程   
    for line in filepathlist:
        content=line.split(',')
        if(len(content)==2):
            email=content[0].strip()
            password=content[1].strip()        
        t= threading.Thread( target=targetn,args=(email, password,) )
        task_threads.append(t)     
    for task in task_threads:
        if semaphore.acquire():
            task.start()
            task.join() #等待所有线程结束
            semaphore.release()
    print("已经完成所有任务")
         
         
if __name__ == '__main__':
    if num=='1':
        mutithread(flist, choose_pop3)
    elif num=='2':
        mutithread(flist, choose_imap)
    else:
        print 'wrong input,please wash your hand and check somekey to exit'
        os.system('pause')
