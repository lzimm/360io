import MySQLdb
from MySQLdb.cursors import DictCursor
from io360.libs.cluster.sync import ClusterSyncConnection
from io360.libs.cluster.data import ClusterDataConnection
from io360.libs.cluster.comm import ClusterCommConnection
from io360.libs.cluster.log import ClusterLogConnection

mysql = MySQLdb.connect(user="qorporation", db="io360")
sync = ClusterSyncConnection("localhost")
data = ClusterDataConnection("localhost")
comm = ClusterCommConnection("localhost")
log = ClusterLogConnection("localhost")

def db():
    try:
        mysql.ping()
        return mysql.cursor(DictCursor)
    except:
        mysql = MySQLdb.connect(user="qorporation", db="io360")
        return mysql.cursor(DictCursor)

def import_mod(name):
    try:
        mod = __import__(name)
        components = name.split('.')
        for comp in components[1:]:
            mod = getattr(mod, comp)
        return mod
    except Exception, e:
        print e
        raise

def import_comp(mod, comp):
    mod = __import__(mod, globals(), locals(), [comp])
    return getattr(mod, comp)