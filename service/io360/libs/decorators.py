from io360.libs.utils import Redirect
from urllib import quote_plus

from twisted.internet import threads

def deferred_member(func):
    def deferred_func(self, *argv, **kwargs):
        def runner():
            return func(self, *argv, **kwargs)
        return threads.deferToThread(runner)
    return deferred_func

class deferred:
    def __init__(self, func):
        self.func = func
    
    def __call__(self, *argv, **kwargs):
        def deferred_func():
            return self.func(*argv, **kwargs)

        return threads.deferToThread(deferred_func)

class authenticated:
    def __init__(self, func):
        self.func = func
    
    def __call__(self, req):
        if req.user:
            return self.func(req)
        else:
            return Redirect('/auth/login/?p=' + quote_plus(req.uri))

class admin:
    def __init__(self, func):
        self.func = func

    def __call__(self, req):
        if req.user:
            admin = req.settings.ref.admins.filter(id__is=req.user.id).fetchone()
            if admin:
                return self.func(req)
        return Redirect('/auth/login/?p=' + quote_plus(req.uri))