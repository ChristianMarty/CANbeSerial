# CANbeSerial

A protocol to transmit CAN and CAN-FD frames via a serial link (e.g. RS-232, TCP, UDP).

## Message Framing
The CANbeSerial uses the COBS (Consistent Overhead Byte Stuffing) framing algorithm for message framing.

### Example Frame

 <table style="width:100%;">
  <tr>
    <th>Byte</th>
    <td>0</td>
    <td>1</td>
    <td>2</td>
    <td>n</td>
    <td>n+3</td>
    <td>n+4</td>
    <td>n+5</td>
  </tr>
  <tr>
    <th>Data</th>
    <td>NULL (0x00)</td>
    <td>COBS Overhaed</td>
    <td>Payload Id</td>
    <td>Payload</td>
    <td>CRC H</td>
    <td>CRC L</td>
    <td>NULL (0x00)</td>
  </tr>
</table>

### Maximum Frame Lenght

The payload of a frame shall never exceed 74 bytes. In combination with the leading and trailing NULL, the COBS byte, Payload Id and CRC the maximum total frame length is 80 Bytes.

### Implementation Guideline
Each message shall start with a NULL byte and end with a NULL byte. 

The receiving end shall buffer incoming data and process it when NULL is received. 

The receiving stack should be reinitialized after every NULL.

Timing-based framing, frame timeouts or other time-based actions are strongly discouraged.

## CRC 
Each CANbeSerial frame shall have a 16-bit CRC (Cyclic Redundancy Check) at the end of the package.

The CRC shall be calculated over the Payload Id and the Payload. The 0/NULL bytes and payload are not part of the CRC. 

## Payload Id

The Payload Id is an 8-bit number that specifies the type of data contained in the payload.

 <table style="width:100%;">
  <tr>
    <th>Id</th>
    <th>Name</th>
    <th>Description</th>
  </tr>
  <tr>
    <td>0x00</td>
    <td>Data</td>
    <td>Contains the CAN ID and CAN Data</td>
  </tr>

  <tr>
    <td></td>
    <td></td>
    <td></td>
  </tr>

  <tr>
    <td>0x01</td>
    <td>Protocol Version</td>
    <td>The CANbeSerial protocol version</td>
  </tr>
  <tr>
    <td>0x81</td>
    <td>Protocol Version Request</td>
    <td>Request the CANbeSerial protocol version</td>
  </tr>

  <tr>
    <td>0x02</td>
    <td>BUS Configuration State</td>
    <td>Contains the current BUS configuration of the device</td>
  </tr>
  <tr>
    <td>0x82</td>
    <td>BUS Configuration Request</td>
    <td>Request the current BUS configuration of the device</td>
  </tr>
  <tr>
    <td>0xC2</td>
    <td>BUS Configuration Command</td>
    <td>Sets BUS configuration for the device</td>
  </tr>

  <tr>
    <td>0x03</td>
    <td>Device Information</td>
    <td>Contains a string that describes the vendor and version of the device</td>
  </tr>
  <tr>
    <td>0x83</td>
    <td>Device Information Request</td>
    <td>Request the device information</td>
  </tr>

</table>

## Payload Id 0x00 - Data
A data frame shall be constructed as follows:

 <table style="width:100%;">
  <tr>
    <th>Byte</th>
    <td>0</td>
    <td>1</td>
    <td>2</td>
    <td>3</td>
    <td>4</td>
    <td>5</td>
    <td>6</td>
    <td>7</td>
    <td>8</td>
    <td>9</td>
    <td>n</td>
  </tr>
  <tr>
    <th>Data</th>
    <td>Timestap (MSB)</td>
    <td>Timestap </td>
    <td>Timestap </td>
    <td>Timestap (LSB)</td>
    <td>CAN Identifier (MSB)</td>
    <td>CAN Identifier </td>
    <td>CAN Identifier </td>
    <td>CAN Identifier (LSB)</td>
    <td>CAN Control Field</td>
    <td>Reserved</td>
    <td>Data</td>
  </tr>
</table>

## TODO
At the moment a mechanism for the configuration of ID filters is not defined.

