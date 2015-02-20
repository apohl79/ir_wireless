#!/usr/bin/python -u
import sys, socket, threading

irctrl_clnts = list()
irctrl_clnts_lock = threading.Lock()

def ir_server():
    master = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    master.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    master.bind((socket.gethostname(), 98))
    master.listen(8)
    while True:
        (clnt, addr) = master.accept()
        print "IR micro controller connected from {}".format(addr)
        irctrl_clnts_lock.acquire()
        irctrl_clnts.append(clnt)
        irctrl_clnts_lock.release()

def ctrl_server():
    master = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    master.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    master.bind(("localhost", 99))
    master.listen(8)
    while True:
        (clnt, addr) = master.accept()
        threading.Thread(target = lambda: ctrl_handler(clnt)).start()

def ctrl_handler(clnt):
    data = clnt.recv(1024)
    print "Sending command to {} IR controllers".format(len(irctrl_clnts))
    error_clnts = list()
    irctrl_clnts_lock.acquire()
    for irclnt in irctrl_clnts:
        print irclnt
        try:
            print "<- {}".format(data),
            irclnt.send(data)
            ret = irclnt.recv(16)
            print "-> {}".format(ret),
            clnt.send(ret)
        except:
            print "error for {}".format(irclnt)
            irclnt.close()
            error_clnts.append(irclnt)
    for irclnt in error_clnts:
        irctrl_clnts.remove(irclnt)
    irctrl_clnts_lock.release()
    clnt.close()

# IR server to be connected by the micro controller
print "Staring IR server"
ir_thr = threading.Thread(target = lambda: ir_server())
ir_thr.start()
print "Staring CTRL server"
# Control server to be connected by the send script/openhab
ctrl_thr = threading.Thread(target = lambda: ctrl_server())
ctrl_thr.start()

ir_thr.join()
ctrl_thr.join()
