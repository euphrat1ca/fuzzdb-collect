import sys
diclist=open('str.dic').readlines()
prelist=diclist[diclist.index('##pre\n')+1:diclist.index('##preend\n')]
extlist=diclist[diclist.index('##ext\n')+1:diclist.index('##extend\n')]
andlist=diclist[diclist.index('##and\n')+1:diclist.index('##andend\n')]
passlist=diclist[diclist.index('##pass\n')+1:diclist.index('##passend\n')]
passwordlist=[]
for Pre in prelist:
    for Ext in extlist:
        for And in andlist:
             passwordlist.append(Pre.strip()+sys.argv[1].strip()+And.strip()+Ext.strip())
for Pass in passlist:
    passwordlist.append(Pass.strip())
for i in passwordlist:
	print sys.argv[1].strip()+':'+i.strip()
