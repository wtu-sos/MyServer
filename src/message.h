//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <iostream>

#include "proto/a.pb.h"

class message
{
public:
  enum { header_length = 8 };
  enum { max_body_length = 2048 };

  message() { }

  const char* data() const
  {
    return data_;
  }

  char* data()
  {
    return data_;
  }

  std::size_t length() const
  {
    return header_length + mHeader.len();
  }

  const char* body() const
  {
    return data_ + header_length;
  }

  char* body()
  {
    return data_ + header_length;
  }

  const unsigned int id() const
  {
    return mHeader.id();
  }

  void set_id(unsigned int id) 
  {
    return mHeader.set_id(id);
  }

  std::size_t body_length() const
  {
    return mHeader.len();
  }

  void body_length(std::size_t new_length)
  {
    mHeader.set_len(new_length);
    if (mHeader.len() > max_body_length)
      mHeader.set_len(max_body_length);
  }

  bool decode_header()
  {
    char id[5] = "";
    std::strncat(id, data_, 4);
    mHeader.set_id(ntohl(*(unsigned int*)id));

    std::cout << "id: " << mHeader.id() << std::endl;
    std::cout << "len: " << mHeader.len() << std::endl;

    char len[5] = "";
    std::strncat(len, data_+4, 4);
    mHeader.set_len(ntohl(*(unsigned int*)len));
    if (mHeader.len() > max_body_length)
    {
      mHeader.set_len(0);
      return false;
    }

    return true;
  }

  void encode_header()
  {
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", htonl(mHeader.id()));
    std::sprintf(header+4, "%4d", htonl(mHeader.len()));
    std::cout << "header: " << header << std::endl;
    std::cout << "id: " << mHeader.id() << std::endl;
    std::cout << "len: " << mHeader.len() << std::endl;
    std::memcpy(data_, header, header_length);
  }

private:
  char data_[header_length + max_body_length];
  MsgHeader mHeader;
};

#endif // CHAT_MESSAGE_HPP
