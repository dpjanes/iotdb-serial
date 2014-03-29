iotdb-serial
============

Serial line oriented protocol for controlling GPIO on Raspberry Pi, Arduino, etc.

# Directories
## Client

This is the code you're probably most interested in. When you're using the 
clients it looks like you're directly interacting with GPIO. In fact, you're going
through a line-oriented serial protocol that 

## Server

The fun code that does all the work. The client code knows what to run. You can
run these up by hand for fun and type commands 

## Samples

Sample Node code

# Protocol

The inspiration for this is based on this code, which we hope to be compatible with
https://sites.google.com/site/robertovalgolio/sistemi-programmi/arduino-serial-protocol

Requests and responses are one per line, and data bits are separated by commas. There
is no backslashing or anything allowed or needed.

The requests don't have to be in sync with responses, and you may get many responses
back for a single request. Coordination between requests and responses is done through 
the <sync> parameter, which can be mostly anything though we suggest monotonically 
increasing numbers is the best way to go about this.

We're mosly set up to deal with Integer data but this may be extended to 

## Requests

Requests are in the format

    <seq>,<command>,<pin>[,<value>]

For example, set Pin 10 to "on":

    909,DigitalWrite,10,1

Read Pin 8

    910,DigitalRead,8

Tell me all changes to Pin 9

    911,DigitalListen,9

## Response

Responses are in the format

    <seq>,<result>

If the result is "-", no further results are expected for that sequence number

For the Set 10 on example above:

    909,-

For the read Pin 8 example above (it's off). 

    910,0

For the notification on Pin 9 example above. The first response
is just acking that the command was received. The following respones
are based on the value changes over time. These do not need to come
in all together and may be intersperced with other results and requests

    911,
    911,1
    911,0
    911,1
    911,0
    911,1

