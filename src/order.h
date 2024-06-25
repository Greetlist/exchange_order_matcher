#ifndef __ORDER_H_
#define __ORDER_H_

#include <string>

enum class OrderDirection {
  Buy = 1,
  Sell = 2,
  Cancel = 3,
};

class Order {
public:
  Order(std::string&& uid, std::string&& exchange, std::string&& order_id, OrderDirection direction, double price, int volume);
  ~Order() = default;
  bool operator<(const Order& r);
  inline std::string GetUid() {return uid_;}
  inline std::string GetExchange() {return exchange_;}
  inline std::string GetOrderId() {return order_id_;}
  inline OrderDirection GetDirection() {return direction_;}
  inline double GetPrice() {return price_;}
  inline int GetVolume() const {return volume_;}
  void SetVolume(int volume) {volume_ = volume;}
private:
  std::string uid_;
  std::string exchange_;
  std::string order_id_;
  OrderDirection direction_;
  double price_;
  int volume_;
};

#endif
