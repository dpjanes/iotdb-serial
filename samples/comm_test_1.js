/*
 *  comm_test_1.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 *
 *  This just listens to the CommTest
 *  server, which will cycle through numbers
 *  when you listen to it
 */

"use strict"

var CommTest = require("../clients/comm_test").CommTest

var comm = new CommTest()

comm.digital_listen(1, function(pin, value) {
    console.log("+ result", pin, value)
})
