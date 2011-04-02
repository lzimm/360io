from datetime import datetime
from hack.web2.http_headers import Cookie
from twisted.internet.defer import Deferred
from io360 import data, sync
from io360.settings import SECRET_KEY
from io360.includes.helpers import updateUserCookie
import string
import simplejson
import re
import hashlib

email_re = re.compile(
    r"(^[-!#$%&'*+/=?^_`{}|~0-9A-Z]+(\.[-!#$%&'*+/=?^_`{}|~0-9A-Z]+)*"  # dot-atom
    r'|^"([\001-\010\013\014\016-\037!#-\[\]-\177]|\\[\001-011\013\014\016-\177])*"' # quoted-string
    r')@(?:[A-Z0-9](?:[A-Z0-9-]{0,61}[A-Z0-9])?\.)+[A-Z]{2,6}\.?$', re.IGNORECASE)

def http(req):
    return None

def http_POST(req):
    name = req.args.get('name', [None])[0]
    email = req.args.get('email', [None])[0]
    password = req.args.get('password', [None])[0]

    if email and email_re.search(email):
        id = hashlib.md5(str(email).lower()).hexdigest()
        private = {'password': password, 'email': email}
        public = {'id': id, 'name': name}
        
        res = Deferred()

        def registerCb(payload):
            if (payload == 'ok'):
                req.result = public
                updateUserCookie(req, public, twitter=None)
            else:
                req.result = False

            res.callback(None)

        d1 = data.addUser(id, simplejson.dumps(private), simplejson.dumps(public))
        d1.addCallback(registerCb)

        return res

    else:
        req.result = None
        return None