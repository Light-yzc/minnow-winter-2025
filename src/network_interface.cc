#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  EthernetHeader eth_header;
  eth_header.src = ethernet_address_;
  eth_header.type = EthernetHeader::TYPE_IPv4;
  const auto u32_ip = next_hop.ipv4_numeric();
  EthernetFrame eth_frame;

  auto it = arp_table_.find(u32_ip);
  if (it != arp_table_.end()) {
    EthernetAddress arp_eth_frame = it->second.ethernet_address;
    eth_header.dst = arp_eth_frame;
    eth_frame.payload = serialize(dgram);

  }
  else {
      eth_header.type = EthernetHeader::TYPE_ARP;
      auto it_coold = arp_cool_down.find(u32_ip);
      datagrams_to_send_[u32_ip].push(dgram);
      if (it_coold != arp_cool_down.end()) {
        return;
      } 
      eth_header.dst = ETHERNET_BROADCAST;
      ARPMessage arp_msg;
      arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
      arp_msg.target_ip_address = u32_ip;
      arp_msg.sender_ethernet_address = ethernet_address_;
      arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
      eth_frame.payload = serialize(arp_msg);
      arp_cool_down[u32_ip] = 5000;

  }
  eth_frame.header = eth_header;
  transmit(eth_frame);
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  if (frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST) {
    return;
  }
  if (frame.header.type == EthernetHeader::TYPE_IPv4) {
      InternetDatagram dgram;
      if (parse(dgram, frame.payload)) {
       datagrams_received_.push(dgram);
    }
  }
  else if (frame.header.type == EthernetHeader::TYPE_ARP) {
      ARPMessage arp_msg;
      auto res = parse(arp_msg, frame.payload);
      if (res && arp_msg.opcode == ARPMessage::OPCODE_REQUEST && arp_msg.target_ip_address == ip_address_.ipv4_numeric()) {
        ARPMessage arp_reply;
        arp_reply.sender_ethernet_address = ethernet_address_;
        arp_reply.sender_ip_address = ip_address_.ipv4_numeric();
        arp_reply.target_ethernet_address = arp_msg.sender_ethernet_address;
        arp_reply.target_ip_address = arp_msg.sender_ip_address;
        arp_reply.opcode = ARPMessage::OPCODE_REPLY;
        EthernetFrame eth_frame;
        EthernetHeader eth_header;
        eth_header.src = ethernet_address_;
        eth_header.dst = arp_reply.target_ethernet_address;
        eth_header.type = EthernetHeader::TYPE_ARP;
        eth_frame.header = eth_header;
        eth_frame.payload = serialize(arp_reply);
        transmit(eth_frame);
        // return;

      }
      if (res) {
        arp_ arp_to_push;
        arp_to_push.ethernet_address = arp_msg.sender_ethernet_address;
        arp_to_push.time_tick = 30000;
        arp_table_[arp_msg.sender_ip_address] = arp_to_push;
        auto it = datagrams_to_send_.find(arp_msg.sender_ip_address);
        if (it != datagrams_to_send_.end()) {
          while (!it->second.empty())
          {
            send_datagram(it->second.front(), Address::from_ipv4_numeric(it->first));
            it->second.pop();
          }
          datagrams_to_send_.erase(it);
        }
    }
  }

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  const int32_t int32_tick = static_cast<int32_t> (ms_since_last_tick);
  for(auto it = arp_cool_down.begin(); it != arp_cool_down.end();) {
    if (it->second - int32_tick > 0) {
        it->second = it->second - int32_tick;
        it++;

    }
    else {
      auto it_pending = datagrams_to_send_.find(it->first);
      if (it_pending!=datagrams_to_send_.end()) {
        datagrams_to_send_.erase(it_pending);
      }
      it = arp_cool_down.erase(it);
    }
  }

  for (auto it = arp_table_.begin(); it != arp_table_.end();) {
    if (it->second.time_tick - int32_tick > 0) {
      it->second.time_tick = it->second.time_tick - int32_tick;
      it++;

    }
    else {
      it = arp_table_.erase(it);
    }
  }
}
