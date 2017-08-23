
[code="py"]
#-*- coding:utf-8 -*-

#简短地生成随机密码，包括大小写字母、数字，可以指定密码长度。

#生成随机密码

print '''
    #--------------------------------------------------#
    #          WelCome to forum.90sec.org      #
    #--------------------------------------------------#  
     '''


from random import choice
import string

#python3中为string.ascii_letters,而python2下则可以使用string.letters和string.ascii_letters




def GenPassword(length=8,chars=string.ascii_letters+string.digits):
    return ''.join([choice(chars) for i in range(length)])

if __name__=="__main__":
    #生成10个随机密码    
    for i in range(10):
        #密码的长度为15
        print (GenPassword(15))

#下面两行代码是为了运行后命令窗口不自动关闭！        
import code        

code.interact(banner = "", local = locals())

[/code]
