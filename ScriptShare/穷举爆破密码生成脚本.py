前段时间，群里的小伙伴找到了一个shell版的穷举爆破密码生成工具，可是在windows环境下不能运行，就问怎么解决这个问题，于是用python重写了下
直接运行dictionary.py会显示使用说明
输出结果导入文件的话，要使用重定向，例如：dictionary.py --type 0 --number 6 >> dic.txt


# coding:utf-8
'''
Created on 2016年5月18日
 
@author: 莫须有
 
通过type参数判断生成字典类型
通过number参数判断生成字典长度
'''
import sys
 
 
sb=['']
string=''
def loop(str,number):
    global string
    for loop1 in range(0,len(str)):
        sb[len(sb)-number]=str[loop1]
        if(number-1>0):
            loop(str, number-1)
        else:
            for loop2 in range(0,len(sb)):
                string=string+sb[loop2]
            print string
            string=''
                     
                     
if len(sys.argv) >=5:   
    if sys.argv[1]=='--type' and sys.argv[3]=='--number':
        type=sys.argv[2]
        number=int(sys.argv[4])
        sb=sb*number
        str0=['0','1','2','3','4','5','6','7','8','9']
        str1=['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z']
        str2=['A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z']
        str3=str0+str1
        str4=str0+str2
        str5=str1+str2
        str6=str0+str1+str2
        if type=='0':
            loop(str0, number)
            sys.exit()
        if type=='1':
            loop(str1, number)
            sys.exit()
        if type=='2':
            loop(str2, number)
            sys.exit()
        if type=='3':
            loop(str3, number)
            sys.exit()
        if type=='4':
            loop(str4, number)
            sys.exit()
        if type=='5':
            loop(str5, number)
            sys.exit()
        if type=='6':
            loop(str6, number)
            sys.exit()
print """
*********************************DICTIONARY********************************
*****        command:dictionary.py --type type --number number        *****
*****        type:0-->>'0'to'9'                                       *****
*****        type:1-->>'a'to'z'                      # # # # #        *****
*****        type:2-->>'A'to'Z'                    |=# x = x #=|      *****
*****        type:3-->>'0'to'9'+'a'to'z'           |=# o x o #=|      *****
*****        type:4-->>'0'to'9'+'A'to'Z'             # x 0 x #        *****
*****        type:5-->>'a'to'z'+'A'to'Z'             # # # # #        *****
*****        type:6-->>'0'to'9'+'a'to'z'+'A'to'Z'                     *****
*****        e.g.:dictionary.py --type 0 --number 6--->>>123456       *****
*********************************DICTIONARY********************************
"""
