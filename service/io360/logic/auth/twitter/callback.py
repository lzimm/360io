from io360.settings import TWITTER_KEY, TWITTER_SECRET
from io360.libs.utils import Redirect
from io360 import data

from twisted.internet.defer import Deferred
from twisted.web import client

from cgi import parse_qs
from hashlib import sha1
from hmac import new as hmac
from random import getrandbits
from time import time
from urllib import urlencode
from urllib import quote as urlquote
from urllib import unquote as urlunquote

import simplejson

TWITTER_ACCESS_URL = "http://twitter.com/oauth/access_token"

def encode(text):
    return urlquote(text, safe='~')

def http(req):    
    oauth_token = req.args.get('oauth_token', [None])[0]
    oauth_verifier = req.args.get('oauth_verifier', [None])[0]
    
    if req.user:  
        res = Deferred()
              
        def userCb(payload):
            user = simplejson.loads(payload)
            
            private = user['private']
            oauth_secret = private['twitter']['oauth_token_secret']
            
            def tokCb(payload):
                result = parse_qs(payload)
                private['twitter'] = result

                def saveCb(payload):
                    if (result.get('screen_name', None)):
                        req.result = result['screen_name'][0]
                    else:
                        req.result = ''

                    res.callback(None)
                
                d2 = data.set('AUTH', req.user['id'], simplejson.dumps(private))
                d2.addCallback(saveCb)

            params = {
                "oauth_consumer_key": encode(TWITTER_KEY),
                "oauth_signature_method": "HMAC-SHA1",
                "oauth_timestamp": str(int(time())),
                "oauth_nonce": encode(str(getrandbits(64))),
                "oauth_version": "1.0",
                "oauth_token": encode(oauth_token)
            }

            params_str = "&".join(["%s=%s" % (encode(k), encode(params[k])) for k in sorted(params)])
            message = "&".join(["GET", encode(TWITTER_ACCESS_URL), encode(params_str)])

            key = "%s&%s" % (encode(TWITTER_SECRET), encode(oauth_secret))
            signature = hmac(key, message, sha1)
            digest_base64 = signature.digest().encode("base64").strip()
            params["oauth_signature"] = digest_base64

            tok = client.getPage(url="%s?%s" % (TWITTER_ACCESS_URL, urlencode(params)))

            tok.addCallback(tokCb)
    
        d1 = data.get('AUTH', req.user['id'])
        d1.addCallback(userCb)
    
        return res
    
    else:
        req.result = None
        return None