from io360.settings import SECRET_KEY
from hack.web2.http_headers import Cookie
import hashlib

def updateUserCookie(req, user, **kwargs):
    if user:
        userdata = hashlib.md5(SECRET_KEY + req.remoteAddr.host + str(user['id'])).hexdigest()
        id = user['id']
        name = user['name']

    else:
        userdata = ''
        id = ''
        name = ''
        
    req.cookies.append(Cookie('auth', userdata, path='/'))
    req.cookies.append(Cookie('user_id', id, path='/'))
    req.cookies.append(Cookie('user_name', name, path='/'))
    
    for k, v in kwargs.iteritems():
        req.cookies.append(Cookie(k, v, path='/'))