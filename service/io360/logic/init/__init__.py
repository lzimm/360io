from io360 import comm, data
from twisted.internet.defer import Deferred
from hack.web2.http_headers import Cookie

def initCommSession(req, res):
    def cb(payload):
        req.result.update({'session': payload})
        res.callback(payload)
    
    d1 = comm.init(req.remoteAddr.host)
    d1.addCallback(cb)

    return d1
    
def registerCommSession(req, res, session, channel):
    def cb(payload):
        res.callback(payload)
    
    d1 = comm.addSub(session, channel)
    d1.addCallback(cb)

    return d1
    
def getStreamTail(req, res, lid):
    def cb(payload):
        req.result.update({'tail': payload})
        res.callback(payload)
    
    d1 = data.get("TAIL", lid)
    d1.addCallback(cb)

    return d1
    
def authCommSession(req, res, session):
    if req.user or req.uchan:
        if req.user:        
            def addCb(payload):
                def broadcastCb(payload):
                    res.callback(None)
                
                d2 = comm.broadcast('/user/' + req.user['id'], "{connected: \"%s\"}" % (session))
                d2.addCallback(broadcastCb)
                req.cookies.append(Cookie('user_chan', req.user['id'], path='/'))
        
            d1 = comm.addSub(session, '/user/' + req.user['id'])
            d1.addCallback(addCb)

        elif req.uchan:
            def remCb(payload):
                def broadcastCb(payload):
                    res.callback(None)
                
                d2 = comm.broadcast('/user/' + req.uchan, "{disconnected: \"%s\"}" % (session))
                d2.addCallback(broadcastCb)
                req.cookies.append(Cookie('user_chan', '', path='/'))
        
            d1 = comm.removeSub(session, '/user/' + req.uchan)
            d1.addCallback(remCb)
        
        else:
            res.callback(None)
            return None       
        
        return None
        
    else:
        res.callback(None)
        return None

def http(req):
    return None