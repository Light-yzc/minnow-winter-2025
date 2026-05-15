#include "router.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // debug( "unimplemented add_route() called" );
  router_table_chunk route_chunk {};
  route_chunk.route_prefix = route_prefix;
  route_chunk.prefix_length = prefix_length;
  route_chunk.next_hop = next_hop;
  route_chunk.interface_num = interface_num;
  router_table.push_back(route_chunk);
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // debug( "unimplemented route() called" );
  for(auto inter : interfaces_) {
    auto& datagrams_rec = inter->datagrams_received();
      while (!datagrams_rec.empty())
      {
        /* code */
        InternetDatagram datagram = datagrams_rec.front();
        datagrams_rec.pop();
        auto ip_addr = datagram.header.dst;
        if ( datagram.header.ttl <= 1 ) {
          continue;
        }
        datagram.header.ttl--;
        datagram.header.compute_checksum();
        uint32_t mask;
        router_table_chunk best {};
        bool have_best = false;
        if(datagram.header.ttl > 0) {
          for (auto router_rules : router_table) {
            if (router_rules.prefix_length > 0) {
              mask = (UINT32_MAX << ( 32 - router_rules.prefix_length));
            }
            else {
              mask = 0;
            }
            if ((router_rules.route_prefix & mask) == (ip_addr & mask)) {
                if (have_best == false) {
                  best = router_rules;
                  have_best = true;
                } else if (best.prefix_length < router_rules.prefix_length) {
                  best = router_rules;
                }
                // interfaces_[router_rules.interface_num]->send_datagram(datagram, next_hop);

            }
          }
        }
        if (have_best) {
          Address next_hop = best.next_hop.has_value() ? best.next_hop.value() : Address::from_ipv4_numeric(ip_addr);

          interfaces_[best.interface_num]->send_datagram(datagram, next_hop);
        }
      }
  }
}
