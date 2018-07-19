# unicast2multicast
unicast2multicast - Bridge unicast IPv4 UDP packets and forward as multicast

The original purpose of this is for a DVB streaming server (such as dvblast) on a LAN to unicast UDP packets to one or more bridges, then for this program to run on those bridges and recieve UDP packets on one interface and forward on UDP packets within the bridged LAN on a different interface using multicast.

The program is designed to run as a service.

When multicasting several DVB channels I suggest you send DVB channels to the bridge IP using different ports, then use port 5004 for the multicast group to allow RTP to work correctly.

You may also want to run a sapserver on the bridge to advertise the multicast DVB channels to the clients within the LAN.

Note that super user priviledges may be required to run this.

## Syntax :

`unicast2multicast -i incoming_ip:port -m multicast_group[:port] [-o outgoing_interface_IP]

## Where :

 * `-i` IP:port of local interface where unicast UDP packets are being sent
 * `-m` multicast group to use when forwarding on packets, optionally a different port number
 * `-o` optionally specify the IP address of an interface to use for multicast packets, the default is used if ommited

## Return codes :

 * `0` - Success
 * `1` - Error
