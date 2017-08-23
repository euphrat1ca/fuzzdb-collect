# encoding: utf-8

import sys
reload(sys)
sys.path.append("../../")
sys.setdefaultencoding('utf-8')

import json
from utils.misc import *
from utils.plugin.blacklist import Blacklist

class Resolver_17ce(object):
    """docstring for Resolver_17ce"""
    def __init__(self, domain):
        super(Resolver_17ce, self).__init__()
        self.domain = domain
        self.server = 'http://www.17ce.com'
        self.iplist = []

    def run(self):
        tid = self.get_tid()
        if tid is not None:
            url = '{0}/apis/ajaxfresh?callback=&tid={1}&num=0&ajax_over=0'.format(
                self.server, tid)
            for ip in self.parser(self.get_content(url)):
                self.iplist.append(ip)

    def get_tid(self):
        url = '{0}/apis/dns?callback=&url={1}&curl=&rt=1&nocache=0&host=&referer=&cookie=&agent=&speed=&pingcount=&pingsize=&area[]=0&area[]=1&area[]=2&area[]=3&&isp[]=0&isp[]=1&isp[]=2&isp[]=6&isp[]=7&isp[]=8&isp[]=4'.format(
            self.server, self.domain)
        _regex = re.search('","tid":"(.*?)","pre":""', self.get_content(url))
        return _regex.group(1) if _regex else None

    def parser(self, content):
        _regex = re.compile(r'(?<=\[\{\"srcip\"\:\").*?(?=\",\"ipfrom\")', re.DOTALL)
        ips = _regex.findall(content)
        for item in list(set(ips)):
            yield item

    def get_content(self, url):
        response = http_request_get(url).content
        return response if response is not None else ''

resolver = Resolver_17ce('www.wooyun.org')
resolver.run()
print resolver.iplist


