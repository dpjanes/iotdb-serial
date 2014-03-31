/*
 *  comm.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 *
 *  Manage GPIO-type device communications over the stdin / stdout.
 *
 *  Happy Arduino Day 2014
 */

"use strict"

var fs = require('fs')
var spawn = require('child_process').spawn
var events = require('events');
var util = require('util');

var carrier = require('carrier')
var SerialPort = require("serialport").SerialPort

var Comm = function() {
}
util.inherits(Comm, events.EventEmitter);

Comm.prototype.setup = function(initd) {
    var self = this;

    events.EventEmitter.call(self);

    self._has_open = false;
    self._has_hello = false;
    self._is_ready = false;

    self.sequence = 1
    self.callbackd = {}

    self.sout = null;   // note direction! output of server
    self.serr = null;   // stderr of server
    self.sin = null;    // input of server

    if (initd.program) {
        self.program = spawn(initd.program, initd.av)

        self.sout = self.program.stdout
        self.serr = self.program.stderr
        self.sin = self.program.stdout

        self._has_open = true;
    } else if (initd.tty) {
        var spio = new SerialPort(initd.tty, {
            baudrate: initd.baudrate ? initd.baudrate : 9600
        });
        if (!spio) {
            console.log("# Comm.setup: could not open tty", initd.tty)
            throw "IO Error opening TTY"
        }

        spio.on("open", function() {
            self._has_open = true;
            self._check_ready()
        })
        
        self.sout = spio
        self.sin = spio
    } else if (initd.sout && initd.sin) {
        self.sout = initd.sout
        self.sin = initd.sin

        self._has_open = true;
    }

    // console.log("HERE:A", initd)

    carrier.carry(self.sout, function(line) {
        console.log('- stdout: ' + line);
        var parts = line.split(",")
        if (parts.length < 2) {
            return
        }
        
        if (parts[0] == "!hello")  {
            self._has_hello = true;
            self._check_ready()
        }

        var paramd = self.callbackd[parts[0]]
        if (!paramd) {
            return
        }

        var result = parts[1]
        if (result == "-") {
            delete self.callbackd[parts[0]]
        } else if (result == "") {
        } else if (result.substring(0, 1) == "!") {
        } else if (result.substring(0, 1) == "#") {
        } else {
            paramd.callback(paramd.pin, parseInt(parts[1]))
        }
    });

    if (self.serr) {
        carrier.carry(self.serr, function(line) {
            console.log('- stderr: ' + line);
        });
    }
}

Comm.prototype._check_ready = function() {
    var self = this;

    if (self._is_ready) {
        return
    }
    if (!self._has_open) {
        return
    }
    if (!self._has_hello) {
        return
    }

    self._is_ready = true;
    self.emit("ready")
}

Comm.prototype.analog_write = function(pin, value) {
    this.send({
        command: "AnalogWrite",
        pin: pin,
        value: value
    })
}

Comm.prototype.analog_listen = function(pin, callback) {
    this.send({
        command: "AnalogListen",
        pin: pin,
        callback: callback
    })
}

Comm.prototype.digital_write = function(pin, value) {
    this.send({
        command: "DigitalWrite",
        pin: pin,
        value: value
    })
}

Comm.prototype.digital_read = function(pin, callback) {
    this.send({
        command: "DigitalRead",
        pin: pin,
        callback: callback
    })
}

Comm.prototype.digital_listen = function(pin, callback) {
    this.send({
        command: "DigitalListen",
        pin: pin,
        callback: callback
    })
}

Comm.prototype.send = function(paramd) {
    var self = this;

    if (self._is_ready) {
        self._send(paramd)
    } else {
        self.on("ready", function() {
            self._send(paramd)
        })
    }
}


Comm.prototype._send = function(paramd) {
    var self = this;

    var s = "" + ( self.sequence++ );
    if (paramd.callback) {
        self.callbackd[s] = paramd
    }

    var line = s;
    if (paramd.command !== undefined) {
        line += ","
        line += paramd.command;
    }
    if (paramd.pin !== undefined) {
        line += ","
        line += paramd.pin;
    }
    if (paramd.value !== undefined) {
        line += ","
        line += paramd.value;
    }
    line += "\n"

    console.log("- send", "seq=" + s)
    self.sin.write(line)
}

exports.Comm = Comm
