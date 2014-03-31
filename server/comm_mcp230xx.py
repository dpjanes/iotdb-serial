#
#   comm_mcp230xx.py
#
#   David Janes
#   IOTDB.org
#   2014-03-31
#
#   Control a MCP230xx chip over I2C 
#   from stdin / stdout
#

from __future__ import print_function

import time
import sys

from optparse import OptionParser

from libs.Adafruit_MCP230xx import Adafruit_MCP230XX

import comm

class CommMCP230xx(comm.Comm):
    def __init__(self, address=0x20, num_gpios=8, *av, **ad):
        comm.Comm.__init__(self, *av, **ad)

        self.mcp = Adafruit_MCP230XX(address=address, num_gpios=num_gpios)
        self.send("!hello","CommMCP")
        self.pind = {}

    def _configure_pin(self, pin, is_write):
        if self.pind.get(pin) != None:
            return

        self.pind[pin] = is_write

        if is_write:
            self.mcp.config(pin, self.mcp.OUTPUT)
        else:
            self.mcp.config(pin, self.mcp.INPUT)

    def receive(self, sequence, command, *av):
        if command == "DigitalWrite":
            pin, value = av[0], av[1]
            self._configure_pin(pin, True)
            self.mcp.output(pin, value and 1 or 0)
        elif command == "DigitalRead":
            pin, value = av[0], av[1]
            self._configure_pin(pin, False)
            return self.mcp.input(pin) and 1 or 0
        elif command == "DigitalListen":
            pin = av[0]
            self.listensd.setdefault(pin, set()).add(sequence)

            return ""
        else:
            raise KeyError("unknown command: %s" % command)
            
        print ("comm_mcp230xx.receive: seq=%s comm=%s" % ( sequence, command ), file=sys.stderr)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option(
        "", "--debug",
        default = False,
        action = "store_true",
        dest = "debug",
        help = "",
    )
    parser.add_option(
        "", "--mcp23008",
        default = False,
        action = "store_true",
        dest = "mcp23008",
        help = "",
    )
    parser.add_option(
        "", "--mcp23017",
        default = False,
        action = "store_true",
        dest = "mcp23017",
        help = "",
    )
    parser.add_option(
        "", "--address",
        default = "0x20",
        dest = "address",
        help = "I2C address - prefix hex with '0x' (default 0x20)",
    )
    (options, args) = parser.parse_args()

    ## 0x20 is the default for MCP23008 with A0/A1/A2 grounded
    address = 0x20
    if options.address:
        if options.address.startswith("0x"):
            address = int(options.address, 16)
        else:
            address = int(options.address)

    ## depends on the board
    num_gpios = 8
    if options.mcp23008:
        num_gpios = 8
    elif options.mcp23017:
        num_gpios = 16

    ## run
    print("comm_mcp230xx: starting", file=sys.stderr)

    c = CommMCP230xx(address=address, num_gpios=num_gpios)
    c.run()
