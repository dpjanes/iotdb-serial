#
#   comm_piface.py
#
#   David Janes
#   IOTDB.org
#   2014-03-29
#
#   Control a PiFace board using stdin / stdout
#

from __future__ import print_function

import select
import sys
import os

import pifacedigitalio

import comm

class CommPiFace(comm.Comm):
    def __init__(self, *av, **ad):
        comm.Comm.__init__(self, *av, **ad)
        self.board = 0
        self.listensd = {}

        self.send("!hello","CommPiFace")

        pifacedigitalio.init()

        self.pfd = pifacedigitalio.PiFaceDigital()
        self.listener = None

    def receive(self, sequence, command, *av):
        if command == "DigitalWrite":
            pin, value = av[0], av[1]
            pifacedigitalio.digital_write(pin, value and 1 or 0, self.board)
        elif command == "DigitalRead":
            pin = av[0]
            return pifacedigitalio.digital_read(pin, self.board)
        elif command == "DigitalListen":
            pin = av[0]
            self.listensd.setdefault(pin, set()).add(sequence)

            if self.listener:
                self.listener.deactivate()

            self.listener = pifacedigitalio.InputEventListener(chip=self.pfd)

            for pin in self.listensd.keys():
                self.listener.register(pin, pifacedigitalio.IODIR_ON, self._event)
                self.listener.register(pin, pifacedigitalio.IODIR_OFF, self._event)

            self.listener.activate()
        else:
            raise KeyError("unknown command: %s" % command)
            
        print ("comm_piface.receive: seq=%s comm=%s" % ( sequence, command ), file=sys.stderr)

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

if __name__ == '__main__':
    comm = CommPiFace(verbose=True)

    """
    def switch_pressed(event):
        comm.send(event.pin_num, 1)

    def switch_unpressed(event):
        comm.send(event.pin_num, 0)

    for i in range(4):
        listener.register(i, pifacedigitalio.IODIR_ON, switch_pressed)
        listener.register(i, pifacedigitalio.IODIR_OFF, switch_unpressed)
    listener.activate()
    """

    print("comm_piface: starting", file=sys.stderr)
    comm.run()
