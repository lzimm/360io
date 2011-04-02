from io360 import comm
from twisted.internet.defer import Deferred

def http(req):
    channel = req.args.get('channel', [None])[0]
    
    if channel:
        res = Deferred()
        
        def addCb(payload):
            def broadcastCb(payload):
                req.result = "ok"
                res.callback(None)
                
            d2 = comm.broadcast('/chan/' + channel, "{connected: \"%s\"}" % (req.postpath[0]))
            d2.addCallback(broadcastCb)

        d1 = comm.addSub(req.postpath[0], '/chan/' + channel)
        d1.addCallback(addCb)
        
        return res
        
    else:
        req.result = 'error'
        return None