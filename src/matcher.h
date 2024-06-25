#ifndef __MATCHER_H_
#define __MATCHER_H_

#include <atomic>
#include <string>
#include <deque>
#include <map>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>

#include "order.h"

class Matcher {
public:
  Matcher(std::string&& path, std::string&& uid, std::string&& exchange,double last_price);
  ~Matcher();
  void Start();
  void Stop();
  void PrintOrderDepth();
private:
  void ReadOrderData();
  void AcceptOrder(const Order&& o);
  void Match();
  std::string market_data_path_;
  std::string uid_;
  std::string exchange_;
  double last_price_;
  double price_change_limit_;
  std::multimap<double, Order, std::greater<double>> buy_;
  std::multimap<double, Order> sell_;
  std::deque<Order> order_queue_;
  std::thread match_thread_;
  std::thread read_order_thread_;
  std::mutex cv_mutex_;
  std::condition_variable cv_;
  std::atomic<bool> stop_ = false;
};

#endif
