# Quick Play Protocol

Quick Play is an easy way to connect several toys in an ad-hoc fashion on a local network and have them interact in a meaningful way with each other. It doesn't need a central infrastructure and relies on the exchange of simple messages that devices can emit or react on. It is especially designed to combine a sensing orientated toy (e.g. SmartAss Plug or Deep Throat Trainer) with one or several acting toys like an OSSM, vibrators or shock collars.

The emphasis is on simplicity and ad-hoc connectivity to have two or more toys interact with each other. Creating large elaborated sessions with several toys connected to each other, a complex logic and possibly synced to video or VR is out of scope. For these complex scenarios solutions like node-red or x-toys are well established and should be used.

Each toy will remain it's regular connectivity API and can be controlled as normal.

## Topology

Quick Play uses MQTT as its transport layer. To start a Quick Play session one device must be appointed to become the MQTT broker. It will then broadcast its Quick Play service over mDNS, so that all other toys within the network can see the broker and connect to it. All other devices can then connect to this broker and emit or consume the messages. All messages are published to one default topic `quickplay/`. Several separated Quick Play networks may coexist in the same LAN, as every toy must decide with which MQTT broker it connects.

## Message Format

The message format is simple and just denotes the origin, a tag describing the intend of this message and an intensity value. A consumer of the message can use the tag to filter out only messages that are of interest for it. A vibrator or OSSM may choose to listen to the tag `stimulate`, while a electro stimulation device may listen to `punish`. How the intensity is mapped internally on the various toy properties is completely up to the toys' firmware. If a message is received it will executed for roughly 1.5 seconds. After that, the value will automatically drop to 0.0, if no new updated massage with the same tag has been received. The value 0.0 should not be emitted, as the absence of messages is automatically interpreted as 0.0. In turn a new message should be sent roughly every second to signal the receiver that this is still relevant. The message rate should be limited to no more than 5x per seconds to reduce bandwidth needs. It is best practice to only message on changes and then repeat every second as keep-alive. It is not intended to use these messages as a form of pattern generator for the devices. Rather as a way of signalling them how they should behave in general.

Devices may emit messages with several different tags. Devices consuming tags typically react only to one specific tag.

| Parameter | Type   | Info                                                                                                                                                                                             |
| --------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| origin    | string | The device emitting the message                                                                                                                                                                  |
| tag       | string | A tag describes a certain intent how this message should be interpreted. Default tags are `reward`, `stimulate`, `signal`, `motivate`, `punish`. It is possible to expand this with custom tags. |
| intensity | number | The intensity in percent from [0.0 - 100.0]. Values outside of this range will be cropped.                                                                                                       |

### Example JSON

```JSON
{
    "origin": "SmartAssPlug-c4dee2a795e8",
    "tag": "stimulate",
    "intensity": 50.0,
}
```

### Conflicting Messages

Two devices emitting messages with the same tag may lead to conflicting instructions for a toy. In such a case the source with the highest intensity is given priority.

## Configuring a Quick Play Session

1. Make one of the devices you want to connect to a Quick Play Facilitator
2. Connect the remaining devices to the MQTT broker of the Quick Play Facilitator
3. Configure the devices in question and setup their properties how they emit or react to messages
4. Play along and have fun!
