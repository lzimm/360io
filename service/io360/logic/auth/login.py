from datetime import datetime
from twisted.internet.defer import Deferred
from io360 import data, sync
from io360.includes.helpers import updateUserCookie
import string
import simplejson
import hashlib

def http(req):
    return None

def http_POST(req):
    email = req.args.get('email', [None])[0]
    password = req.args.get('password', [None])[0]

    if email:
        id = hashlib.md5(str(email).lower()).hexdigest()
        
        res = Deferred()

        def loginCb(payload):
            req.result = False

            if payload:
                user = simplejson.loads(payload)
                if user['private']['password'] == password:
                    req.result = user['public']
                    req.result['twitter'] = user['private'].get('twitter', {}).get('screen_name', [None])[0]
                    updateUserCookie(req, user['public'], twitter=user['private'].get('twitter', {}).get('screen_name', [None])[0])

            res.callback(None)

        d1 = data.get('AUTH', id)
        d1.addCallback(loginCb)

        return res

    else:
        req.result = None
        return None