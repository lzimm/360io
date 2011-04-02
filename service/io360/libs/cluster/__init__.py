from twisted.protocols.basic import LineReceiver
from twisted.internet.defer import Deferred
from twisted.internet import reactor, protocol

CLUSTER_DATA_PORT   = 9090
CLUSTER_SYNC_PORT   = 9900
CLUSTER_COMM_PORT   = 9000
CLUSTER_LOG_PORT    = 9909

class ClusterConnection:
    def __init__(self, host, port):
        self.deferred = protocol.ClientCreator(reactor, ClusterReceiver).connectTCP(host, port)
        
    def send(self, line):
        cb = Deferred()
        
        def cmd(proto):
            proto.backlog.append(cb)
            proto.sendLine(line)
            return proto

        self.deferred.addCallback(cmd)

        return cb

class ClusterReceiver(LineReceiver):
    def __init__(self):         
        self.backlog = []
        
        self.expectNone = False
        self.lenExpected = None
        self.bufferLength = 0
        self.buffer = []
        
        self.setRawMode()

    def rawDataReceived(self, data):
        self.buffer.extend(data)
        self.bufferLength += len(data)
        
        if self.lenExpected is None:
            spc = self.buffer.index(' ')
            if spc > 0:
                self.lenExpected = int(''.join(self.buffer[:spc]))
                if self.lenExpected == -1:
                    self.expectNone = True
                    self.lenExpected = 4

                self.bufferLength -= (spc + 1)
                self.buffer = self.buffer[spc + 1:]
            else:
                return

        if self.bufferLength >= self.lenExpected + 2:
            data = ''.join(self.buffer[:self.lenExpected])
            if self.expectNone and data == "NONE":
                data = None
            
            self.buffer = self.buffer[self.lenExpected + 2:]
            self.expectNone = False
            self.lenExpected = None
            self.bufferLength = 0
            
            self.backlog.pop(0).callback(data)