/*
 *  comm_piface.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 */

"use strict"

var comm = require("./comm")

var CommPiFace = function() {
    this.setup({
        program: "python3",
        av: [ "../server/comm_piface.py", ]
    })
}

CommPiFace.prototype = new comm.Comm;

exports.CommPiFace = CommPiFace
