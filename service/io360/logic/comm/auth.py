from io360 import comm
from twisted.internet.defer import Deferred
from hack.web2.http_headers import Cookie

def http(req):
    if req.user or req.uchan:
        res = Deferred()
        
        if req.user:        
            def addCb(payload):
                def broadcastCb(payload):
                    req.result = 'ok'
                    res.callback(None)
                
                d2 = comm.broadcast('/user/' + req.user['id'], "{connected: \"%s\"}" % (req.postpath[0]))
                d2.addCallback(broadcastCb)
                req.cookies.append(Cookie('user_chan', req.user['id'], path='/'))
        
            d1 = comm.addSub(req.postpath[0], '/user/' + req.user['id'])
            d1.addCallback(addCb)

        elif req.uchan:
            def remCb(payload):
                def broadcastCb(payload):
                    req.result = 'ok'
                    res.callback(None)
                
                d2 = comm.broadcast('/user/' + req.uchan, "{disconnected: \"%s\"}" % (req.postpath[0]))
                d2.addCallback(broadcastCb)
                req.cookies.append(Cookie('user_chan', '', path='/'))
        
            d1 = comm.removeSub(req.postpath[0], '/user/' + req.uchan)
            d1.addCallback(remCb)
        
        else:
            req.result = 'ok'
            return None       
        
        return res
        
    else:
        req.result = 'error'
        return None