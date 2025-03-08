#include <iostream>

#include "sunnet.h"

void test() {
  auto type = std::make_shared<std::string>("ping");
  auto ping1 = Sunnet::instance().newService(type);
  auto ping2 = Sunnet::instance().newService(type);
  auto pong = Sunnet::instance().newService(type);

  Sunnet::instance().Send(
      pong, ServiceMsg::makeMsg(ping1, new char[3]{'h', 'i', '\0'}, 3));
  Sunnet::instance().Send(
      pong, ServiceMsg::makeMsg(ping2,
                                new char[6]{'h', 'e', 'l', 'l', 'o', '\0'}, 6));
}

int main() {
  Sunnet::instance().start();
  test();
  Sunnet::instance().wait();
  return 0;
}