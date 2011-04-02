from datetime import datetime
from hack.web2.http_headers import Cookie
from twisted.internet.defer import Deferred
from io360 import data, sync, log
from io360.logic.create import num_encode

import string
import simplejson
import cgi

def http(req):
    return None

def http_POST(req):
    type = req.args.get('type', [None])[0]
    param = req.args.get('param', [None])[0]
    body = req.args.get('body', [None])[0]
    body = cgi.escape(body, quote=True).replace(':','&#58;').replace('\n', '<br />')
    attached = req.args.get('attached', [None])[0]
    
    time = datetime.now().isoformat()

    if req.user:
        user = simplejson.dumps(req.user)
        uid = req.user['id']

    else:
        name = req.args.get('name', [None])[0]
        email = req.args.get('email', [None])[0]
        user = simplejson.dumps({'name': name, 'email': email})
        uid = None

    title = req.args.get('title', [None])[0]

    if type:
        res = Deferred()
        
        def resCb(payload):
            id = num_encode(int(payload))

            params = {'param': param}
            if attached:
                params['attached'] = simplejson.loads(attached)

            jsonParams = simplejson.dumps(params)
            safeBody = simplejson.dumps({'body': body})[len("{'body': \""):-2]

            def headCb(payload):
                req.result = id

                if uid:
                    log.push("%s/head" % uid, simplejson.dumps({'type': type, 'params': params, 'body': safeBody}))
                
                res.callback(None)
            
            d2 = data.putHead(id, title, type, uid, jsonParams, safeBody, time, user)
            d2.addCallback(headCb)
        
        d1 = sync.getNext('res')
        d1.addCallback(resCb)
        
        return res

    else:
        req.result = None
        return None