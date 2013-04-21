#!/bin/sh

i=0

function ensure()
{
  if [ "$1" != "$2" ] ; then
    echo "test failed:"
    echo "  '$1'"
    echo "is not the same as:"
    echo "  '$2'"
    exit 1
  fi
  
  i=$(($i + 1))
  echo "test $i: OK"
}

# // ------------------------------------------------------------
# 
# 1. Convert hex to base64 and back.
# 
# The string:
# 
#   49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d
# 
# should produce:
# 
#   SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t
# 
# Now use this code everywhere for the rest of the exercises. Here's a
# simple rule of thumb:
# 
#   Always operate on raw bytes, never on encoded strings. Only use hex
#   and base64 for pretty-printing.
# 
ensure `bin/mcp1 49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d` "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t"

# // ------------------------------------------------------------
# 
# 2. Fixed XOR
# 
# Write a function that takes two equal-length buffers and produces
# their XOR sum.
# 
# The string:
# 
#  1c0111001f010100061a024b53535009181c
# 
# ... after hex decoding, when xor'd against:
# 
#  686974207468652062756c6c277320657965
# 
# ... should produce:
# 
#  746865206b696420646f6e277420706c6179
# 
ensure `bin/mcp2 1c0111001f010100061a024b53535009181c 686974207468652062756c6c277320657965` "746865206b696420646f6e277420706c6179"

# // ------------------------------------------------------------
# 
# 3. Single-character XOR Cipher
# 
# The hex encoded string:
# 
#       1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736
# 
# ... has been XOR'd against a single character. Find the key, decrypt
# the message.
# 
# Write code to do this for you. How? Devise some method for "scoring" a
# piece of English plaintext. (Character frequency is a good metric.)
# Evaluate each output and choose the one with the best score.
# 
# Tune your algorithm until this works.
# 
ensure "`bin/mcp3 1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736`" "key: 58, msg: Cooking MC's like a pound of bacon"

# // ------------------------------------------------------------
# 
# 4. Detect single-character XOR
# 
# One of the 60-character strings at:
# 
#   https://gist.github.com/3132713
# 
# has been encrypted by single-character XOR. Find it. (Your code from
# #3 should help.)
# 
inp=`cat gistfile1.txt`
ensure "`bin/mcp4 $inp`" "cipher: 7b5a4215415d544115415d5015455447414c155c46155f4058455c5b523f, key: 35, msg: Now that the party is jumping."

# // ------------------------------------------------------------
# 
# 5. Repeating-key XOR Cipher
# 
# Write the code to encrypt the string:
# 
#   Burning 'em, if you ain't quick and nimble
#   I go crazy when I hear a cymbal
# 
# Under the key "ICE", using repeating-key XOR. It should come out to:
# 
#   0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f
# 
# Encrypt a bunch of stuff using your repeating-key XOR function. Get a
# feel for it.
# 
PLAIN="4275726e696e672027656d2c20696620796f752061696e277420717569636b20616e64206e696d626c650a4920676f206372617a79207768656e2049206865617220612063796d62616c"
ICE="494345"
ensure `bin/mcp2 $PLAIN $ICE` "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f"

