# Uplink Message Format

We assume familiarity with design considerations for the adaptive ClAir transmission scheme, which we explaine in a separate document. In the following, Details of the message structure are specified. This strucutre is the same for all MCS.

## Design Principles

- Each message consists of a time series of _samples_. The time series must be equispaced in time, so that no explicit time stamp per sample is required.
- Implicit time stamps: Because clock synchronization is impractical, we take the reception time at the ClAir Server as time stamp of the last sample in a message and backwards determine the time stamps for all other samples in the same message. In consequence, the overall time series will not be evenly spaced. The small deviations do not matter for our purposes, though.
- Multi-measurement samples: A sample may contain several measurement values for different quantities, like CO&#x2082;, temperature, or humidity. All these measurements must have been taken at the same time for implicit time stamps to work.
- Decouple data format and transmission mode: To separate concerns and keep implementation simple, both on the Node and for all backend data processing, we use the same data representation for each MCS. In particular, we do not alter the resolution (bit/measurement) across MCSs.
- Allow for additional messages in the future: Even though we design a single message to report CO&#x2082; concentration here and now, there might arise the need to measure other quantities, or to transmit status information. Therefore, we need a way for the ClAir Server to differentiate between different types of received messages via an explicit _message ID_.
- Enable different versions of a given message: Once the system is in operation, we hope to learn how it is used and to feed back our learnings into updates. We need to include an explicit _version number_ with each message, so that future Nodes can use an improved message format.
- Allow for custom node models with custom message format: We envision sensor nodes from multiple manufacturers, in addition to the ClAirchen prototype we design at the moment. Each Node type might use different sensor elements and sensing principles, different data processing, and come with different guarantees on sensor resolution and accuracy. We do not and cannot foresee all those details; instead, each manufacturer can specify its own data format for a given sample, and provide encoder and decoder for it. We foresee two ways to route messages of different node types to different decoders: Either, all messages of a given node model arrive at a dedicated server endoint, or the ClAir server can differentiate node types on the basis of the node's hardware ID. We do not reserve space for a node model field in the message header.

## Message Structure

Each message has the same structure:

- Header: 1 byte of fixed format that encodes the message type, message version and sampling rate (see below).
- A sequence of equispaced samples. Each sample must consist of the same fixed number of bytes, where the encoding is up to the manufacturer of the Node. All samples of a message must have the same format and be of the same length. For example, the Clairchen samples are all two bytes, with format explained below.
- Sample timing is implicit. Samples transmitted within one message are assumed to be equispaced in time, with the leftmost sample at index 0 being the oldest, and the rightmost sample being the newest, when the message is written left to right (see example below). This allows for arrays or lists where new samples are appended at the end. The sampling rate is encoded in the header.

Message example:

| header (1 byte) | sample1 (N bytes) | sample2 (N bytes) | sample3 (N bytes |

Here, sample3 is has been taken last.

## Header format

The message header consists of a single byte. We use it to encode the message type, message version, and sampling rate as follows, with bit0 being the least significant bit (LSB):

- bit0 - bit1: Version number - 4 versions of the protocol.
- bit2 - bit4: Message type - 8 different messages overall. Version number and message type might be traded off against each other.
- bit5 - bit7: Message-specific header. For the messages considered so far, where the payload consists of a sample series, the message header is a sampling rate (SR) index that allows for 8 different sampling rates. Because the sample size may vary between different Node types, the SR may vary as well. Therefore, the SR index selects the actual SR from a Node-type-specific rate table.

For our prototype ClAirchen Node, there is only one uplink message with a number of measurement samples that can be expressed in three bits. There, the mapping between the message-specific header and the number of samples transmitted is simpl: number of samples = decimal(msg-specific header) + 1. Thus a message-specific header of `b000` implies 1 sample, and a message-specific header of `b111`implies 8 samples in the message payload.
