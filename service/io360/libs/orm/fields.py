import hashlib

from MySQLdb import escape_string
from io360 import import_comp
from query import Query

class Field(object):
    _getset = None
    
    def __init__(self, *args, **kwargs):
        for k in kwargs:
            setattr(self, k, kwargs[k])
    
    def _interface(self, name):
        if self._getset is None:
            
            def getter(ref):
                return ref._fieldvals[name]['value']
                
            def setter(ref, value):
                ref._fieldvals[name]['value'] = escape_string(value)
                ref._fieldvals[name]['dirty'] = True
                
            self._getset = (getter, setter)
            
        return self._getset

    def _fireChange(self, ref, name, dirty):
        return None
    
    def _filter_is(self, ref, name, value):
        return ("%s = '%s'" % (name, value), None)

class ModelField(Field):
    def __init__(self, *args, **kwargs):
        self._model = None
        self._table = None
        self._counterpart = None
        self.cache = None
        self.orderby = None
        self.order = "DESC"
        Field.__init__(self, *args, **kwargs)

    def _interface(self, name):
        if self._getset is None:                            
            if self.relation is 'n_to_n':
            
                def getter(ref):
                    self._prep(ref=ref)

                    if self._table is None:
                        self._table = ((name > self._counterpart) and 
                                        ("%s_%s" % (name, self._counterpart)) or
                                        ("%s_%s" % (self._counterpart, name)))
                    
                    query = "SELECT * FROM %s WHERE %s = '%s';" % (self._table, ref._table, ref.id)
                    
                    return Query(query).prepare(wrapper=self._model)

                def setter(ref, value):
                    self._prep(ref=ref)

                    if self._table is None:
                        self._table = ((name > self._counterpart) and 
                                        ("%s_%s" % (name, self._counterpart)) or
                                        ("%s_%s" % (self._counterpart, name)))

                    if isinstance(value, self._model):
                        value = self._model(value)

                    query = """"INSERT INTO %s (%s, %s) VALUES 
                            ('%s', '%s')""" % (self._table, self._model._table, ref._table, value.id, ref.id)
                    query += ("""WHERE (SELECT COUNT(*) FROM %s WHERE %s = '%s' AND %s = '%s') 
                            IS '0';""" % (self._table, self._model._table, value.id, ref._table, ref.id))
                            
                    Query(query).execute()
                
            elif self.relation is '1_to_n':
            
                def getter(ref):
                    self._prep(ref=ref)

                    query = "SELECT * FROM %s WHERE %s = '%s';" % (self._model._table, self._counterpart, ref.id)
                    return Query(query).prepare(wrapper=self._model)

                def setter(ref, value):
                    self._prep(ref=ref, counterpart=False)

                    if not isinstance(value, self._model):
                        value = self._model(value)

                    value._fieldvals[ref._table]['value'] = ref.id
                    value._fieldvals[ref._table]['dirty'] = True
                    value.save()
                
            else:
            
                def getter(ref):
                    self._prep(ref=ref, counterpart=False)

                    if ref._fieldvals[name]['value']:
                        return ref._fields[name]._model.get(ref._fieldvals[name]['value'])
                    else:
                        return None

                def setter(ref, value):
                    self._prep(ref=ref, counterpart=False)

                    if isinstance(value, self._model):
                        value = value.id

                    ref._fieldvals[name]['value'] = value
                    ref._fieldvals[name]['dirty'] = True
                    
            self._getset = (getter, setter)
            
        return self._getset

    def _fireChange(self, ref, name, dirty):
        self._prep(ref=ref)
        
        cache = self._model._fields.get(self._counterpart, type('',(),{'_cache':None})).cache
        sep = ""
        
        if cache:
            query = "UPDATE %s SET " % self._model._table
            for field in dirty:
                if field in cache:
                    query += (sep + ("%s = '%s'" % (field, getattr(ref, field))))
                    sep = ", "
            query += " WHERE %s = '%s';" % (ref._table, ref.id)
        
        if sep:
            return Query(query)
        else:
            return None

    def _filter_join(self, ref, name, value):
        if self.relation is 'n_to_n':
            self._prep(ref)
            if self._table is None:
                self._table = ((name > self._counterpart) and 
                                ("%s_%s" % (name, self._counterpart)) or
                                ("%s_%s" % (self._counterpart, name)))

            return (None, "LEFT JOIN %s ON %s.%s = %s.id" % (self._table, self._table, self._counterpart, self._counterpart))
        elif self.relation is '1_to_n':
            self._prep(ref)
            return (None, "LEFT JOIN %s ON %s.%s = %s.id" % (self._model._table, self._model._table, self._counterpart, self._counterpart))
        else:
            return (None, None)
    
    def _prep(self, ref, counterpart = True):
        if self._model is None:
            self._model = import_comp('io360.models', self.model)

        if self._counterpart is None and counterpart:
            relation = self.relation
            if relation is 'n_to_1':
                relation = '1_to_n'
            elif relation is '1_to_n':
                relation = 'n_to_1'

            for field in self._model._fields:
                if getattr(self._model._fields[field], '_model', True) is None:
                    self._model._fields[field]._model = import_comp('io360.models', self._model._fields[field].model)
                    
                if (getattr(self._model._fields[field], '_model', type('',(),{'_table':None}))._table is ref._table
                    and getattr(self._model._fields[field], 'relation', None) is relation):
                        self._counterpart = field