# // ------------------------------------------------------------
# 
# 6. Break repeating-key XOR
# 
# The buffer at the following location:
# 
#  https://gist.github.com/3132752
# 
# is base64-encoded repeating-key XOR. Break it.
# 
# Here's how:
# 
# a. Let KEYSIZE be the guessed length of the key; try values from 2 to
# (say) 40.
# 
# b. Write a function to compute the edit distance/Hamming distance
# between two strings. The Hamming distance is just the number of
# differing bits. The distance between:
# 
#   this is a test
# 
# and:
# 
#   wokka wokka!!!
# 
# is 37.
# 
# c. For each KEYSIZE, take the FIRST KEYSIZE worth of bytes, and the
# SECOND KEYSIZE worth of bytes, and find the edit distance between
# them. Normalize this result by dividing by KEYSIZE.
# 
# d. The KEYSIZE with the smallest normalized edit distance is probably
# the key. You could proceed perhaps with the smallest 2-3 KEYSIZE
# values. Or take 4 KEYSIZE blocks instead of 2 and average the
# distances.
# 
# e. Now that you probably know the KEYSIZE: break the ciphertext into
# blocks of KEYSIZE length.
# 
# f. Now transpose the blocks: make a block that is the first byte of
# every block, and a block that is the second byte of every block, and
# so on.
# 
# g. Solve each block as if it was single-character XOR. You already
# have code to do this.
# 
# e. For each block, the single-byte XOR key that produces the best
# looking histogram is the repeating-key XOR key byte for that
# block. Put them together and you have the key.
# 
inp=`cat gistfile2.txt`
ans="key: 5465726d696e61746f7220583a204272696e6720746865206e6f697365, plain: I'm back and I'm ringin' the bell .A rockin' on the mike while the fly girls yell .In ecstasy in the back of me .Well that's my DJ Deshay cuttin' all them Z's .Hittin' hard and the girlies goin' crazy .Vanilla's on the mike, man I'm not lazy. ..I'm lettin' my drug kick in .It controls my mouth and I begin .To just let it flow, let my concepts go .My posse's to the side yellin', Go Vanilla Go! ..Smooth 'cause that's the way I will be .And if you don't give a damn, then .Why you starin' at me .So get off 'cause I control the stage .There's no dissin' allowed .I'm in my own phase .The girlies sa y they love me and that is ok .And I can dance better than any kid n' play ..Stage 2 -- Yea the one ya' wanna listen to .It's off my head so let the beat play through .So I can funk it up and make it sound good .1-2-3 Yo -- Knock on some wood .For good luck, I like my rhymes atrocious .Supercalafragilisticexpialidocious .I'm an effect and that you can bet .I can take a fly girl and make her wet. ..I'm like Samson -- Samson to Delilah .There's no denyin', You can try to hang .But you'll keep tryin' to get my style .Over and over, practice makes perfect .But not if you're a loafer. ..You'll get nowhere, no place, no time, no girls .Soon -- Oh my God, homebody, you probably eat .Spaghetti with a spoon! Come on and say it! ..VIP. Vanilla Ice yep, yep, I'm comin' hard like a rhino .Intoxicating so you stagger like a wino .So punks stop trying and girl stop cryin' .Vanilla Ice is sellin' and you people are buyin' .'Cause why the freaks are jockin' like Crazy Glue .Movin' and groovin' trying to sing along .All through the ghetto groovin' this here song .Now you're amazed by the VIP posse. ..Steppin' so hard like a German Nazi .Startled by the bases hittin' ground .There's no trippin' on mine, I'm just gettin' down .Sparkamatic, I'm hangin' tight like a fanatic .You trapped me once and I thought that .You might have it .So step down and lend me your ear .'89 in my time! You, '90 is my year. ..You're weakenin' fast, YO! and I can tell it .Your body's gettin' hot, so, so I can smell it .So don't be mad and don't be sad .'Cause the lyrics belong to ICE, You can call me Dad .You're pitchin' a fit, so step back and endure .Let the witch doctor, Ice, do the dance to cure .So come up close and don't be square .You wanna battle me -- Anytime, anywhere ..You thought that I was weak, Boy, you're dead wrong .So come on, everybody and sing this song ..Say -- Play that funky music Say, go white boy, go white boy go .play that funky music Go white boy, go white boy, go .Lay down and boogie and play that funky music till you die. ..Play that funky music Come on, Come on, let me hear .Play that funky music white boy you say it, say it .Play that funky music A little louder now .Play that funky music, white boy Come on, Come on, Come on .Play that funky music ."
ensure "`bin/mcp6 "$inp"`" "$ans"

