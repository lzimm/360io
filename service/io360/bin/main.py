import sys, traceback
sys.path.insert(0, "/Life/360io/service/io360/patch")
sys.path.insert(0, "/Life/360io/service")

import os.path, time
import io360.server
from hack.web2 import server, http, resource, channel, iweb, stream, static
from twisted.internet import defer
from Cheetah.Template import Template
from io360 import import_mod, settings
from io360.server.www import import_tmpl
from io360.libs.utils import HardResponder
from io360.server.www.bindings import middleware_bindings
from io360.libs.orm.query import Query

logic_tree = dict()
view_tree = dict()

class IO360HTTP(resource.Resource):
    addSlash = True
    
    def __init__(self, logic_mod, path, mode = 'http'):
        self.logic_mod = logic_mod
        self.path = path
        self.mode = mode
        
        if '__val__' not in view_tree:
            logic_tree['__val__'] = import_mod('.'.join([self.logic_mod,'index']))
            view_tree['__val__'] = import_tmpl('index')

    def locateChild(self, req, segs):
        for binding in middleware_bindings:
            binding(req)
        
        try:
            [self.mode, name] = segs[0].split('-')
        except:
            name = segs[0]
        
        if name == '' and len(segs) == 1:
            return self, server.StopTraversal
        if os.path.isfile(os.path.join(self.path, name)):
            return IO360HTTPStatic(self, os.path.join(self.path, name)), []
        elif os.path.exists(os.path.join(self.path, name)):
            return IO360HTTPStatic(self, os.path.join(self.path, name)), segs[1:]
        else:
            logicpath = '.'.join([self.logic_mod,name])

            if name not in view_tree:
                try:
                    try:
                        logic_tree[name] = {'__val__':import_mod(logicpath)}
                    except:
                        pass

                    view_tree[name] = {'__val__':import_tmpl(name)}
                except:
                    return self, server.StopTraversal

            return IO360HTTPView(self, name, view_tree[name], logicpath, logic_tree[name], self.mode), segs[1:]

    def renderHTTP(self, req):            
        responder = getattr(logic_tree['__val__'], self.mode, logic_tree['__val__'].http)(req)
        view = view_tree['__val__']()
        view.req = req
        
        response = http.Response(200, stream=stream.MemoryStream(view.respond().encode('utf8')))
        response.headers.setRawHeaders('Content-Type', ['text/html; charset=utf-8'])
        
        if responder:
            responder(response)
        return response      

class IO360HTTPView(resource.Resource):
    addSlash = True

    def __init__(self, root, viewpath, view, logicpath, logic, mode = 'http'):
        self.root = root
        self.viewpath = viewpath
        self.logicpath = logicpath
        self.view = view
        self.logic = logic
        self.mode = mode
        
    def locateChild(self, req, segs):
        name = segs[0]
        
        self.logicpath = '.'.join([self.logicpath,name])
        self.viewpath = '.'.join([self.viewpath,name])
        
        if name == '' and len(segs) == 1:
            return self, server.StopTraversal
        elif name not in self.view:
            try:
                self.view[name] = {'__val__':import_tmpl(self.viewpath)}
                self.view = self.view[name]
                
                try:
                    if type(self.logic) is not type([]):
                        self.logic[name] = {'__val__':import_mod(self.logicpath)}
                        self.logic = self.logic[name]
                except:
                    self.logic = [self.logic]
            except:
                return self, server.StopTraversal
        else:
            self.view = self.view[name]
            try:
                if type(self.logic) is not type([]):
                    self.logic = self.logic[name]
            except:
                self.logic = self.logic
        return self, segs[1:]

    def renderHTTP(self, req):
        if type(self.logic) is type([]):
            self.logic = self.logic[0]
        
        mode = self.mode + '_' + req.method
        logic = getattr(self.logic['__val__'], mode, self.logic['__val__'].http)
        
        if req.method == 'POST':
            return server.parsePOSTData(req).addCallback(self.renderDeferred, *[req, logic])
        else:
            return self.renderDeferred(None, req, logic)
    
    def renderDeferred(self, dcb, req, logic):
        deferredLogic = defer.maybeDeferred(logic, req)
        
        def renderCallback(responder):
            if isinstance(responder, HardResponder):
                return responder.response(getattr(req, 'cookies', None))

            view = self.view['__val__']()
            view.req = req
            responseData = view.respond().encode('utf8')

            response = http.Response(200, stream=stream.MemoryStream(responseData))
            response.headers.setRawHeaders('Content-Type', ['text/html; charset=utf-8'])

            if getattr(req, 'cookies', None):
                response.headers.setHeader('Set-Cookie', req.cookies)

            if responder:
                responder(response)

            return response

        return deferredLogic.addCallback(renderCallback)
    
class IO360HTTPStatic(resource.Resource):
    addSlash = True

    def __init__(self, root, path):
        self.root = root
        self.path = path

    def locateChild(self, req, segs):
        if not segs:
            return None, server.StopTraversal

        name = segs[0]
        self.path = os.path.join(self.path, name)
        if not os.path.exists(self.path):
            return None, server.StopTraversal
        elif os.path.isdir(self.path):
            return IO360HTTPStatic(self.root, self.path), segs[1:]
        else:
            return self, server.StopTraversal

    def renderHTTP(self, req):
        if os.path.isfile(self.path):
            return static.File(self.path)
        else:
            return http.Response(200, stream=stream.MemoryStream('forbidden'))

site = server.Site(IO360HTTP('io360.logic', os.path.join(os.path.dirname(os.getcwd()), 'server/www/static')))

try:
    from twisted.internet import cfreactor
    cfreactor.install()
except:
    try:
        from twisted.internet import epollreactor
        epollreactor.install()
    except:
        pass

from twisted.application import service, strports
application = service.Application("io360")
service = strports.service('tcp:1080', channel.HTTPFactory(site))
service.setServiceParent(application)