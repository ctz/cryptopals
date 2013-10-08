import hashlib
import os

f_length = 2 # bytes
g_length = 3
fg_blocksz = 16
f_initial = 'lo'
g_initial = 'lol'

def C_f(m, H):
    r = hashlib.md5(H + m).digest()[:f_length]
    return r

def C_g(m, H):
    # just truncated md5
    r = hashlib.md5(H + m).digest()[:g_length]
    return r

def pad(m):
    for i in range(0, len(m), fg_blocksz):
        yield m[i:i+fg_blocksz]
    yield 'length:%d' % (len(m))

def generic_h(m, initial, C):
    H = initial

    for block in pad(m):
        H = C(block, H)
    return H

def f(m):
    return generic_h(m, f_initial, C_f)
def g(m):
    return generic_h(m, g_initial, C_g)
def h(m):
    return f(m) + g(m)

h_collide_calls = 0

def internal_collide(H, C):
    global h_collide_calls
    h_collide_calls += 1
    
    x = os.urandom(fg_blocksz)

    xH = C(x, H)
    while True:
        y = os.urandom(fg_blocksz)
        yH = C(y, H)
        if xH == yH and x != y:
            return x, y, xH, yH

def crosscheck(found, H):
    # check leftmost and rightmost leaf messages hash to same thing under H
    left = ''.join(x[0] for x in found)
    right = ''.join(x[1] for x in found)
    assert H(left) == H(right)

def generate_messages(found):
    if len(found) == 0:
        yield ''
    else:
        x, y = found[0]
        for suffixes in generate_messages(found[1:]):
            yield x + suffixes
            yield y + suffixes

def find_dual_collision():
    H_f = f_initial
    found = []
    for i in range(g_length * 8):
        # generate a big block of colliding messages in f
        x, y, xh, yh = internal_collide(H_f, C_f)
        found.append((x, y))
        H_f = xh

    crosscheck(found, f)

    # now compute g for all our f-colliding messages, and
    # notice if we find a collision
    check = {}
    for msg in generate_messages(found):
        gh = g(msg)

        collision = check.setdefault(gh, msg)
        if collision != msg:
            assert h(collision) == h(msg)
            print 'found collision after', len(check), 'g tests'
            break
    
if __name__ == '__main__':
    find_dual_collision()
