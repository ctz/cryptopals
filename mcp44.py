import dsa, rsa
from hashlib import sha1

p = 0x800000000000000089e1855218a0e7dac38136ffafa72eda7859f2171e25e65eac698c1702578b07dc2a1076da241c76c62d374d8389ea5aeffd3226a0530cc565f3bf6b50929139ebeac04f48c3c84afb796d61e5a4f9a8fda812ab59494232c7d2b4deb50aa18ee9e132bfa85ac4374d7f9091abc3d015efc871a584471bb1
q = 0xf4f47f05794b256174bba6e9b396a7707e563c5b
g = 0x5958c9d3898b224b12672c0b98e06c60df923cb8bc999d119458fef538b8fa4046c8db53039db620c094c9fa077ef389b5322a559946a71903f990f1f7e0e025e2d7f7cf494aff1a0470f5b64c36b625a097f1651fe775323556fe00b3608c887892878480e99041be601a62166ca6894bdd41a7054ec89f756ba9fc95302291

group = dsa.group(p, q, g)

y = 0x2d026f4bf30195ede3a088da85e398ef869611d0f68f0713d51c9c1a3a26c95105d915e2d8cdf26d056b86b8a7b85519b1c23cc3ecdc6062650462e3063bd179c2a6581519f674a61f1d89a1fff27171ebc1b93d4dc57bceb7ae2430f98a6a4d83d8279ee65d71c1203d2c96d65ebbf7cce9d32971c3de5084cce04a2e147821
hash_x = 'ca8f6f7c66fa362d40760d135b763eb8527d3d52'

sigs = [
  dict(msg = 'Listen for me, you better listen for me now. ',
       s = 1267396447369736888040262262183731677867615804316,
       r = 1105520928110492191417703162650245113664610474875,
       m = 'a4db3de27e2db3e5ef085ced2bced91b82e0df19'),
       
  dict(msg = 'Listen for me, you better listen for me now. ',
       s = 29097472083055673620219739525237952924429516683,
       r = 51241962016175933742870323080382366896234169532,
       m = 'a4db3de27e2db3e5ef085ced2bced91b82e0df19'),
       
  dict(msg = 'When me rockin\' the microphone me rock on steady, ',
       s = 277954141006005142760672187124679727147013405915,
       r = 228998983350752111397582948403934722619745721541,
       m = '21194f72fe39a80c9c20689b8cf6ce9b0e7e52d4'),
       
  dict(msg = 'Yes a Daddy me Snow me are de article dan. ',
       s = 1013310051748123261520038320957902085950122277350,
       r = 1099349585689717635654222811555852075108857446485,
       m = '1d7aaaa05d2dee2f7dabdc6fa70b6ddab9c051c5'),
       
  dict(msg = 'But in a in an\' a out de dance em ',
       s = 203941148183364719753516612269608665183595279549,
       r = 425320991325990345751346113277224109611205133736,
       m = '6bc188db6e9e6c7d796f7fdd7fa411776d7a9ff'),
       
  dict(msg = 'Aye say where you come from a, ',
       s = 502033987625712840101435170279955665681605114553,
       r = 486260321619055468276539425880393574698069264007,
       m = '5ff4d4e8be2f8aae8a5bfaabf7408bd7628f43c9'),
       
  dict(msg = 'People em say ya come from Jamaica, ',
       s = 1133410958677785175751131958546453870649059955513,
       r = 537050122560927032962561247064393639163940220795,
       m = '7d9abd18bbecdaa93650ecc4da1b9fcae911412'),
       
  dict(msg = 'But me born an\' raised in the ghetto that I want yas to know, ',
       s = 559339368782867010304266546527989050544914568162,
       r = 826843595826780327326695197394862356805575316699,
       m = '88b9e184393408b133efef59fcef85576d69e249'),
       
  dict(msg = 'Pure black people mon is all I mon know. ',
       s = 1021643638653719618255840562522049391608552714967,
       r = 1105520928110492191417703162650245113664610474875,
       m = 'd22804c4899b522b23eda34d2137cd8cc22b9ce8'),
       
  dict(msg = 'Yeah me shoes a an tear up an\' now me toes is a show a ',
       s = 506591325247687166499867321330657300306462367256,
       r = 51241962016175933742870323080382366896234169532,
       m = 'bc7ec371d951977cba10381da08fe934dea80314'),
       
  dict(msg = 'Where me a born in are de one Toronto, so ',
       s = 458429062067186207052865988429747640462282138703,
       r = 228998983350752111397582948403934722619745721541,
       m = 'd6340bfcda59b6b75b59ca634813d572de800e8f')
]

if __name__ == '__main__':
    pub = (group, y)
    
    # check signatures for sanity
    for s in sigs:
        sig = (s['r'], s['s'])
        dsa.verify_sha1(pub, sig, s['msg'])
    
    # look at all pairs for signatures and see if key falls out
    for i1, d1 in enumerate(sigs):
        for i2, d2 in enumerate(sigs):
            if i1 == i2:
                continue
            m1 = dsa.hash(d1['msg'])
            m2 = dsa.hash(d2['msg'])
            s1 = d1['s']
            s2 = d2['s']
            
            sd = (s1 - s2) % group.q
            invsd = rsa.invmod(sd, group.q)
            k = (((m1 - m2) % group.q) * invsd) % group.q
            
            x = dsa.recover_x_given_sig_k(group, k, (d1['r'], d1['s']), d1['msg'])
            
            if dsa.sha1('%x' % x).hexdigest() == hash_x:
                print 'i1: %d, i2: %d, k: 0x%x, x: 0x%x' % (i1, i2, k, x)
                exit(0)