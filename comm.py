#
#   comm.py
#
#   David Janes
#   IOTDB.org
#   2014-03-29
#
#   Manage GPIO-type device communications
#   over the stdin / stdout.
#   See
#   https://sites.google.com/site/robertovalgolio/sistemi-programmi/arduino-serial-protocol
#
#   The real action is in the subclasses
#
#   Happy Arduino Day 2014
#

from __future__ import print_function

import select
import sys
import os
import traceback

class Comm(object):
    def __init__(self, cin=sys.stdin, cout=sys.stdout, verbose=False):
        self.cin = cin
        self.cout = cout
        self.verbose = verbose

        self.quit = False
        self.polling = False

        self.poll = select.poll()
        self.poll.register(cin, select.POLLIN|select.POLLHUP)

        self.sends = []

    def run(self):
        while not self.quit:
            try:
                ## is there data to send?
                if self.sends:
                    self.poll.register(self.cout, select.POLLOUT|select.POLLHUP)
                else:
                    try:
                        self.poll.unregister(self.cout)
                    except KeyError:
                        pass
                
                ## wait for data - this can be interrupted
                try:
                    self.polling = True
                    ts = self.poll.poll()
                except KeyboardInterrupt:
                    if self.verbose:
                        pass ## print("comm: caught Interrupt! [1]", file=sys.stderr)

                    continue
                finally:
                    self.polling = False

                ## try one write or read operation
                for fileno, mode in ts:
                    if fileno == self.cout.fileno():
                        if self.sends:
                            send = self.sends.pop()
                            self.cout.write(send + "\n")
                    elif fileno == self.cin.fileno():
                        self._process_input(self.cin.readline())
                    else:
                        print("comm: impossible state - quiting", file=sys.stderr)
                        sys.exit(1)

                    break
            except KeyboardInterrupt:
                print("comm: caught Interrupt! [2]", file=sys.stderr)

        print("comm: finished comm loop", file=sys.stderr)

    def send(self, sequence, value, *av):
        """Send the line to the output"""
        line = "%s,%s" % ( sequence, value, )
        self.sends.append(line)

        if self.polling:
            os.kill(os.getpid(), 2)

    def receive(self, sequence, command, *av):
        """Redefine me in subclasses"""

    def _process_input(self, line):
        if self.verbose:
            print("comm: LINE: " + repr(line), file=sys.stderr)

        if line == "":
            self.quit = True
            self.stop()
            return

        parts = line.split(",")
        if len(parts) < 2:
            print("comm: bad line: " + repr(line), file=sys.stderr)
            return

        try:
            result = self.receive(parts[0], parts[1], *map(lambda p: int(p), parts[2:]))
            if result == None:
                result = "-"

            self.send(parts[0], result)
        except:
            traceback.print_exc()
            self.send(parts[0], "!error")
