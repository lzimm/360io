from io360.settings import TWITTER_KEY, TWITTER_SECRET
from io360.libs.utils import Redirect
from io360 import data

from twisted.internet.defer import Deferred
from twisted.web import client
from hack.web2.http_headers import Cookie

from cgi import parse_qs
from hashlib import sha1
from hmac import new as hmac
from random import getrandbits
from time import time
from urllib import urlencode
from urllib import quote as urlquote
from urllib import unquote as urlunquote

import simplejson

TWITTER_REQUEST_URL = "http://twitter.com/oauth/request_token"
TWITTER_AUTHORIZE_URL = "http://twitter.com/oauth/authorize"

def encode(text):
    return urlquote(text, safe='~')

def http(req):    
    if req.user:  
        res = Deferred()
              
        def userCb(payload):
            user = simplejson.loads(payload)
            
            private = user['private']
            
            def tokCb(payload):
                result = parse_qs(payload)
                auth_token = result["oauth_token"][0]
                auth_secret = result["oauth_token_secret"][0]

                private['twitter'] = {'oauth_token_secret': auth_secret}

                def saveCb(payload):
                    req.result = "ok"
                    
                    redirect = Redirect("%s?oauth_token=%s" % (TWITTER_AUTHORIZE_URL, auth_token))
                    res.callback(redirect)
                
                d2 = data.set('AUTH', req.user['id'], simplejson.dumps(private))
                d2.addCallback(saveCb)               

            params = {
                "oauth_consumer_key": encode(TWITTER_KEY),
                "oauth_signature_method": "HMAC-SHA1",
                "oauth_timestamp": str(int(time())),
                "oauth_nonce": encode(str(getrandbits(64))),
                "oauth_version": "1.0"
            }

            params_str = "&".join(["%s=%s" % (encode(k), encode(params[k])) for k in sorted(params)])
            message = "&".join(["GET", encode(TWITTER_REQUEST_URL), encode(params_str)])

            key = "%s&%s" % (encode(TWITTER_SECRET), "")
            signature = hmac(key, message, sha1)
            digest_base64 = signature.digest().encode("base64").strip()
            params["oauth_signature"] = digest_base64

            tok = client.getPage(url="%s?%s" % (TWITTER_REQUEST_URL, urlencode(params)))
            tok.addCallback(tokCb)
    
        d1 = data.get('AUTH', req.user['id'])
        d1.addCallback(userCb)
    
        return res
    
    else:
        req.result = None
        return None