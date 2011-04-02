from io360.libs.cluster import ClusterConnection, CLUSTER_SYNC_PORT

class ClusterSyncConnection(ClusterConnection):
    def __init__(self, host):
        ClusterConnection.__init__(self, host, CLUSTER_SYNC_PORT)
        
    def getNext(self, key):
        return self.send("0 NEXT_CONT %s\r\n" % (key))