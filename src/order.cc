#include "order.h"

Order::Order(std::string&& uid, std::string&& exchange, std::string&& order_id, OrderDirection direction, double price, int volume) : uid_(uid), exchange_(exchange), order_id_(order_id), direction_(direction), price_(price), volume_(volume) {
}

bool Order::operator<(const Order& r) {
  return order_id_ < r.order_id_;
}
