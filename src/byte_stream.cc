#include "byte_stream.hh"
#include "debug.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// Push data to stream, but only as much as available capacity allows.
void Writer::push( string data )
{
  // Your code here (and in each method below)
  // debug( "Writer::push({}) not yet implemented", data );
  if ( closed_ ) {
    return;
  }
  const auto push_len = std::min(data.size(), available_capacity());
  bytes_pushed_ += push_len;
  buffer_ += data.substr(0, push_len);
}

// Signal that the stream has reached its ending. Nothing more will be written.
void Writer::close()
{
  // debug( "Writer::close() not yet implemented" );
  closed_ = true;
}

// Has the stream been closed?
bool Writer::is_closed() const
{
  // debug( "Writer::is_closed() not yet implemented" );
  return closed_; // Your code here.
}

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const
{
  // debug( "Writer::available_capacity() not yet implemented" );
  const auto remain_len = capacity_ - buffer_.size();
  return remain_len; // Your code here.
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const
{
  // debug( "Writer::bytes_pushed() not yet implemented" );
  return bytes_pushed_; // Your code here.
}

// Peek at the next bytes in the buffer -- ideally as many as possible.
// It's not required to return a string_view of the *whole* buffer, but
// if the peeked string_view is only one byte at a time, it will probably force
// the caller to do a lot of extra work.
string_view Reader::peek() const
{
  // debug( "Reader::peek() not yet implemented" );
  return buffer_; // Your code here.
}

// Remove `len` bytes from the buffer.
void Reader::pop( uint64_t len )
{
  buffer_ = buffer_.substr(len);
  bytes_popped_ += len;
  // debug( "Reader::pop({}) not yet implemented", len );
}

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const
{
  // debug( "Reader::is_finished() not yet implemented" );
  return closed_ && buffer_.empty(); // Your code here.
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const
{
  // debug( "Reader::bytes_buffered() not yet implemented" );
  return buffer_.size(); // Your code here.
}

// Total number of bytes cumulatively popped from stream
uint64_t Reader::bytes_popped() const
{
  return bytes_popped_; // Your code here.
}
