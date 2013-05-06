import rsa
import sys

def pkcs1_oracle(priv, cipher):
    plain = rsa.raw_decrypt(priv, cipher)
    N = priv[1]
    modlen = rsa.bit_len(N)
    topbyte = (plain >> (modlen - 8)) & 0xff
    topbyte2 = (plain >> (modlen - 16)) & 0xff
    return topbyte == 0x00 and topbyte2 == 0x02

if __name__ == '__main__':
    modsz = int(sys.argv[1])
    pub, priv = rsa.gen_rsa(modsz, 3)
    e, n = pub
    n_bytes = modsz / 8
    B = 2 ** (8 * (n_bytes - 2))

    pt = 'kick it, CC'.encode('hex')
    pad = 'af' * (n_bytes - 3 - len(pt) / 2)
    msg = '0002' + pad + '00' + pt
    msg = long(msg, 16)
    ct = rsa.raw_encrypt(pub, msg)
    assert pkcs1_oracle(priv, ct)

    # don't need to do blinding here
    i = 1
    M0 = [(2 * B, 3 * B - 1)]
    s0 = 1
    c0 = (ct * rsa.raw_encrypt(pub, s0)) % n

    def attempt(s):
        return pkcs1_oracle(priv, (c0 * rsa.raw_encrypt(pub, s)) % n)

    def ceil_div(a, b):
        return (a + b - 1) // b

    def floor_div(a, b):
        return a // b

    def check_range(A):
        for a, b in A:
            assert a <= b

    def intersect(A, B):
        Ai = 0
        Bi = 0
        check_range(A)
        check_range(B)
        out = []
        while Ai < len(A) and Bi < len(B):
            u, v = A[Ai]
            x, y = B[Bi]

            # B contains larger range; need to swap
            if x < u:
                A, B = B, A
                Ai, Bi = Bi, Ai
                u, v, x, y = x, y, u, v

            # B entirely contained within A
            if u <= x <= y <= v:
                out.append((x, y))
                Bi += 1
                continue

            # disjoint
            if v <= x:
                Ai += 1
                continue

            out.append((x, v))
            Ai += 1

        check_range(out)
        return out

    def search_linear(start):
        si = start
        while True:
            if attempt(si):
                return si
            si += 1

    def search_start():
        return search_linear(ceil_div(n, 3 * B))

    def search_multi(si_1):
        return search_linear(si_1 + 1)

    def search_single(M, si_1):
        assert len(M) == 1
        a, b = M[0]

        ri = ceil_div(2 * (b * si_1 - 2 * B), n)
        while True:
            si = (2 * B + ri * n) / b
            simax = (3 * B + ri * n) / a
            while si <= simax:
                if attempt(si):
                    return si
                si += 1
            ri += 1

    def search(i, M, si_1):
        if i == 1:
            return search_start()
        elif len(M) > 1:
            return search_multi(si_1)
        else:
            return search_single(M, si_1)

    def narrow(M, si):
        M_out = []

        for a, b in M:
            rmin = (a * si - 3 * B + 1) / n
            rmax = (b * si - 2 * B) / n
            assert rmin <= rmax
            
            r = rmin
            while r <= rmax:
                ac = ceil_div(2 * B + r * n, si)
                bc = floor_div(3 * B - 1 + r * n, si)
                M_out.append((ac, bc))
                    
                r += 1
                
        return intersect(M, M_out)

    s_i = s0
    M_i = M0
    i = 0

    while True:
        s_i = search(i, M_i, s_i)
        #print 's_' + str(i), s_i
        M_i = narrow(M_i, s_i)
        #print 'M_' + str(i), M_i

        if len(M_i) == 1:
            dd = M_i[0][1] - M_i[0][0]
            if dd == 1:
                break
        i += 1

    recovered_a, recovered_b = M_i[0]
    assert recovered_a == msg or recovered_b == msg
    print 'ok'
