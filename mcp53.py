import hashlib
import random
import math

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

def iterate(H, k):
    # feed in lots of zero blocks
    if k == 0:
        return H
    for i in xrange(2 ** (k - 1)):
        H = C('\x00' * lolhash_blocksz, H)
    return H

def random_block():
    return ''.join(chr(random.getrandbits(8)) for i in range(lolhash_blocksz))

def lolhash_collide_long(H, k):
    # return a pair of colliding blocks given start state H
    x = random_block()
    xH = C(x, H)

    # find starting point given 2^k-1 dummy blocks
    y_start = iterate(H, k)

    # now collide from there, like before
    while True:
        y = random_block()
        yH = C(y, y_start)
        if xH == yH and x != y:
            return x, y, xH, yH

def counter(i):
    assert lolhash_blocksz == 16
    return ('%032x'%(i)).decode('hex')

def long_message_collision():
    k = 16 # log_2 blocks

    found = {}
    
    # build a mapping of values in [1,k] to collisions
    H = lolhash_initial
    for i in range(k, 0, -1):
        x, y, xh, yh = lolhash_collide_long(H, i)
        assert C(x, H) == C(y, iterate(H, i))
        found[i] = (x, y, xh, yh)
        H = xh

    # our target message is 2 ** k blocks of counter, say
    intermediates = {}
    M_h = lolhash_initial
    
    for i in xrange(2 ** k):
        M_h = C(counter(i), M_h)
        intermediates[M_h] = i

    # the end of our expandable message
    _, _, target, _ = found[1]

    # find our where we collided internally
    index = intermediates.get(target, None)

    # if we didn't find the end of our expandable message at
    # some intermediate state in the real message, give up
    if index is None:
        return False

    # cross-check: if we can get to the target, ensure
    # we get to M_h having added the rest of M on
    Hv = target
    for i in xrange(index + 1, 2 ** k):
        Hv = C(counter(i), Hv)
    assert Hv == M_h

    print 'colliding block at', index, '...',

    # awesome. now we just need to decompose 'index' bitwise and
    # in doing so, hash either short or long blocks
    collide = lolhash_initial
    left = index

    for i in range(k, 0, -1):
        block_short, block_long, _, _ = found[i]
        long_length = 2 ** (i - 1) + 1
        
        if left - i > long_length:
            # add a long block
            collide = C(block_long, iterate(collide, i))
            left -= long_length
            print 'long',
        else:
            # add a short block
            collide = C(block_short, collide)
            left -= 1
            print 'short',

    # finally, shove on the end of the real message
    for i in xrange(index + 1, 2 ** k):
        collide = C(counter(i), collide)
    
    assert collide == M_h
    print 'win!'
    return True
    
if __name__ == '__main__':
    while not long_message_collision():
        pass
