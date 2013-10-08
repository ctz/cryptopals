import hashlib
import os

lolhash_length = 2 # bytes
lolhash_blocksz = 16
lolhash_initial = 'lo'

def C(m, H):
    # just truncated md5
    r = hashlib.md5(H + m).digest()[:lolhash_length]
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
    lolhash_collide_calls += 1
    
    # return a pair of colliding blocks given start state H
    x = os.urandom(lolhash_blocksz)

    xH = C(x, H)
    while True:
        y = os.urandom(lolhash_blocksz)
        yH = C(y, H)
        if xH == yH and x != y:
            return x, y, xH, yH

def crosscheck(found):
    # for each pair, check the leftmost and rightmost stems hash to the same thing
    H_left = H_right = lolhash_initial

    for i in range(len(found)):
        left, right = found[i]
        H_left = C(left, H_left)
        H_right = C(right, H_right)

    assert H_left == H_right

    left = ''.join(x[0] for x in found)
    right = ''.join(x[1] for x in found)
    assert lolhash(left) == lolhash(right)

def f(n):
    # start off
    x, y, xh, yh = lolhash_collide(lolhash_initial)

    found = [(x, y)]
    to_process = [x, y]
    for i in range(1,n):
        x, y, xh, yh = lolhash_collide(xh)
        to_process.extend([x, y, xh, yh])
        found.append((x, y))

    crosscheck(found)
    print 'found', len(found), 'pairs of internal collisions giving', 2 ** len(found), 'colliding messages'
    
if __name__ == '__main__':
    f(32)
