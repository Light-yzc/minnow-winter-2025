#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // debug( "unimplemented sequence_numbers_in_flight() called" );

  return next_seq_no_ - abs_ackno_;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  debug( "unimplemented consecutive_retransmissions() called" );
  return times_retransport;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // debug( "unimplemented push() called" );


  while ( true ) {
    TCPSenderMessage payload;
    payload.seqno = Wrap32::wrap(next_seq_no_, isn_);

    payload.RST = input_.has_error();
    if (!SYN_send_) {
      payload.SYN = true;
      SYN_send_ = true;
    }
    uint64_t effect_window_size = (window_size_ == 0 ? 1 :window_size_);
    if(effect_window_size <= sequence_numbers_in_flight()) {return;} 
    auto available_size = effect_window_size - sequence_numbers_in_flight();
    auto tcp_size = std::min(available_size, TCPConfig::MAX_PAYLOAD_SIZE);
    std::string payload_data;
    tcp_size = payload.SYN ? tcp_size - 1 : tcp_size;
    read(reader(), tcp_size, payload_data);
    // if (payload.FIN) {}
    payload.payload = payload_data;
    if ( reader().is_finished() && !FIN_sed_
      && payload.sequence_length() + 1 <= available_size ) {
      payload.FIN = true;
      FIN_sed_ = true;
      }
    if(payload.sequence_length() == 0) {
      return;
    }
    transmit(payload);
    out_seg_que_.push_back({payload});
    next_seq_no_ += payload.sequence_length();
    }
  // (void)transmit;
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // debug( "unimplemented make_empty_message() called" );
  TCPSenderMessage payload;
  payload.seqno = Wrap32::wrap(next_seq_no_, isn_);
  payload.RST = input_.has_error();
  return payload;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if (msg.RST) {
    input_.set_error();
    return;
  }
  // debug( "unimplemented receive() called" );
  window_size_ = msg.window_size;
  if (msg.ackno.has_value()) {
    const auto tmp_ackno = msg.ackno->unwrap(isn_, next_seq_no_);
    if (tmp_ackno <= next_seq_no_ && tmp_ackno > abs_ackno_) {abs_ackno_ = tmp_ackno;}
    else {return;}
    while (!out_seg_que_.empty())
    {
      const auto front_buffer_msg = out_seg_que_.front().message;
      const uint64_t abs_buff_seq_no = front_buffer_msg.seqno.unwrap(isn_, next_seq_no_) + front_buffer_msg.sequence_length();
      if (abs_buff_seq_no <= abs_ackno_) {
        out_seg_que_.pop_front();
        continue;
      }
      break;
    }
    
    times_retransport = 0;
    cur_RTO = 0;
    acc_tick = 0;
  }

  
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // debug( "unimplemented tick({}, ...) called", ms_since_last_tick );
  // (void)transmit;
  if (cur_RTO == 0) {cur_RTO = initial_RTO_ms_;}
  acc_tick += ms_since_last_tick;
  if(acc_tick >= cur_RTO && !out_seg_que_.empty()) {
    auto paylaod = out_seg_que_.front().message;
    transmit(paylaod);
    if (window_size_ != 0){
      cur_RTO *= 2;
      times_retransport++;
    }
    acc_tick = 0;
  }
  if ( out_seg_que_.empty() ) {
    acc_tick = 0;
    cur_RTO = initial_RTO_ms_;
  }


}
