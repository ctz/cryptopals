class rc4:
    def __init__(self, key):
        self.S = [x for x in range(256)]
        j = 0
        for i in range(256):
            j = (j + self.S[i] + key[i % len(key)]) & 0xff
            self.swap(i, j)
        self.i = 0
        self.j = 0

    def swap(self, i, j):
            self.S[i], self.S[j] = self.S[j], self.S[i]

    def encrypt(self, pt):
        ct = []

        for p in pt:
            self.i = (self.i + 1) & 0xff
            self.j = (self.j + self.S[self.i]) & 0xff
            self.swap(self.i, self.j)
            k = self.S[(self.S[self.i] + self.S[self.j]) & 0xff]
            ct.append(p ^ k)

        return bytes(ct)

    def decrypt(self, ct):
        return self.encrypt(ct)

def run_tests():
    def test(key, plaintext, ciphertext):
        key = bytes(key, 'ASCII')
        plaintext = bytes(plaintext, 'ASCII')
        ciphertext = bytes(ciphertext)

        assert ciphertext == rc4(key).encrypt(plaintext)
        assert plaintext == rc4(key).decrypt(ciphertext)

    test('Key', 'Plaintext', [0xbb, 0xf3, 0x16, 0xe8, 0xd9, 0x40, 0xaf, 0x0a, 0xd3])
    test('Wiki', 'pedia', [0x10, 0x21, 0xbf, 0x04, 0x20])
    test('Secret', 'Attack at dawn', [0x45, 0xa0, 0x1f, 0x64, 0x5f, 0xc3, 0x5b, 0x38, 0x35, 0x52, 0x54, 0x4b, 0x9b, 0xf5])

if __name__ == '__main__':
    run_tests()
