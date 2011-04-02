from hack.web2 import http, responsecode

class HardResponder(object):
    def __init__(self, data):
        self.data = data
    
    def response(self, cookies=None):
        return self.data
        
class Redirect(HardResponder):
    def __init__(self, data):
        HardResponder.__init__(self, data)
    
    def response(self, cookies=None):
        if cookies:
            res = http.RedirectResponse(self.data)
            res.headers.setHeader('Set-Cookie',cookies)
            return res
        else:
            return http.RedirectResponse(self.data)

class NotFound(HardResponder):
    def __init__(self, data):
        HardResponder.__init__(self, data)
    
    def response(self, cookies=None):
        return responsecode.NOT_FOUND