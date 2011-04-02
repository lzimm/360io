import sys, traceback
sys.path.append("/Life/360io/service")

from io360.libs.cluster.data import ClusterDataConnection, pack
data = ClusterDataConnection("localhost")
data.connect()

import simplejson

#data.set("USER", "uid", simplejson.dumps({"id": "uid", "name": "lewis"}))
#data.putLink("title", "url", "uid", "subject", "comment", "time") 
#print data.get("LEAD", "url")
print data.get("LEAD", "d")