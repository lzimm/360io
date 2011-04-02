from io360 import data

from twisted.internet.defer import Deferred
from twisted.web import client
from hack.web2.http_headers import Cookie

import cgi
import simplejson
import re

STYLE_MATCH = re.compile("([^,{]*)[^{]*\s*\{([^}]*)}")

def http(req):
    if req.user:
        res = Deferred()
              
        def userCb(payload):
            user = simplejson.loads(payload)
            public = user['public']
            
            req.result = {'style' : public.get('style', '')}
            res.callback(None)            
    
        d1 = data.get('AUTH', req.user['id'])
        d1.addCallback(userCb)
    
        return res
    
    else:
        req.result = None
        return None

def http_POST(req):
    style = req.args.get('style', [None])[0]
    
    if style and req.user:
        style = style[1:]
        
        res = Deferred()
        
        def userCb(payload):
            user = simplejson.loads(payload)
            
            encoded_style = ""
            for tag, val in STYLE_MATCH.findall(style):
                if not tag.strip():
                    continue
                encoded_style = encoded_style + ("div._comment div.style_%s %s{%s}" % (req.user['id'], tag, val))
            
            public = user['public']
            public['style'] = encoded_style

            def saveCb(payload):
                req.result = "ok"
                res.callback(None)
            
            d2 = data.set('USER', req.user['id'], simplejson.dumps(public))
            d2.addCallback(saveCb)               
    
        d1 = data.get('AUTH', req.user['id'])
        d1.addCallback(userCb)
    
        return res
        
    elif req.user:
        return http(req)
    
    else:
        req.result = None
        return None