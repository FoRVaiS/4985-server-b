#!/bin/bash

# account create
echo -ne '\x0D\x02\x00\x00\x00\x16\x0C\x07\x54\x65\x73\x74\x69\x6E\x67\x0C\x0B\x50\x61\x73\x73\x77\x6F\x72\x64\x31\x32\x33' | nc 127.0.0.1 8081  | hexdump -C

# account login
echo -ne '\x0A\x02\x00\x00\x00\x16\x0C\x07\x54\x65\x73\x74\x69\x6E\x67\x0C\x0B\x50\x61\x73\x73\x77\x6F\x72\x64\x31\x32\x33' | nc 127.0.0.1 8081  | hexdump -C

# account login wrong password
echo -ne '\x0A\x02\x00\x00\x00\x16\x0C\x07\x54\x65\x73\x74\x69\x6E\x67\x0C\x0B\x50\x61\x73\x73\x77\x6F\x72\x64\x31\x32\x31' | nc 127.0.0.1 8081  | hexdump -C

# account logout
echo -ne '\x0C\x02\x00\x01\x00\x00' | nc 127.0.0.1 8081 | hexdump -C

# account create Tia
echo -ne '\x0D\x02\x00\x00\x00\x12\x0C\x03\x54\x69\x61\x0C\x0B\x50\x61\x73\x73\x77\x6F\x72\x64\x31\x32\x33' | nc 127.0.0.1 8081  | hexdump -C

# account login Tia
echo -ne '\x0A\x02\x00\x00\x00\x12\x0C\x03\x54\x69\x61\x0C\x0B\x50\x61\x73\x73\x77\x6F\x72\x64\x31\x32\x33' | nc 127.0.0.1 8081  | hexdump -C

# chat
echo -ne '\x14\x02\x00\x01\x00\x1E\x18\x0F\x32\x30\x32\x34\x30\x33\x30\x31\x31\x32\x33\x30\x34\x35\x5A\x0C\x02\x68\x69\x0C\x07\x54\x65\x73\x74\x69\x6E\x67' | nc 127.0.0.1 8081  | hexdump -C

# chat invalid
echo -ne '\x15\x02\x00\x01\x00\x1E\x18\x0F\x32\x30\x32\x34\x30\x33\x30\x31\x31\x32\x33\x30\x34\x35\x5A\x0C\x02\x68\x69\x0C\x07\x54\x65\x73\x74\x69\x6E\x67' | nc 127.0.0.1 8081  | hexdump -C

