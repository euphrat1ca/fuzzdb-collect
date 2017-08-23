'''
pop验证邮箱账号密码，163、QQ登邮箱的邮箱密码需要输入第三方授权码，自己搭建的邮箱服务则输入密码即可。
'''
#enconding=utf-8
import poplib

def check(email, password):
    try:
        email = email
        password = password
        pop3_server = "pop.163.com"
        server = poplib.POP3(pop3_server)
        
        #ssl加密后使用
        #server = poplib.POP3_SSL('pop.163.com', '995')
        print(server.set_debuglevel(1)) #打印与服务器交互信息
        print(server.getwelcome()) #pop有欢迎信息
        server.user(email)
        server.pass_(password)
        print('Messages: %s. Size: %s' % server.stat())
        print(email+": successful")
    except poplib.error_proto as e:
        print(email+"：fail")
        print(e)


if __name__ == '__main__':
    f = open("pop.txt", "r+")
    d = f.readlines()
    f.close()
    for line in d:
        line = line.strip()
        # mail = line.split(":")[0]
        # pwd = line.split(":")[1]
        a = line.split(":")
        check(a[0], a[1])
        print("*"*9)