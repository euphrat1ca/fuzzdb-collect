# coding=gb2312
__author__ = '云絮'


pwdlist=[]
#获取Fuzzer密码的模板，每行为一条放进pwdlist列表中
def getPwdTemplat():
    fr=open('pwd.yx','r')
    while 1:
        line=fr.readline().strip()
        if not line:
            break
        pwdlist.append(line)

    fr.close()
#输出最后的字典结果到当前目录下的password.txt文件
def outputDicFile(fuzzerPwdList):
    f=open('password.txt','w')
    for pwd in fuzzerPwdList:
       f.write(pwd+'\n')
    f.close()
if __name__ == '__main__':
    word=raw_input('请输入Fuzzer密码的关键字[多个关键字","隔开]:')
    wordlist=word.split(',')
    getPwdTemplat()
    '''
        遍历列表，将密码模板的内容替换为列表里的关键字
    '''
    fuzzerResult=[]
    for word in wordlist:
        for i in pwdlist:
            i=i.replace('%username%',word)
            if i not in fuzzerResult:
                fuzzerResult.append(i)
    '''
        最后输出所有Fuzzer出的密码
    '''
    for temp in fuzzerResult:
        print temp

    outputDicFile(fuzzerResult)
    print '结果已保存到password.txt中...'


