from __future__ import with_statement

import socket, os, time, copy, fields, hashlib

from datetime import datetime
from threading import Semaphore
from query import Query, Transaction
from zope.interface import Attribute, Interface, interface

relations = {'1_to_n':'1_to_n', 'n_to_1':'n_to_1', 'n_to_n':'n_to_n', '1_to_1':'1_to_1'}

class ModelBuilder(type):
    def __new__(cls, name, bases, dct):
        _fields = dict()
        _fieldvals = dict()
        
        dct.setdefault('created', fields.TimestampField())
        
        for (index, val) in dct.items():
            if isinstance(val, fields.Field):
                _fields[index] = val
                _fieldvals[index] = {'value':None, 'dirty':False}
                     
                getter, setter = val._interface(index)
                dct[index] = property(getter, setter)
        
        dct['_fields'] = _fields
        dct['_fieldvals'] = _fieldvals
        dct['_table'] = name.lower()
        
        return type.__new__(cls, name, bases, dct)

class Model(object):
    __metaclass__ = ModelBuilder
    
    id = property(lambda self: self._id)
    sema = Semaphore()
    
    def __init__(self, row=None):
        self._fieldvals = copy.deepcopy(self.__class__._fieldvals)
        
        if row:
            for field in self._fields:
                if field in row:
                    self._fieldvals[field] = {'value': row[field], 'dirty':False}
            
            self._id = row['id']
            self._created = False
        else:
            self._id = self._generate_id()
            self._created = True
            self.created = str(datetime.today())
    
    def _row(self):
        row = {'id': self._id}
        for field in self._fields:
            row[field] = self._fieldvals[field]['value']
        return row
        
    def _generate_id(self):    
        base = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz"
        
        self.sema.acquire()
        stamp = long(str(time.time()).replace('.',''))
        self.sema.release()
        
        unique = ""
        while stamp > 0:
            unique = base[stamp % 62] + unique
            stamp /= 62
        
        proc = str(os.getpid())
        node = hashlib.sha1(socket.gethostname()).hexdigest()[0:5]
        
        return node + proc + unique

    def get(cls, id):
        query = "SELECT * FROM " + cls._table + " WHERE id = '%s';" % id
        return Query(query).execute(wrapper = cls).fetchone()

    get = classmethod(get)
    
    def filter(cls, *args, **kwargs):
        join, query = cls.filter_string(*args, **kwargs)

        if query:
            query = "SELECT * FROM %s %s WHERE %s" % (cls._table, join, query)
            return Query(query).execute(wrapper = cls, limit = kwargs.get('limit', 10), offset = kwargs.get('offset', 0))
        else:
            return None

    filter = classmethod(filter)
    
    def filter_string(cls, *args, **kwargs):
        join = ""
        query = ""
        joinsep = kwargs.get("joinsep", "")
        sep = kwargs.get("sep", "")
        group = kwargs.get("group", "")
        order_by = kwargs.get("order_by", "")
        order_dir = kwargs.get("order_dir", "DESC")
        limit = kwargs.get("limit", "")
        offset = kwargs.get("offset", "0")
        
        for key in kwargs:
            parts = key.split('__')
            if parts[0] in cls._fields:
                querybuilder = getattr(cls._fields[parts[0]], '_filter_' + parts[1], None)
                if querybuilder:
                    wherepart, joinpart = querybuilder(cls, parts[0], kwargs[key])
                    if wherepart:
                        query += (sep + wherepart)
                        sep = " AND "
                    if joinpart:
                        join += (joinsep + joinpart)
                        joinsep = " "
            elif parts[0] == 'id':
                querybuilder = getattr(fields.Field(), '_filter_' + parts[1], None)
                query += (sep + querybuilder(cls, parts[0], kwargs[key])[0])
                sep = " AND "
                
        if group:
            query += " GROUP BY %s" % (group)
                    
        if order_by:
            query += " ORDER BY %s %s" % (order_by, order_dir)

        if limit:
            query += " LIMIT %s, %s" % (int(offset) * int(limit), limit)
        
        return (join, query)

    filter_string = classmethod(filter_string)
        
    def save(self):
        dirty = list()
        
        if self._created:
            query = "INSERT INTO %s" % (self._table)
            fields = ""
            values = ""
            for (field, data) in self._fieldvals.items():
                if data['dirty']:
                    dirty.append(field)
                    fields += "," + field
                    values += "," + ("'%s'" % data['value'])
            query = "%s (id%s) VALUES ('%s'%s)" % (query, fields, self._id, values)
        else:
            query = "UPDATE %s SET" % (self._table)
            sep = ""
            for (field, data) in self._fieldvals.items():
                if data['dirty']:
                    dirty.append(field)
                    query += ("%s %s = '%s'" % (sep, field, data['value']))
                    sep = ","
            query = query + (" WHERE id = '%s';" % (self._id))
        
        queryset = list()
        for (field) in self._fields:
            queryset.append(self._fields[field]._fireChange(self, field, dirty))
        
        if len(dirty):
            with Transaction() as transaction:
                head = Query(query)
                transaction.execute(query=head, callback=lambda: self._saved(dirty))
                for query in queryset:
                    if query is not None:
                        transaction.execute(query=query)
    
    def _saved(self, dirty):
        self._created = False
        for field in dirty:
            self._fieldvals[field]['dirty'] = False

class IUser(Interface):
    def authenticate(self): pass