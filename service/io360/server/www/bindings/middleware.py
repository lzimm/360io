import cPickle
from StringIO import StringIO
from hack.web2.http_headers import Cookie
from io360.includes.helpers import updateUserCookie
from io360.settings import SECRET_KEY
import hashlib

def auth(req):
    req.cookies = []
    req.user = None
    req.uchan = None
    req.cookieData = {}

    auth = None
    uid = None
    uname = None
    
    cookies = req.headers.getHeader('cookie', [])
    
    if cookies:
        for cookie in cookies:
            req.cookieData[cookie.name] = cookie.value
            
            if cookie.name == 'auth':
                auth = cookie.value
            elif cookie.name == 'user_id':
                uid = cookie.value
            elif cookie.name == 'user_name':
                uname = cookie.value
            elif cookie.name == 'user_chan':
                req.uchan = cookie.value
    
    if auth and uid:
        if auth == hashlib.md5(SECRET_KEY + req.remoteAddr.host + str(uid)).hexdigest():
            req.user = {'id': uid, 'name': uname}

        else:
            updateUserCookie(req, None, twitter=None)

    return None