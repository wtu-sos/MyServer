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

#include "asio.hpp"

#include "proto/a.pb.h"

class message
{
public:
  enum { header_length = 4 };
  enum { max_body_length = 2048 };

  message() { 
  }

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
    return header_len + sizeof(int);
  }

  const char* body() const
  {
    return data_ + sizeof(int);
  }

  char* body()
  {
    return data_ + sizeof(int);
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
    return header_len;
  }

  void body_length(std::size_t new_length)
  {
    header_len = new_length;
    if (header_len > max_body_length)
      header_len = max_body_length;
  }

  void set_payload(std::string payload) {
    mHeader.set_content(payload);
  }
  bool decode_len() {
    char len[5] = "";
    std::strncat(len, data_, sizeof(int));
    std::cout << "header len: " << *((int*)data_) << std::endl;
    header_len = *((int*)data_);
    return true;
  }

  bool decode_header()
  {
    bool result = mHeader.ParseFromArray(data_ + sizeof(int), header_len);

    std::cout << " header id: " << mHeader.id() 
              << " content: " << mHeader.content() << std::endl;

    return result;
  }

  void encode_header()
  {
    std::string data;
    std::cout << " header id: " << mHeader.id() 
              << " content len: " << header_len
              << " content: " << mHeader.content() << std::endl;
    mHeader.SerializeToString(&data);
    std::cout << "data size: " << data.size() << " data content: " << data << std::endl;
    //data_ += m_payload;
    header_len = data.size();
    std::memset(data_, 0, 2048);
    std::memcpy(data_, &header_len, sizeof(int));
    std::memcpy(data_ + sizeof(int), data.c_str(), data.size());
    //std::strcpy(data_ + sizeof(int), data.c_str());
    //std::strncat(data_ + data.size(), m_payload.data(), m_payload.size());
    //printf("data content: %s \n", data_);
  }

private:
  char data_[2048];
  asio::mutable_buffer raw_data;
  int header_len;
  MsgHeader mHeader;
};

#endif // CHAT_MESSAGE_HPP
