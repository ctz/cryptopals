import hashlib
import os

lolhash_length = 2 # bytes
lolhash_blocksz = 16
lolhash_initial = 'lo'

C_debug = False

def C(m, H):
    # just truncated md5
    
    r = hashlib.md5(H + m).digest()[:lolhash_length]
    if C_debug:
        print('C(%r, %r) = %r' % (m, H, r))
    return r

def pad(m):
    for i in range(0, len(m), lolhash_blocksz):
        yield m[i:i+lolhash_blocksz]
    yield 'length:%d' % (len(m))

def lolhash(m):
    H = lolhash_initial

    for block in pad(m):
        H = C(block, H)
    return H

lolhash_collide_calls = 0

def lolhash_collide(H):
    global lolhash_collide_calls
    global C_debug
    lolhash_collide_calls += 1
    
    # return a pair of colliding blocks given start state H
    x = os.urandom(lolhash_blocksz)

    xH = C(x, H)
    while True:
        y = os.urandom(lolhash_blocksz)
        yH = C(y, H)
        if xH == yH and x != y:
            C_debug = True
            print 'found pair', x.encode('hex'), y.encode('hex')
            print 'found pair', C(x, H).encode('hex'), C(y, H).encode('hex')
            C_debug = False
            return x, y    

def crosscheck(found):
    global C_debug
    
    # for each pair, check the left and right stems hash to the same thing
    H_left = H_right = lolhash_initial
    print 'start', H_left.encode('hex'), H_right.encode('hex')

    C_debug = True
    for i in range(len(found)):
        left, right = found[i]
        print 'pair', i, left.encode('hex'), right.encode('hex')
        H_left = C(left, H_left)
        H_right = C(right, H_right)
        print 'hash', H_left.encode('hex'), H_right.encode('hex')
    C_debug = False

    print H_left, H_right

def f(n):
    target = 2 ** n
    
    # start off
    x, y = lolhash_collide(lolhash_initial)

    found = [(x, y)]
    to_process = [x, y]
    while len(found) < target / 2:
        x, y = lolhash_collide(x)
        to_process.extend([x, y])
        found.append((x, y))

    print 'found', len(found), 'collision pairs'
    print repr(found)

    crosscheck(found)
    
if __name__ == '__main__':
    f(5)
    
