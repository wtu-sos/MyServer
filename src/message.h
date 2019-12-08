#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <arpa/inet.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "asio.hpp"
#include "glog/logging.h"

struct MsgHeader {
  unsigned int uiType;
  unsigned int uiMsgLen;
};

class message {
 public:
  enum { header_length = 8 };
  enum { max_body_length = 2048 };

  message() {
    mHeader.uiType = 0;
    mHeader.uiMsgLen = 0;
  }

  void set_message(std::string content) {
    if (content.size() > max_body_length) {
      LOG(ERROR) << "message length is too large: " << content.size();
    }
    LOG(INFO) << "message content " << content;
    mHeader.uiType = 1;
    mHeader.uiMsgLen = content.size();
    encode_header();
    std::memcpy(data_ + header_length, content.c_str(), content.size());
  }

  std::string get_message() const {
    std::string m(body(), body_length());
    return std::move(m);
  }

  const char* data() const { return data_; }

  char* data() { return data_; }

  std::size_t length() const { return mHeader.uiMsgLen + header_length; }

  const char* body() const { return data_ + header_length; }

  char* body() { return data_ + header_length; }

  std::size_t body_length() const { return mHeader.uiMsgLen; }

  bool decode_header() {
    char len[5] = "";
    std::memcpy(len, data_, sizeof(unsigned int));
    mHeader.uiType = *((int*)len);
    LOG(INFO) << "message type: " << mHeader.uiType;

    std::memset(len, 0, 5);

    std::memcpy(len, data_ + sizeof(unsigned int), sizeof(unsigned int));
    mHeader.uiMsgLen = *((int*)len);
    LOG(INFO) << "message length: " << mHeader.uiMsgLen;

    return true;
  }

  void encode_header() {
    std::memset(data_, 0, max_body_length + header_length);
    std::memcpy(data_, &mHeader.uiType, sizeof(int));
    std::memcpy(data_ + sizeof(int), &mHeader.uiMsgLen, sizeof(int));
  }

 private:
  char data_[max_body_length + header_length];
  MsgHeader mHeader;
};

#endif  // CHAT_MESSAGE_HPP
