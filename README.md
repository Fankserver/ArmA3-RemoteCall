ArmA3-RemoteCall ![CC-BY-SA 4.0](http://i.creativecommons.org/l/by-sa/3.0/88x31.png)
================

ArmA3-RemoteCall allows server admins to execute arbitrary script code (*.sqf) without restarting the game server. The protocol is designed in such a way that it allows multiple concurrent clients.

- Copyright 2014 Florian "Fank" Kinder, Niko "nano2k" Bochan
- Licensed under Creative Commons Attribution-ShareAlike 4.0 International Public License (CC-BY-SA 4.0)

## Usage

- Copy the remote_call.dll in your ArmA addon folder (for example `@A3RemoteCall`).
- Add the remote_call.fsm to your mission, and add the following lines to your initServer.sqf or init.sqf (this should be called server side only!):
```sqf
_remoteCall = [] execFSM "remote_call.fsm";
```

## Protocol specification
Version 1

### General Information

- TCP/IP
- Port: adjustable in config file
- Server means the extension.dll
- Client is the Remote-Client


### Packet types


**Header:**

`'R'(0x52) 'C'(0x43) | 1 Byte Version | 0xFF`

2. Handshake (Server <- Client):
`HEADER + | 0x00 | password`

3. Handshake Response (Server -> Client):
`HEADER + | 0x01 | (0x00 (ok) | 0x01 (wrong password) | 0x02 (wrong version) | 0x03 (wrong command))`

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

## Supported Client Libraries

- Go [go-remotecall](https://github.com/nano2k/go-remotecall)
