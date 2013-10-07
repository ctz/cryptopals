import aes
import os

shared_key = '0123456789abcdef'

def aes_cbcmac(key, msg):
    return aes.encryptData(key, msg, iv = '\0' * 16)[-16:]

def parse_qs(v):
    parts = v.split('&')
    d = dict()
    for p in parts:
        if len(p) == 0:
            continue
        assert '=' in p
        k, v = p.split('=', 1)
        d[k] = v
    return d

def server_verify(msg):
    msg, mac = msg[:-16], msg[-16:]
    if aes_cbcmac(shared_key, msg) != mac:
        print 'verify failed'
        return None
    else:
        data = parse_qs(msg)
        fromid = int(data['from'])
        txns = data['tx_list'].split(';')
        pairs = [txn.split(':', 1) for txn in txns]
        return [{'to': int(to), 'amount': int(amount), 'from': fromid} for to, amount in pairs]

def print_txns(txns):
    descriptions = []
    for txn in txns:
        descriptions.append('%(amount)d spacebucks from account %(from)d to account %(to)d' % txn)
    print 'transfers:', ', '.join(descriptions)

def client_sign(fromid, txns):
    # txns is a map toid -> amount
    txns = ';'.join('%d:%d' % txn for txn in txns.items())
    msg = 'from=%d&tx_list=%s' % (fromid, txns)
    return msg + aes_cbcmac(shared_key, msg)

def xor_block(a, b):
    assert len(a) == len(b)
    return ''.join(chr(ord(a[i]) ^ ord(b[i])) for i in range(len(a)))

if __name__ == '__main__':
    MY_ACCOUNT = 1
    THEIR_ACCOUNT = 2
    OTHER_ACCOUNT = 3

    # this is our target message
    msg = client_sign(THEIR_ACCOUNT, {OTHER_ACCOUNT: 10})
    
    # check it verifies
    assert server_verify(msg)

    # unpick and repad their message
    msg_raw, msg_mac = msg[:-16], msg[-16:]
    npad = 16 - (len(msg_raw) % 16)
    pad = chr(npad) * npad

    msg_raw_padded = msg_raw + pad

    # build an appendix that we want to add (at least one block)
    appendix = ';%d:%d&&&&&&' % (MY_ACCOUNT, 1000000)
    new_tag = aes_cbcmac(shared_key, xor_block(msg_mac, appendix))

    attack_msg = msg_raw_padded + appendix + new_tag
    txns = server_verify(attack_msg)
    assert txns is not None
    print_txns(txns)
    
