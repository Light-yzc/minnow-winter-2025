#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if (message.RST) {reassembler_.set_error();}
  if ( !seqno_is_set_ ) {
    if ( !message.SYN ) {
      return;
    }

    base_ = message.seqno;
    seqno_is_set_ = true;
  }
  // if(message.seqno == seqno_){seqno_ = Wrap32(seqno_ + message.sequence_length());}
  uint64_t checkpoint = reassembler_.writer().bytes_pushed() + 1;
  uint64_t abs_seqno = message.seqno.unwrap( base_, checkpoint );
  uint64_t first_index = message.SYN ? 0 : abs_seqno - 1;

  reassembler_.insert( first_index, message.payload, message.FIN );
}


TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  // debug( "unimplemented send() called" );
  TCPReceiverMessage payload {};
  // if (seqno_is_set_) {payload.ackno = seqno_;}
  if ( seqno_is_set_ ) {
  uint64_t ack_abs = reassembler_.writer().bytes_pushed() + 1;

  if ( reassembler_.writer().is_closed() ) {
    ack_abs += 1;
  }

  payload.ackno = Wrap32::wrap( ack_abs, base_ );
}

  uint64_t max_len = 65535;
  if (reassembler_.writer().has_error()) {payload.RST = true;}
  payload.window_size = std::min(reassembler_.writer().available_capacity(), max_len);
  return payload;
}
