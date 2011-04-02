from datetime import datetime
from twisted.internet.defer import Deferred
from io360 import data, sync
from io360.includes.helpers import updateUserCookie
import string
import simplejson

def http(req):
    updateUserCookie(req, None, twitter=None)
    req.result = 'ok'
    return None

def http_POST(req):
    updateUserCookie(req, None, twitter=None)
    req.result = 'ok'
    return None