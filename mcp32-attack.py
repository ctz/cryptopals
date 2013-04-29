
from urllib import urlopen
import time

URL = 'http://localhost:8080/test/'
SIGLEN = 20
ROUNDS = 16

def hex2(tag):
    return ''.join('%02x' % x for x in tag)

def build_url(name, tag):
    return URL + name + '/' + hex2(tag)

def recover_sig(name):
    o = [0] * SIGLEN

    for i in range(SIGLEN):
        print i, ':', hex2(o)
        dist = {}

        for t in range(256):
            attack = list(o)
            attack[i] = t
            url = build_url(name, attack)

            for _ in range(ROUNDS):
                start = time.time()
                req = urlopen(url)
                code = req.getcode()
                end = time.time()
                dist[t] = dist.get(t, 0) + (end - start)
       
        chosen, _ = max(dist.items(), key = lambda x: x[1])
        o[i] = chosen

    # check
    req = urlopen(build_url(name, o))
    print hex2(o), 'yields', req.getcode()

    return o

if __name__ == '__main__':
    recover_sig('test')
