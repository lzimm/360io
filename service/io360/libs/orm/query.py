import MySQLdb
from MySQLdb.cursors import DictCursor

class Query():
    
    db = MySQLdb.connect(user="qorporation", db="io360")
    
    def __enter__(self):
        return self
        
    def __init__(self, query):
        self.query = query
        self.limit = 0
        self.offset = 0
        self.count = None
        
        if self.query.endswith(';'):
            self.query = self.query[0:len(self.query)-1]
            
        self.c = None
        
    def filter(self, *args, **kwargs):
        if self.wrapper:
            try:
                self.db.ping()
                self.c = self.db.cursor(DictCursor)
            except:
                self.db = MySQLdb.connect(user="qorporation", db="io360")
                self.c = self.db.cursor(DictCursor)
            
            table = self.wrapper.__name__.split(".")[-1].lower()
            queryparts = self.query.split("WHERE")
            
            kwargs["sep"] = " AND "
            join, filterpart = self.wrapper.filter_string(*args, **kwargs)
            
            if queryparts[0].split("FROM")[1].strip() != table:
                self.query = """%s LEFT JOIN %s ON %s = id %s WHERE %s %s""" % (queryparts[0], table, table, join,
                        queryparts[1], filterpart)
            else:
                self.query = """%s %s WHERE %s %s""" % (queryparts[0], join, queryparts[1], filterpart)

            self.execute(None, self.wrapper, limit=kwargs.get('limit', 10), offset=kwargs.get('offset', 0))

        return self
    
    def execute(self, callback=None, wrapper=None, limit=0, offset=0, delayed=False):
        self.wrapper = wrapper
        
        if not delayed or not wrapper:
            self.transact(limit=limit, offset=offset)
            self.db.commit()
        
            if callback is not None:
                callback()
        else:
            if limit:
                self.limit = int(limit)
                self.offset = int(offset)
                      
        return self

    def prepare(self, wrapper=None, limit=0, offset=0):
        self.wrapper = wrapper

        if limit:
            self.limit = int(limit)
            self.offset = int(offset)
            
        return self
    
    def transact(self, callback=None, limit=0, offset=0):
        query = self.query
        
        if limit:
            self.limit = int(limit)
            self.offset = int(offset)
        
        if query.find('LIMIT') < 0 and self.limit:
            query = "%s LIMIT %s, %s" % (query, self.offset * self.limit, self.limit)
            
        if query.find('LIMIT') > 0 and query.find('CALC_FOUND_ROWS') < 0:
            query = query.replace('SELECT', 'SELECT SQL_CALC_FOUND_ROWS', 1)

        try:
            self.db.ping()
            self.c = self.db.cursor(DictCursor)
        except:
            self.db = MySQLdb.connect(user="qorporation", db="io360")
            self.c = self.db.cursor(DictCursor)

        self.c.execute(query)

        if callback is not None:
            callback()
            
        return self
    
    def commit(self):
        self.db.commit()

    def rollback(self):
        self.db.rollback()
    
    def fetchone(self):
        if self.c:
            row = self.c.fetchone()

            if not row:
                return None

            if self.wrapper:
                return self.wrapper(row)
            else:
                return row
        else:
            self.execute(None, self.wrapper, self.limit, self.offset)
            return self.fetchone()
            
    def fetchmany(self, n):
        if self.c:
            if self.wrapper:
                return [self.wrapper(row) for row in self.c.fetchmany(n)]
            else:
                return self.c.fetchmany(n)

        else:
            self.execute(None, self.wrapper, self.limit, self.offset)
            return self.fetchmany(n)
    
    def fetchall(self, count=False):
        if self.c:
            ret = self.c.fetchall()
            
            if self.wrapper:
                if count:
                    self.fetchcount()
                    
                return [self.wrapper(row) for row in ret]
    
            else:
                if count:
                    self.fetchcount()
                    
                return ret

        else:
            self.execute(None, self.wrapper, self.limit, self.offset)
            return self.fetchall()
    
    def fetchcount(self, more=False):
        if self.c:
            if self.count is None:
                self.c.execute('SELECT FOUND_ROWS()')
                self.count = self.c.fetchone()['FOUND_ROWS()']
                
            if more:
                return self.count > (self.limit + self.offset * self.limit)
            else:
                return self.count

        else:
            self.execute(None, self.wrapper, self.limit, self.offset)
            return self.fetchcount(more)
            
    def __exit__(self, exc_type, exc_value, exc_traceback):
        if exc_type is not None:
            self.db.rollback()
        else:
            self.db.commit()
        self.c.close()

class Transaction():
    def __enter__(self):
        return self
    
    def __init__(self):
        pass
    
    def execute(self, query, callback=None, limit=None, offset=0):
        self.query = query
        return query.transact(callback=callback, limit=limit, offset=offset)

    def __exit__(self, exctype, excvalue, traceback):
        if exctype is not None:
            self.query.rollback()
        else:
            self.query.commit()