#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>

#include "asio.hpp"
#include "message.h"
#include "proto/a.pb.h"

// glog
#include "glog/logging.h"

using asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<message> message_queue;

//----------------------------------------------------------------------

class chat_participant {
 public:
  virtual ~chat_participant() {}
  virtual void deliver(const message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room {
 public:
  void join(chat_participant_ptr participant) {
    participants_.insert(participant);
    for (auto msg : recent_msgs_)
      participant->deliver(msg);
  }

  void leave(chat_participant_ptr participant) {
    participants_.erase(participant);
  }

  void deliver(const message& msg) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

    for (auto participant : participants_)
      participant->deliver(msg);
  }

 private:
  std::set<chat_participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session : public chat_participant,
                     public std::enable_shared_from_this<chat_session> {
 public:
  chat_session(tcp::socket socket, chat_room& room)
      : socket_(std::move(socket)), room_(room) {}

  void start() {
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const message& msg) {
    std::cout << "deliver : " << msg.get_message() << std::endl;
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write();
    }
  }

 private:
  void do_read_header() {
    auto self(shared_from_this());
    asio::async_read(socket_,
                     asio::buffer(read_msg_.data(), message::header_length),
                     [this, self](std::error_code ec, std::size_t length) {
                       LOG(INFO) << "do read header call back! msg content: "
                                 << read_msg_.data() << " msg size: " << length
                                 << " error code: " << ec;
                       if (!ec && read_msg_.decode_header()) {
                         do_read_body();
                       } else {
                         room_.leave(shared_from_this());
                       }
                     });
  }

  void do_read_body() {
    auto self(shared_from_this());
    asio::async_read(socket_,
                     asio::buffer(read_msg_.body(), read_msg_.body_length()),
                     [this, self](std::error_code ec, std::size_t length) {
                       if (!ec) {
                         LOG(INFO) << "do read body call back! len: " << length
                                   << " content: " << read_msg_.get_message();
                         room_.deliver(read_msg_);
                         do_read_header();
                       } else {
                         room_.leave(shared_from_this());
                       }
                     });
  }

  void do_write() {
    LOG(INFO) << "do write";
    auto self(shared_from_this());
    asio::async_write(
        socket_,
        asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
        [this, self](std::error_code ec, std::size_t length) {
          if (!ec) {
            LOG(INFO) << "do write call back! length: " << length << " "
                      << "content: " << write_msgs_.front().get_message();
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
              do_write();
            }
          } else {
            room_.leave(shared_from_this());
          }
        });
  }

  tcp::socket socket_;
  chat_room& room_;
  message read_msg_;
  message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server {
 public:
  chat_server(asio::io_context& io_context, const tcp::endpoint& endpoint)
      : acceptor_(io_context, endpoint) {
    do_accept();
  }

 private:
  void do_accept() {
    acceptor_.async_accept([this](std::error_code ec, tcp::socket socket) {
      LOG(INFO) << "do accept call back";
      if (!ec) {
        std::make_shared<chat_session>(std::move(socket), room_)->start();
      }

      do_accept();
    });
  }

  tcp::acceptor acceptor_;
  chat_room room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[]) {
  // Initialize Google's logging library.
  google::InitGoogleLogging("MyServer");
  google::SetLogDestination(google::GLOG_INFO, "./log/info/");

  // LOG(INFO) << "pb message header :" << sizeof(MsgHeader) << "\n byte size: "
  // << MsgHeader().ByteSizeLong() << std::endl;
  try {
    if (argc < 2) {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    asio::io_context io_context;

    std::list<chat_server> servers;
    for (int i = 1; i < argc; ++i) {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  google::ShutdownGoogleLogging();

  return 0;
}
