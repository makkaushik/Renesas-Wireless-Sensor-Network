# Renesas-Wireless-Sensor-Network
Sensor Network Operation
When YRDKRX63N board wants to transmit a data to a destination node, it needs to broadcast a block signal first to drive other nodes in silent mode. The broadcast packet contains the address of the desired destination node with which communication is to be established. The address of the desired destination node is populated into the packet using a routing table. This broadcast of the this node puts the nodes in the vicinity of the transmitter in silent mode. The receiving node then broadcasts a medium block packet to nodes in its vicinity. The transmitting node then sends out the data packet to the receiving node. This data packet is sent specifically to the receiving node and is not broadcast. Once the data is sent, the transmitter and receiver nodes send out a broadcast message to nodes in their vicinity informing them that the medium is free.
COORDINATOR ADDRESS: 0x1001, ROUTER ADDRESS : 0x2001, END NODE - 0x3001
The codes for END NODE and ROUTER NODE are same except for ‘main.c’ file, where the routing table differs.

Algorithms

Algorithm for Broadcasting a Block signal:
The Block broadcast signal contains the address of the destination that the sender wants to communicate with. Thus when a node receives a Block broadcast signal, the packet compares the address field with itself. If the destination address does not match with address field, the node/controller is driven into silent mode. However, if the addresses match, then the receiver sends an Acknowledgment Broadcast to drive other nodes in its vicinity into silent mode.

The destination node broadcasts the sender’s address along with Acknowledgment Block signal (0x00) to reserve the medium.
1. To broadcast a signal, controller generates a TX Request (16 bit address) packet. 2. The parameter is configured as follows:
i. Length (MSB) = 0x00
ii. Length (LSB) = 0x08
iii. Frame ID = 0x00
iv. API Identifier = 0x01
v. Destination address [0:1] = 0xFFFF vi. Options = 0x01
vii. Data [0] = 0x01 (Block signal)
viii. Data [1] = MSB of (16 bit) destination address. (from routing table) ix. Data [2] = LSB of destination address.
3. Evaluate the checksum append the result in the frame. 4. Add the XBee packet to the buffer for transmission.
5. Serially transmit the data from buffer.

Algorithm to Broadcast an Acknowledgement Block signal:
1. To broadcast an ACK signal, controller generates a TX Request (16 bit address) packet. 2. The parameter should be configured as follows:
i. Length (MSB) = 0x00
ii. Length (LSB) = 0x08
iii. Frame ID = 0x00
iv. API Identifier = 0x01
v. Destination address [0:1] = 0xFFFF vi. Options = 0x01
vii. Data [0] = 0x11 (Acknowledgement Block signal)
viii. Data [1] = MSB of (16 bit) sender address. (from initial broadcast signal) ix. Data [2] = LSB of destination address.
3. Evaluate the checksum append the result in the frame. 4. Add the XBee packet to the buffer for transmission.
5. Serially transmit the data.

Algorithm for sending actual data packet :
When the transmitting node receives acknowledgement broadcast from the desired destination node, it assembles the API data packet (TX Request) and transmits it to the destination node.
1. To send a data packet, controller generates a TX Request (16 bit address) packet. 2. The parameter should be configured as follows:
i. Length (MSB) = 0x00
ii. Length (LSB) = depends upon data length
iii. Frame ID = 0x00
iv. API Identifier = 0x01
v. Destination address [0:1] = Destination address (2 Bytes) vi. Options = 0x01
vii. Data [0] = Desired destination address (MSB)
viii. Data [1] = Desired destination address (LSB)
ix. Data [2:n] = Data
3. Evaluate the checksum append the result in the frame.
4. Add the XBee packet to the buffer for transmission.
5. Serially transmit the data from buffer.
When the transmission of data completes, the sender board will initialize an Unblock broadcast signal, to release the blocked nodes in its vicinity.

Algorithm to Broadcast a Un-Block signal:
1. To broadcast a signal, controller generates a TX Request (16 bit address) packet. 2. The parameter is configured as follows:
i. Length (MSB) = 0x00
ii. Length (LSB) = 0x08
iii. Frame ID = 0x00
iv. API Identifier = 0x01
v. Destination address [0:1] = 0xFFFF vi. Options = 0x01
vii. Data [0] = 0x10 (Un-Block signal)
viii. Data [1] = MSB of (16 bit) destination address. (from routing table)
ix. Data [2] = LSB of destination address.
3. Evaluate the checksum append the result in the frame.
4. Add the XBee packet to the buffer for transmission.
5. Serially transmit the data from buffer.

Algorithm to Broadcast an Acknowledgement Un-Block signal:
The destination node, upon receiving Unblock signal, will transmit an Acknowledgement Unblock signal to release the blocked nodes in its vicinity.
1. To broadcast a signal, controller generates a TX Request (16 bit address) packet. 2. The parameter is configured as follows:
i. Length (MSB) = 0x00
ii. Length (LSB) = 0x08
iii. Frame ID = 0x00
iv. API Identifier = 0x01
v. Destination address [0:1] = 0xFFFF vi. Options = 0x01
vii. Data [0] = 0x11 (Acknowledgement Un-Block signal)
viii. Data [1] = MSB of (16 bit) destination address. (from routing table) ix. Data [2] = LSB of destination address.
3. Evaluate the checksum append the result in the frame. 4. Add the XBee packet to the buffer for transmission.
5. Serially transmit the data from buffer.

Whether the packet is Block, Unblock, or acknowledgement is determined from the first three bytes of data section of API packet. The first byte represents block/unblock requests and the second and third bytes represent the MSB and LSB of the address of the desired destination node
