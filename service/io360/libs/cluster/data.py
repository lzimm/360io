import struct

from io360.libs.cluster import ClusterConnection, CLUSTER_DATA_PORT

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

class ClusterDataConnection(ClusterConnection):
    def __init__(self, host):
        ClusterConnection.__init__(self, host, CLUSTER_DATA_PORT)
        
    def get(self, table, key):
        return self.send("0 RETR_%s %s\r\n" % (table, key))

    def add(self, table, key, value):
        return self.send("%s STOR_%s %s %s\r\n" % (len(value), table, key, value))
        
    def set(self, table, key, value):
        return self.send("%s REPL_%s %s %s\r\n" % (len(value), table, key, value))
        
    def append(self, table, key, value):
        return self.send("%s APND_%s %s %s\r\n" % (len(value), table, key, value))
    
    def addUser(self, uid, public, private):
        data = pack(public, private)
        return self.add("USER", uid, data)
    
    def putLink(self, lid, title, url, uid, comment, time, user):
        data = pack(title, url, uid, comment, time, user)
        return self.set("LINK", lid, data)
        
    def putComment(self, lid, id, pid, uid, user, comment, time):
        data = pack(id, pid, uid, user, comment, time)
        return self.set("POST", lid, data)
    
    def putStream(self, lid, id, pid, uid, user, comment, time):
        data = pack(id, pid, uid, user, comment, time)
        return self.set("STRM", lid, data)
        
    def putHead(self, lid, title, type, uid, params, body, time, user):
        data = pack(title, type, uid, params, body, time, user)
        return self.set("HEAD", lid, data)

    def putTail(self, lid, id, pid, uid, user, type, params, body, time):
        data = pack(id, pid, uid, user, type, params, body, time)
        return self.set("TAIL", lid, data)