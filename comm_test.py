#
#   comm_test.py
#
#   David Janes
#   IOTDB.org
#   2014-03-29
#
#   Test the communications code
#

from __future__ import print_function

import select
import sys
import os
import threading
import time

import comm

class CommTest(comm.Comm):
    def __init__(self, *av, **ad):
        comm.Comm.__init__(self, *av, **ad)
        self.listensd = {}
        self.stated = {}
        self.do_timer = False

        self.send("!hello","CommTest")

    def timer(self):
        for pin, sequences in self.listensd.items():
            for sequence in sequences:
                self.send(sequence, int(time.time()) % 100)

        if not self.quit:
            threading.Timer(1, self.timer).start()

    def receive(self, sequence, command, *av):
        if command == "DigitalWrite":
            pin, value = av[0], av[1]
            self.stated[pin] = value and 1 or 0
        elif command == "DigitalRead":
            pin = av[0]
            return self.stated.get(pin) and 1 or 0
        elif command == "DigitalListen":
            pin = av[0]
            self.listensd.setdefault(pin, set()).add(sequence)

            if self.do_timer == False:
                self.do_timer = True
                self.timer()

            return ""
        else:
            raise KeyError("unknown command: %s" % command)
            
        print ("comm_test.receive: seq=%s comm=%s" % ( sequence, command ), file=sys.stderr)

    def stop(self):
        pass

if __name__ == '__main__':
    comm = CommTest(verbose=True)

    print("comm_test: starting", file=sys.stderr)
    comm.run()
