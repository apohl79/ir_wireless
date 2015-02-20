#!/usr/bin/python -u
import sys, os, socket, stat

srv = 'localhost'
port = 99
script_dir = '/opt/ir_wireless/scripts'

if len(sys.argv) != 2:
    print "Usage: {} name".format(sys.argv[0])
    exit(1)

name = sys.argv[1]

print "Receiving code"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((srv, port))
s.send('RCV\n')
code = s.recv(1024)
s.close()

file = '{}/{}'.format(script_dir, name)
with open(file, "w") as f:
    f.write('/usr/bin/python -u {}/send.pyc {} {} 1 {}\n'.format(script_dir, srv, port, code.strip()))
    f.close()
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

print "Done"

