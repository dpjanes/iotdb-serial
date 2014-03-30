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

        self.send("!hello","CommRPIO")


    def receive(self, sequence, command, *av):
        if command == "DigitalWrite":
            pin, value = av[0], av[1]

            RPIO.setup(pin, RPIO.OUT)
            RPIO.output(pin, value and True or False)
        elif command == "DigitalRead":
            pass
            ## pin = av[0]
            ## return pifacedigitalio.digital_read(pin, self.board)
        elif command == "DigitalListen":
            pass
            """
            pin = av[0]
            self.listensd.setdefault(pin, set()).add(sequence)

            if self.listener:
                self.listener.deactivate()

            self.listener = pifacedigitalio.InputEventListener(chip=self.pfd)

            for pin in self.listensd.keys():
                self.listener.register(pin, pifacedigitalio.IODIR_ON, self._event)
                self.listener.register(pin, pifacedigitalio.IODIR_OFF, self._event)

            self.listener.activate()
            return ""
            """
        else:
            raise KeyError("unknown command: %s" % command)
            
        print ("comm_piface.receive: seq=%s comm=%s" % ( sequence, command ), file=sys.stderr)

    """
    def stop(self):
        if self.listener:
            self.listener.deactivate()
            self.listener = None

    def _event(self, event):
        pin = event.pin_num

        listens = self.listensd.get(pin)
        if not listens:
            return

        for sequence in listens:
            value = pifacedigitalio.digital_read(pin, self.board)
            self.send(sequence,value)
    """

if __name__ == '__main__':
    comm = CommRPIO(verbose=True)

    print("comm_piface: starting", file=sys.stderr)
    comm.run()
