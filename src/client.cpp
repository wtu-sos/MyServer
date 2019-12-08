#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>

#include "asio.hpp"
#include "message.h"

using asio::ip::tcp;

typedef std::deque<message> message_queue;

class chat_client {
 public:
  chat_client(asio::io_context& io_context,
              const tcp::resolver::results_type& endpoints)
      : io_context_(io_context), socket_(io_context) {
    do_connect(endpoints);
  }

  void write(const message& msg) {
    asio::post(io_context_, [this, msg]() {
      bool write_in_progress = !write_msgs_.empty();
      write_msgs_.push_back(msg);
      if (!write_in_progress) {
        do_write();
      }
    });
  }

  void close() {
    asio::post(io_context_, [this]() { socket_.close(); });
  }

 private:
  void do_connect(const tcp::resolver::results_type& endpoints) {
    asio::async_connect(socket_, endpoints,
                        [this](std::error_code ec, tcp::endpoint) {
                          if (!ec) {
                            do_read_header();
                          }
                        });
  }

  void do_read_header() {
    asio::async_read(socket_,
                     asio::buffer(read_msg_.data(), message::header_length),
                     [this](std::error_code ec, std::size_t /*length*/) {
                       if (!ec && read_msg_.decode_header()) {
                         do_read_body();
                       } else {
                         socket_.close();
                       }
                     });
  }

  void do_read_body() {
    asio::async_read(
        socket_, asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](std::error_code ec, std::size_t length) {
          if (!ec) {
            // std::cout.write(read_msg_.body(), read_msg_.body_length());
            std::cout << __LINE__ << " recivied: " << length
                      << "\n message: " << read_msg_.get_message() << std::endl;
            // std::cout << "\n";
            do_read_header();
          } else {
            socket_.close();
          }
        });
  }

  void do_write() {
    asio::async_write(
        socket_,
        asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
        [this](std::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
              do_write();
            }
          } else {
            std::cout << "error : " << ec << std::endl;
            socket_.close();
          }
        });
  }

 private:
  asio::io_context& io_context_;
  tcp::socket socket_;
  message read_msg_;
  message_queue write_msgs_;
};

int main(int argc, char* argv[]) {
  // Initialize Google's logging library.
  google::InitGoogleLogging("MyClient");
  google::SetLogDestination(google::GLOG_INFO, "./log/info/");
  try {
    if (argc != 3) {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
    }

    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    chat_client c(io_context, endpoints);

    std::thread t([&io_context]() { io_context.run(); });

    char line[message::max_body_length + 1];
    unsigned int id = 1;
    std::cout << "input : " << line;
    while (std::cin.getline(line, message::max_body_length + 1)) {
      std::cout << "input : ";
      message msg;
      // msg.set_id(id++);
      // msg.body_length(std::strlen(line));
      msg.set_message(line);
      LOG(INFO) << "send message : " << msg.get_message();
      c.write(msg);
    }

    c.close();
    t.join();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  google::ShutdownGoogleLogging();

  return 0;
}
