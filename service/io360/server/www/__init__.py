def import_tmpl(tmpl):
    path = tmpl
    tmpl = tmpl[(tmpl.rfind('.')+1):]

    try:
        mod = __import__('io360.server.www.templates.' + path + '.index', globals(), locals(), ['index'])
        return getattr(mod, 'index')
    except:
        mod = __import__('io360.server.www.templates.' + path, globals(), locals(), [tmpl])
        return getattr(mod, tmpl)

def import_wrapper(tmpl):
    path = tmpl
    tmpl = tmpl[(tmpl.rfind('.')+1):]
    
    mod = __import__('io360.server.www.wrappers.' + path, globals(), locals(), [tmpl])
    return getattr(mod, tmpl)