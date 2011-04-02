import string
import simplejson
import cgi

TYPES = {
    '.gif' : {
        'contentType': 'image/gif',
        'extension': 'gif',
        'encoder': 'GIF'
    },
    
    '.jpg' : {
        'contentType': 'image/jpeg',
        'extension': 'jpg',
        'encoder': 'JPEG'
    },
    
    'jpeg' : {
        'contentType': 'image/jpeg',
        'extension': 'jpeg',
        'encoder': 'JPEG'
    },
    
    '.png' : {
        'contentType': 'image/png',
        'extension': 'png',
        'encoder': 'PNG'
    }
}

ALPHABET = string.digits + string.ascii_lowercase + string.ascii_uppercase
ALPHABET_REVERSE = dict((c, i) for (i, c) in enumerate(ALPHABET))
BASE = len(ALPHABET)

def num_encode(n):
    s = []
    while True:
        n, r = divmod(n, BASE)
        s.append(ALPHABET[r])
        if n == 0: break
    return ''.join(reversed(s))

def http(req):
    return None