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

def pad_for_message_length(l):
    return 'length:%d' % (l)

def split_blocks(m):
    for i in range(0, len(m), lolhash_blocksz):
        yield m[i:i+lolhash_blocksz]

def pad(m):
    for b in split_blocks(m):
        yield b
    yield pad_for_message_length(len(m))

def lolhash(m):
    H = lolhash_nopad(m)
    return C(pad_for_message_length(len(m)), H)

def lolhash_nopad(m):
    H = lolhash_initial

    for block in split_blocks(m):
        H = C(block, H)
    return H

def random_block():
    return ''.join(chr(random.getrandbits(8)) for i in range(lolhash_blocksz))

def lolhash_find_block(x_start, y_start):
    # find a pair of blocks X,Y which H(X, x_start) == H(Y, y_start)
    x = random_block()
    xH = C(x, x_start)

    # now collide from there, like before
    while True:
        y = random_block()
        yH = C(y, y_start)
        if xH == yH and x != y:
            return x, y, xH

def next_level(states):
    level = []
    for x, y in zip(states[::2], states[1::2]):
        x, _, _ = x
        y, _, _ = y
        msgx, msgy, next = lolhash_find_block(x, y)
        level.append((next, msgx, msgy))
    return level

def find_glue_block(preglue, index):
    while True:
        candidate = random_block()
        h = C(candidate, preglue)

        i = index.get(h, None)
        if i is not None:
            return i, candidate, h

def commitment_collision(k):
    # initial states
    initial = []
    for _ in xrange(2 ** k):
        x = C(random_block(), lolhash_initial)
        initial.append((x, None, None))

    # build our tree level by level    
    tree = [ initial ]

    while len(tree[-1]) != 1:
        tree.append(next_level(tree[-1]))

    # index the bottom of the tree, which we collide into
    index = dict((v[0], i) for i, v in enumerate(tree[0]))

    # make commitment from root of tree
    message_length = 32
    glue_block = lolhash_blocksz
    preimage_length = message_length + glue_block + lolhash_blocksz * k
    precommitment = tree[-1][0][0] # unpadded
    commitment = C(pad_for_message_length(preimage_length), precommitment)
    print 'commitment:', commitment.encode('hex'),
    
    # decide on message
    message = 'Arsenal: 0,  Manchester City: 2.'
    assert message_length == len(message)

    # find a glue block into index
    preglue = lolhash_nopad(message)
    root_index, glue, hash_here = find_glue_block(preglue, index)

    # now find the suffix blocks
    blocks = list(split_blocks(message)) + [ glue ]
    prev_index = root_index

    for i in range(1, len(tree)):
        # choose which node to look at based on the previous,
        # and the message block likewise
        index_here = prev_index // 2
        left_right = prev_index % 2

        blocks.append(tree[i][index_here][1 + left_right])

        prev_index = index_here

    # win!
    message = ''.join(blocks)
    assert lolhash(message) == commitment
    print 'message', repr(message), '=', lolhash(message).encode('hex')
    
if __name__ == '__main__':
    commitment_collision(4)
