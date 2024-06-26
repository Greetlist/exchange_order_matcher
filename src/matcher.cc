#include "matcher.h"

Matcher::Matcher(std::string&& path, std::string&& uid, std::string&& exchange, double last_price) : market_data_path_(path), uid_(uid), exchange_(exchange), last_price_(last_price) {
}

Matcher::~Matcher() {
  match_thread_.join();
  read_order_thread_.join();
  std::cout << "[destructor] Order Queue len is: " << order_queue_.size() << std::endl;
  std::cout << "Quit Matcher" << std::endl;
}

void Matcher::Start() {
  match_thread_ = std::thread(&Matcher::Match, this);
  read_order_thread_ = std::thread(&Matcher::ReadOrderData, this);
}

void Matcher::Stop() {
  while (true) {
    std::unique_lock<std::mutex> inner_lock(cv_mutex_);
    if (!order_queue_.empty()) {
      cv_.notify_all();
      std::cout << "[Stop] Order Queue len is: " << order_queue_.size() << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
      std::cout << "[Stop] Order Queue len is: " << order_queue_.size() << std::endl;
      break;
    }
  }
  stop_ = true;
  std::unique_lock<std::mutex> lk(cv_mutex_);
  cv_.notify_all();
}

void Matcher::PrintOrderDepth() {
  std::map<double, int> m;
  for (const auto& [price, order] : sell_) {
    m[price] += order.GetVolume();
  }

  std::vector<std::pair<double, int>> v;
  for (const auto& [price, volume] : m) {
    v.emplace_back(price, volume);
  }

  std::cout << "=================================" << std::endl;
  std::cout << "Price    Volume" << std::endl;
  for (int i = v.size()-1; i >= 0; --i) {
    std::cout << v[i].first << "     " << v[i].second << std::endl;
  }

  std::cout << "---------------------------------" << std::endl;
  m.clear();
  v.clear();
  for (const auto& [price, order] : buy_) {
    m[price] += order.GetVolume();
  }

  for (const auto& [price, volume] : m) {
    v.emplace_back(price, volume);
  }
  for (int i = v.size()-1; i >= 0; --i) {
    std::cout << v[i].first << "     " << v[i].second << std::endl;
  }
  std::cout << "=================================" << std::endl;
}

void Matcher::Match() {
  while (!stop_) {
    std::unique_lock<std::mutex> lk(cv_mutex_);
    cv_.wait(lk, [=]{return order_queue_.size() > 0 || stop_;});
    while (!order_queue_.empty()) {
      Order order = order_queue_.front();
      order_queue_.pop_front();
      std::cout << "price:" << order.GetPrice() << ", Direction: " << static_cast<std::underlying_type_t<OrderDirection>>(order.GetDirection()) << std::endl;
      auto bid_iter = buy_.begin()++;
      auto ask_iter = sell_.begin()++;

      if (order.GetDirection() == OrderDirection::Buy) {
        int left_volume = order.GetVolume();
        if (ask_iter == sell_.end() || order.GetPrice() < ask_iter->first) {
          buy_.emplace(order.GetPrice(), order);
          continue;
        }
        while (order.GetPrice() >= ask_iter->first) { //can match
          auto [sell_order_begin, sell_order_end] = sell_.equal_range(ask_iter->first);
          for (; sell_order_begin != sell_order_end;) {
            int cur_volume = sell_order_begin->second.GetVolume();
            std::cout << "cur volume: " << cur_volume << std::endl;
            if (left_volume >= cur_volume) {
              left_volume -= cur_volume;
              sell_order_begin = sell_.erase(sell_order_begin);
            } else {
              sell_order_begin->second.SetVolume(cur_volume - left_volume);
              left_volume = 0;
              break;
            }
          }
          if (left_volume <= 0) break;
          ask_iter = sell_.begin();
        }

        if (left_volume > 0) {
          order.SetVolume(left_volume);
          buy_.emplace(order.GetPrice(), order);
        }
      } else if (order.GetDirection() == OrderDirection::Sell) {
        if (bid_iter == buy_.end() || order.GetPrice() > bid_iter->first) {
          sell_.emplace(order.GetPrice(), order);
          continue;
        }
        int left_volume = order.GetVolume();
        while (order.GetPrice() <= bid_iter->first) { //can match
          std::cout << "buy first: " << bid_iter->first << std::endl;
          auto [buy_order_begin, buy_order_end] = buy_.equal_range(bid_iter->first);
          for (; buy_order_begin != buy_order_end;) {
            int cur_volume = buy_order_begin->second.GetVolume();
            if (left_volume >= cur_volume) {
              left_volume -= cur_volume;
              buy_order_begin = buy_.erase(buy_order_begin);
            } else {
              buy_order_begin->second.SetVolume(cur_volume - left_volume);
              left_volume = 0;
              break;
            }
          }
          if (left_volume <= 0) break;
          bid_iter = buy_.begin();
        }

        if (left_volume > 0) {
          order.SetVolume(left_volume);
          sell_.emplace(order.GetPrice(), order);
        }
      } else {
        auto [bbegin, bend] = buy_.equal_range(order.GetPrice());
        auto bit = bbegin;
        for (;bit != bend;) {
          if (order.GetOrderId() == bit->second.GetOrderId()) {
            bit = buy_.erase(bit);
          }
        }
        auto [sbegin, send] = sell_.equal_range(order.GetPrice());
        auto sit = sbegin;
        for (;sit != send;) {
          if (order.GetOrderId() == sit->second.GetOrderId()) {
            sit = buy_.erase(sit);
          }
        }
      }
    }
    PrintOrderDepth();
  }
}

void Matcher::AcceptOrder(const Order&& o) {
  std::unique_lock<std::mutex> lk(cv_mutex_);
  order_queue_.push_back(o);
  cv_.notify_one();
}

// read order data
void Matcher::ReadOrderData() {
  std::vector<Order> ov {
    {"600759", "SH", "10000", OrderDirection::Buy, 2.49, 1000},
    {"600759", "SH", "10001", OrderDirection::Buy, 2.5, 1000},
    {"600759", "SH", "10002", OrderDirection::Buy, 2.51, 1000},
    {"600759", "SH", "10003", OrderDirection::Buy, 2.52, 1000},
    {"600759", "SH", "10004", OrderDirection::Buy, 2.53, 1000},
    {"600759", "SH", "10005", OrderDirection::Sell, 2.54, 1000},
    {"600759", "SH", "10006", OrderDirection::Sell, 2.55, 1000},
    {"600759", "SH", "10007", OrderDirection::Sell, 2.56, 1000},
    {"600759", "SH", "10008", OrderDirection::Sell, 2.57, 1000},
    {"600759", "SH", "10009", OrderDirection::Sell, 2.58, 1000},
    {"600759", "SH", "10010", OrderDirection::Buy, 2.56, 2200},
    {"600759", "SH", "10011", OrderDirection::Sell, 2.51, 3200}
  };

  for (auto& o : ov) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::unique_lock<std::mutex> lk(cv_mutex_);
    order_queue_.push_back(std::move(o));
    cv_.notify_one();
  }
}
