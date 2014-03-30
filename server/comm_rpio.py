#
#   comm_rpio.py
#
#   David Janes
#   IOTDB.org
#   2014-03-29
#
#   Control a Raspberry PI board using stdin / stdout
#   and the RPIO library
#

from __future__ import print_function

import select
import sys
import os

import RPIO

import comm

class CommRPIO(comm.Comm):
    def __init__(self, *av, **ad):
        comm.Comm.__init__(self, *av, **ad)
        self.board = 0
        self.listensd = {}
        self.modeld = {}
        self.pind = {}

        self.send("!hello","CommRPIO")

    def _setup_pin(self, pin, pin_type):
        if self.pind.get(pin):
            return

        self.pind[pin] = True
        RPIO.setup(pin, pin_type)

    def receive(self, sequence, command, *av):
        if command == "DigitalWrite":
            pin, value = av[0], av[1]

            self._setup_pin(pin, RPIO.OUT)
            RPIO.output(pin, value and True or False)
        elif command == "DigitalRead":
            pin = av[0]

            self._setup_pin(pin, RPIO.IN)
            return RPIO.input(pin) and 1 or 0
            ## return pifacedigitalio.digital_read(pin, self.board)
        elif command == "DigitalListen":
            pin = av[0]
            self.listensd.setdefault(pin, set()).add(sequence)

            RPIO.add_interrupt_callback(pin, send._event)
        else:
            raise KeyError("unknown command: %s" % command)
            
        print ("comm_piface.receive: seq=%s comm=%s" % ( sequence, command ), file=sys.stderr)

    def stop(self):
        pass

    def _event(self, pin, value):
        listens = self.listensd.get(pin)
        if not listens:
            return

        for sequence in listens:
            self.send(sequence,value)

if __name__ == '__main__':
    comm = CommRPIO(verbose=True)

    print("comm_piface: starting", file=sys.stderr)
    comm.run()