# // ------------------------------------------------------------
# 
# 7. AES in ECB Mode
# 
# The Base64-encoded content at the following location:
# 
#     https://gist.github.com/3132853
# 
# Has been encrypted via AES-128 in ECB mode under the key
# 
#     "YELLOW SUBMARINE".
# 
# (I like "YELLOW SUBMARINE" because it's exactly 16 bytes long).
# 
# Decrypt it.
# 
# Easiest way:
# 
# Use OpenSSL::Cipher and give it AES-128-ECB as the cipher.
# 
inp=`cat gistfile3.txt`
ans="I'm back and I'm ringin' the bell .A rockin' on the mike while the fly girls yell .In ecstasy in the back of me .Well that's my DJ Deshay cuttin' all them Z's .Hittin' hard and the girlies goin' crazy .Vanilla's on the mike, man I'm not lazy. ..I'm lettin' my drug kick in .It controls my mouth and I begin .To just let it flow, let my concepts go .My posse's to the side yellin', Go Vanilla Go! ..Smooth 'cause that's the way I will be .And if you don't give a damn, then .Why you starin' at me .So get off 'cause I control the stage .There's no dissin' allowed .I'm in my own phase .The girlies sa y they love me and that is ok .And I can dance better than any kid n' play ..Stage 2 -- Yea the one ya' wanna listen to .It's off my head so let the beat play through .So I can funk it up and make it sound good .1-2-3 Yo -- Knock on some wood .For good luck, I like my rhymes atrocious .Supercalafragilisticexpialidocious .I'm an effect and that you can bet .I can take a fly girl and make her wet. ..I'm like Samson -- Samson to Delilah .There's no denyin', You can try to hang .But you'll keep tryin' to get my style .Over and over, practice makes perfect .But not if you're a loafer. ..You'll get nowhere, no place, no time, no girls .Soon -- Oh my God, homebody, you probably eat .Spaghetti with a spoon! Come on and say it! ..VIP. Vanilla Ice yep, yep, I'm comin' hard like a rhino .Intoxicating so you stagger like a wino .So punks stop trying and girl stop cryin' .Vanilla Ice is sellin' and you people are buyin' .'Cause why the freaks are jockin' like Crazy Glue .Movin' and groovin' trying to sing along .All through the ghetto groovin' this here song .Now you're amazed by the VIP posse. ..Steppin' so hard like a German Nazi .Startled by the bases hittin' ground .There's no trippin' on mine, I'm just gettin' down .Sparkamatic, I'm hangin' tight like a fanatic .You trapped me once and I thought that .You might have it .So step down and lend me your ear .'89 in my time! You, '90 is my year. ..You're weakenin' fast, YO! and I can tell it .Your body's gettin' hot, so, so I can smell it .So don't be mad and don't be sad .'Cause the lyrics belong to ICE, You can call me Dad .You're pitchin' a fit, so step back and endure .Let the witch doctor, Ice, do the dance to cure .So come up close and don't be square .You wanna battle me -- Anytime, anywhere ..You thought that I was weak, Boy, you're dead wrong .So come on, everybody and sing this song ..Say -- Play that funky music Say, go white boy, go white boy go .play that funky music Go white boy, go white boy, go .Lay down and boogie and play that funky music till you die. ..Play that funky music Come on, Come on, let me hear .Play that funky music white boy you say it, say it .Play that funky music A little louder now .Play that funky music, white boy Come on, Come on, Come on .Play that funky music ....."
ensure "`bin/mcp7 "$inp" "YELLOW SUBMARINE"`" "$ans"

# // ------------------------------------------------------------
# 
# 8. Detecting ECB
# 
# At the following URL are a bunch of hex-encoded ciphertexts:
# 
#    https://gist.github.com/3132928
# 
# One of them is ECB encrypted. Detect it.
# 
# Remember that the problem with ECB is that it is stateless and
# deterministic; the same 16 byte plaintext block will always produce
# the same 16 byte ciphertext.
# 
inp=`cat gistfile4.txt`
ans="index: 132, cipher: d880619740a8a19b7840a8a31c810a3d08649af70dc06f4fd5d2d69c744cd283e2dd052f6b641dbf9d11b0348542bb5708649af70dc06f4fd5d2d69c744cd2839475c9dfdbc1d46597949d9c7e82bf5a08649af70dc06f4fd5d2d69c744cd28397a93eab8d6aecd566489154789a6b0308649af70dc06f4fd5d2d69c744cd283d403180c98c8f6db1f2a3f9c4040deb0ab51b29933f2c123c58386b06fba186a"
ensure "`bin/mcp8 $inp`" "$ans"
