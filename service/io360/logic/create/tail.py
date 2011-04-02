from datetime import datetime
from hack.web2.http_headers import Cookie
from twisted.internet.defer import Deferred
from io360 import data, sync, comm, log
from io360.logic.create import num_encode

import string
import simplejson
import cgi

def http(req):
    return None

def http_POST(req):
    pid = req.args.get('pid', [None])[0]
    puid = req.args.get('uid', [None])[0]
    lid = req.args.get('lid', [None])[0]
    
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

    if lid:
        res = Deferred()
        
        def nextCb(payload):
            id = num_encode(int(payload))
            req.result = id

            params = {'param': param}
            if attached:
                params['attached'] = simplejson.loads(attached)

            jsonParams = simplejson.dumps(params)
            safeBody = simplejson.dumps({'body': body})[len("{'body': \""):-2]

            def tailCb(payload):
                if payload:
                    payload = simplejson.loads(payload)

                broadcast = {
                    'tail' : [{
                        'id' : id,
                        'parent' : pid,
                        'uid' : uid,
                        'lid' : lid,
                        'user' : user,
                        'type' : type,
                        'params' : params,
                        'body' : safeBody,
                        'time' : time
                    }],

                    'users' : [payload]
                }
                
                def broadcastCb(payload):
                    if puid:
                        def uidCb(payload):
                            res.callback(None)

                        if puid:
                            log.push("%s/reply" % uid, simplejson.dumps({'reply': broadcast['tail'][0]}))
                        
                        d4 = comm.broadcast('/user/' + puid, simplejson.dumps({'reply': broadcast}))
                        d4.addCallback(uidCb)
                                    
                    else:
                        res.callback(None)

                if uid:
                    log.push("%s/tail" % uid, simplejson.dumps({'tail': broadcast['tail'][0]}))
                        
                d3 = comm.broadcast('/chan/tail_' + lid, simplejson.dumps({'broadcast': broadcast}))
                d3.addCallback(broadcastCb)
            
            d2 = data.putTail(lid, id, pid, uid, user, type, jsonParams, safeBody, time)
            d2.addCallback(tailCb)
        
        d1 = sync.getNext('tail_' + lid)
        d1.addCallback(nextCb)
        
        return res
        
    else:
        req.result = None
        return None