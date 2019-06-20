#!/usr/bin/python
# orapass.py - implements Oracle's password hashing algorithm
# Author: iphelix
import sys, string, binascii
from Crypto.Cipher import DES

if len(sys.argv) != 3:
  sys.exit("Usage: ./orapass login password")

# Grab user input from command line
username=sys.argv[1]
password=sys.argv[2]

# 1) Concatenate the username and the password to produce a plaintext string
pw = username + password

# 2) Convert the plaintext string to uppercase characters
pw = string.upper(pw)

# 3) Convert the plaintext string to multi-byte storage format; ASCII characters have the
#    high byte set to 0x00
pw = string.join(list(pw),"\x00")
pw = "\x00"+pw

# 4) Encrypt the plaintext string (padded with 0s if necessary to the next even block length)
#    using the DES algorithm in cipher block chaining (CBC) mode with a fixed key value of
#    0x0123456789ABCDEF;
pads = 8 - len(pw)%8
pw += '\x00' * pads

iv = "\x00\x00\x00\x00\x00\x00\x00\x00"
key = "\x01\x23\x45\x67\x89\xAB\xCD\xEF"

cipher=DES.new(key,DES.MODE_CBC,iv)
ciphertext=cipher.encrypt(pw)

# 5) Encrypt the plaintext string again with DES-CBC, but using the last block of the output
#    of the previous step (ignoring parity bits) as the encryption key. 
key = ciphertext[-8:]

cipher=DES.new(key,DES.MODE_CBC,iv)
ciphertext=cipher.encrypt(pw)

# 6) The last block of the output is converted into a printable string to produce the password 
#    hash value.
pwhash = ciphertext[-8:]
print binascii.hexlify(pwhash)

