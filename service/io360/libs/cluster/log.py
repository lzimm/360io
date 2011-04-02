import struct

from io360.libs.cluster import ClusterConnection, CLUSTER_LOG_PORT

def pack(*args):
    format = "="
    params = []
    
    for arg in args:
        if (arg):
            argLen = len(arg)
            format += "i%isx" % (len(arg))
            params += [argLen, arg]
        else:
            argLen = 0
            format += "ix"
            params += [argLen]

    return struct.pack(format, *params)

class ClusterLogConnection(ClusterConnection):
    def __init__(self, host):
        ClusterConnection.__init__(self, host, CLUSTER_LOG_PORT)
        
    def pull(self, key, count):
        return self.send("4 PULL %s %s\r\n" % (key, struct.pack("ix", count)))
        
    def push(self, key, value):
        return self.send("%s PUSH %s %s\r\n" % (len(value), key, value))