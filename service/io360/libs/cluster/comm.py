from io360.libs.cluster import ClusterConnection, CLUSTER_COMM_PORT

class ClusterCommConnection(ClusterConnection):
    def __init__(self, host):
        ClusterConnection.__init__(self, host, CLUSTER_COMM_PORT)
        
    def init(self, ip):
        return self.send("0 INIT %s \r\n" % (len(ip), ip))

    def addSub(self, cid, sub):
        return self.send("%s ASUB %s %s\r\n" % (len(sub), cid, sub))
        
    def removeSub(self, cid, sub):
        return self.send("%s RSUB %s %s\r\n" % (len(sub), cid, sub))
        
    def broadcast(self, chann, msg):
        return self.send("%s SEND %s %s\r\n" % (len(msg), chann, msg))