class BooleanField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)

    def _interface(self, name):
        if self._getset is None:

            def getter(ref):
                return int(ref._fieldvals[name]['value']) and True

            def setter(ref, value):
                ref._fieldvals[name]['value'] = int(value and True)
                ref._fieldvals[name]['dirty'] = True

            self._getset = (getter, setter)

        return self._getset

class NumberField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)
        
    def _interface(self, name):
        if self._getset is None:

            def getter(ref):
                return ref._fieldvals[name]['value']

            def setter(ref, value):
                ref._fieldvals[name]['value'] = float(value)
                ref._fieldvals[name]['dirty'] = True

            self._getset = (getter, setter)

        return self._getset
        
    def _filter_in(self, ref, name, value):
        return ("%s IN %s" % (name, str(value).replace("[", "(").replace("]", ")")), None)

class StringField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)

    def _filter_like(self, ref, name, value):
        return ("%s LIKE '%%%s%%'" % (name, value), None)

    def _filter_prefix(self, ref, name, value):
        return ("%s LIKE '%s%%'" % (name, value), None)
        
    def _filter_suffix(self, ref, name, value):
        return ("%s LIKE '%%%s'" % (name, value), None)

    def _filter_in(self, ref, name, value):
        return ("(%s)" % (reduce(lambda acc, val: "%s OR %s LIKE '%s'" % (acc, name, val), list(value), "")[3:]), None)

class EmailField(StringField):
    def __init__(self, *args, **kwargs):
        StringField.__init__(self, *args, **kwargs)

class HashedField(StringField):
    def __init__(self, *args, **kwargs):
        StringField.__init__(self, *args, **kwargs)

    def _interface(self, name):
        def getter(ref):
            return ref._fieldvals[name]['value']
        def setter(ref, value):
            value = hashlib.sha1(value).hexdigest()
            ref._fieldvals[name]['value'] = escape_string(value)
            ref._fieldvals[name]['dirty'] = True
        return getter, setter

class KeyField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)

class TimestampField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)

    def _filter_date(self, ref, name, value):
        return ("%s = '%s'" % (name, value), None)

class DateField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)
  
    def _interface(self, name):
        if self._getset is None:

            def getter(ref):
                return ref._fieldvals[name]['value']

            def setter(ref, value):
                if not isinstance(value, str):
                  value = value.strftime("%Y-%m-%d %H:%M:%S")
                  
                ref._fieldvals[name]['value'] = escape_string(value)
                ref._fieldvals[name]['dirty'] = True

            self._getset = (getter, setter)

        return self._getset

    def _filter_year(self, ref, name, value):
        return ("YEAR(%s) = '%s'" % (name, value), None)

    def _filter_month(self, ref, name, value):
        if len(value) < 6:
            value = value[0:4] + "0" + value[4]
        return ("EXTRACT(YEAR_MONTH FROM %s) = '%s'" % (name, value), None)

    def _filter_date(self, ref, name, value):
        return ("DATE(%s) = '%s'" % (name, value), None)

class TextField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)

class URLField(Field):
    def __init__(self, *args, **kwargs):
        Field.__init__(self, *args, **kwargs)

class UserField(ModelField):
    def __init__(self, *args, **kwargs):
        ModelField.__init__(self, *args, **kwargs)