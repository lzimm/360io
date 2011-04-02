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

TWITTER_UPDATE_URL = "http://twitter.com/statuses/update.json"

"""
'twitter': {'oauth_token_secret': ['DphEbGHPEQf49SBBDMCo38dP14H6T3rKijE4ve9LA'], 
            'user_id': ['13663372'], 
            'oauth_token': ['13663372-q6bjFWYjiiHazLEnDFP6s7sGj8r86DeuAIKqdF8AZ'], 
            'screen_name': ['lzimm']}
"""

def encode(text):
    return urlquote(text, safe='~')

def http(req):
    return http_POST(req)

def http_POST(req):
    tweet = req.args.get('tweet', [None])[0]
    
    if req.user:
        res = Deferred()
              
        def userCb(payload):
            user = simplejson.loads(payload)
            twitter = user['private'].get('twitter', None)
            
            if twitter:
                oauth_token = twitter['oauth_token'][0]
                oauth_secret = twitter['oauth_token_secret'][0]
                           
                def updateCb(payload):
                    req.result = simplejson.loads(payload)
                    res.callback(None)

                params = {
                    "oauth_consumer_key": encode(TWITTER_KEY),
                    "oauth_signature_method": "HMAC-SHA1",
                    "oauth_timestamp": str(int(time())),
                    "oauth_nonce": encode(str(getrandbits(64))),
                    "oauth_version": "1.0",
                    "oauth_token": encode(oauth_token),
                    "status": tweet
                }

                params_str = "&".join(["%s=%s" % (encode(k), encode(params[k])) for k in sorted(params)])
                message = "&".join(["POST", encode(TWITTER_UPDATE_URL), encode(params_str)])

                key = "%s&%s" % (encode(TWITTER_SECRET), encode(oauth_secret))
                signature = hmac(key, message, sha1)
                digest_base64 = signature.digest().encode("base64").strip()
                params["oauth_signature"] = digest_base64
    
                headers = {
                    'Content-Type': 'application/x-www-form-urlencoded; charset=utf-8',
                    'Authorization': ', '.join(['OAuth realm=""'] + ['%s="%s"' % (encode(k), encode(params[k])) for k in params if k[:6] == 'oauth_'])
                }

                update = client.getPage(url=TWITTER_UPDATE_URL, method='POST', postdata=urlencode({"status": tweet}), headers=headers)
                update.addCallback(updateCb)
            
            else:
                req.result = "notether"
                res.callback(None)
    
        d1 = data.get('AUTH', req.user['id'])
        d1.addCallback(userCb)
    
        return res
    
    else:
        req.result = "noaccount"
        return None