#!/usr/bin/python -u
import sys, socket

if len(sys.argv) != 5:
    print "Usage: {} server port repeat code".format(sys.argv[0])
    exit(1)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(10)
s.connect((sys.argv[1], int(sys.argv[2])))
code = 'SND {}\n'.format(sys.argv[4])
for i in range(0, int(sys.argv[3])):
    print "Sending code"
    s.send(code)
    print s.recv(16),
s.close()

