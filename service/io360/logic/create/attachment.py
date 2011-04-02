from datetime import datetime
from hack.web2.http_headers import Cookie
from twisted.internet.defer import Deferred
from io360 import data, sync, comm, log
from io360.deps import s3
from io360.logic.create import TYPES, num_encode
from io360.settings import AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY, AWS_ATTACHMENT_BUCKET, WORKING_DIR 
from PIL import Image, ImageOps, ImageEnhance

import cStringIO
import string
import simplejson
import cgi
import uuid

ALPHABET = string.digits + string.ascii_lowercase + string.ascii_uppercase
ALPHABET_REVERSE = dict((c, i) for (i, c) in enumerate(ALPHABET))
BASE = len(ALPHABET)

def num_encode(n):
    s = []
    while True:
        n, r = divmod(n, BASE)
        s.append(ALPHABET[r])
        if n == 0: break
    return ''.join(reversed(s))

def http(req):
    return None

class DeferredWrapper:
    def __init__(self):
        self.first = False
        self.second = False
        self.deferred = Deferred()
        
    def callback(self, data):
        if self.first and self.second:
            self.deferred.callback(data)
        elif self.first:
            self.second = True
        else:
            self.first = True

def http_POST(req):
    attachment = req.files.get('attachment', [None])[0]
    id = num_encode(uuid.uuid4().int)
    
    atype = None
    aname = None
    if attachment:
        atype = TYPES.get(attachment[0][-4:].lower(), None)
        if atype:
            res = DeferredWrapper() 
                        
            aname = "attch_%s.%s" % (id, atype['extension'])
            store = s3.SimpleStorageService(AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY)
            imgdata = attachment[2].read()

            def putCb(payload):
                res.callback(None)
            
            d1 = store.putObjectData(AWS_ATTACHMENT_BUCKET, aname, imgdata, contentType=atype['contentType'], public=True)
            d1.addCallback(putCb)
            
            # scaled image
            
            imgbuffer = cStringIO.StringIO(imgdata)
            scaledata = Image.open(imgbuffer)
            
            (w, h) = scaledata.size
            (x, y) = (0, 0)
            xscale = float(900.0 / float(w))
            yscale = float(400.0 / float(h))
            if xscale > yscale:
                scale = yscale
            else:
                scale = xscale
            if scale > 1.0:
                scale = 1.0
            w = int(w * scale)
            h = int(h * scale)
            
            scaledata = scaledata.resize((w, h), Image.ANTIALIAS)
            imgbuffer.close()
            imgbuffer = cStringIO.StringIO()
            scaledata.save(imgbuffer, atype['encoder'])
            scaledata = imgbuffer.getvalue()
            imgbuffer.close()

            d2 = store.putObjectData(AWS_ATTACHMENT_BUCKET, "scaled_%s" % aname, scaledata, contentType=atype['contentType'], public=True)
            d2.addCallback(putCb)

            # thumbnail image

            imgbuffer = cStringIO.StringIO(imgdata)
            thumbdata = Image.open(imgbuffer)
            
            (w, h) = thumbdata.size
            (x, y) = (0, 0)
            scale = float(50.0 / float(w))
            w = int(w * scale)
            h = int(h * scale)
            
            thumbdata = thumbdata.resize((w, h), Image.ANTIALIAS)
            thumbdata = ImageEnhance.Brightness(ImageOps.grayscale(thumbdata)).enhance(0.50)
            imgbuffer.close()
            imgbuffer = cStringIO.StringIO()
            thumbdata.save(imgbuffer, atype['encoder'])
            thumbdata = imgbuffer.getvalue()
            imgbuffer.close()
            
            d3 = store.putObjectData(AWS_ATTACHMENT_BUCKET, "thumb_%s" % aname, thumbdata, contentType=atype['contentType'], public=True)
            d3.addCallback(putCb)
            
            req.result = {'name': aname, 'type': atype}
            return None
        else:
            req.result = None
            return None