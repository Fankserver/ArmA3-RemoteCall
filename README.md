ArmA3-RemoteCall
================

Copyright 2014 nano2k, Fank


# Protokoll-Spezifikation
Version 1

Allgemeine Informationen
--------------------------

- TCP/IP-basiert
- Port: ArmA-Gameserver-Port + 6
- Mit "Server" ist die RV-Extension ("remotecall.dll") gemeint
- Mit "Client" ist der Remote-Client gemeint


Paket-Typen
--------------------------

**Header:**

`'R'(0x52) 'C'(0x43) | 1 Byte Version | 0xFF`

2. Handshake (Server <- Client):
`HEADER + | 0x00 | password`

3. Handshake Response (Server -> Client):
`HEADER + | 0x01 | (0x00 (ok) | 0x01 (wrong password) | 0x02 (wrong version))`

4. Query (Server <- Client):
`HEADER + | 0x10 | content`

5. Query Response (Server -> Client):
`HEADER + | 0x11 | 2 byte query id`

6. Query Result Response (Server -> Client):
`HEADER + | 0x12 | 2 byte query id | content`