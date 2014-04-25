ArmA3-RemoteCall ![CC-BY-SA 4.0](http://i.creativecommons.org/l/by-sa/3.0/88x31.png)
================

- Copyright 2014 nano2k, Fank
- Licensed under Creative Commons Attribution-ShareAlike 4.0 International Public License (CC-BY-SA 4.0)


# Protocol specification
Version 1

General Information
--------------------------

- TCP/IP
- Port: adjustable in config file
- Server means the extension.dll
- Client is the Remote-Client


Packet types
--------------------------

**Header:**

`'R'(0x52) 'C'(0x43) | 1 Byte Version | 0xFF`

2. Handshake (Server <- Client):
`HEADER + | 0x00 | password`

3. Handshake Response (Server -> Client):
`HEADER + | 0x01 | (0x00 (ok) | 0x01 (wrong password) | 0x02 (wrong version))`

4. Query content length (Server <- Client);
`HEADER + | 0x10 | 2 byte length`

5. Query content length response (Server -> Client):
`HEADER + | 0x11 | (0x00 (ok) | 0x01 (too short) | 0x02 (too long))`

6. Query (Server <- Client):
`HEADER + | 0x12 | content`

7. Query Response (Server -> Client):
`HEADER + | 0x13 | 2 byte query id`

8. Query Result Response (Server -> Client):
`HEADER + | 0x14 | 2 byte query id | content`
