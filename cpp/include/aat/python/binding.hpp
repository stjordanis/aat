#pragma once
#define AAT_PYTHON

#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11_json/pybind11_json.hpp>

#include <aat/common.hpp>
#include <aat/config/enums.hpp>
#include <aat/core/models/data.hpp>
#include <aat/core/models/event.hpp>
#include <aat/core/models/order.hpp>
#include <aat/core/models/trade.hpp>
#include <aat/core/order_book/order_book.hpp>

namespace py = pybind11;
using namespace aat::common;

PYBIND11_MODULE(binding, m) {
  m.doc() = "C++ bindings";

  /*******************************
   * Enums
   ******************************/
  using namespace aat::config;
  py::enum_<Side>(m, "SideCpp", py::arithmetic()).value("BUY", Side::BUY).value("SELL", Side::SELL).export_values();

  py::enum_<EventType>(m, "EventTypeCpp", py::arithmetic())
    .value("TRADE", EventType::TRADE)
    .value("OPEN", EventType::OPEN)
    .value("CANCEL", EventType::CANCEL)
    .value("CHANGE", EventType::CHANGE)
    .value("FILL", EventType::FILL)
    .value("DATA", EventType::DATA)
    .value("HALT", EventType::HALT)
    .value("CONTINUE", EventType::CONTINUE)
    .value("ERROR", EventType::ERROR)
    .value("START", EventType::START)
    .value("EXIT", EventType::EXIT)
    .export_values();

  py::enum_<DataType>(m, "DataTypeCpp", py::arithmetic())
    .value("ORDER", DataType::ORDER)
    .value("TRADE", DataType::TRADE)
    .export_values();

  py::enum_<InstrumentType>(m, "InstrumentTypeCpp", py::arithmetic())
    .value("CURRENCY", InstrumentType::CURRENCY)
    .value("EQUITY", InstrumentType::EQUITY)
    .export_values();

  py::enum_<OrderType>(m, "OrderTypeCpp", py::arithmetic())
    .value("LIMIT", OrderType::LIMIT)
    .value("MARKET", OrderType::MARKET)
    .value("STOP", OrderType::STOP)
    .export_values();

  py::enum_<OrderFlag>(m, "OrderFlagCpp", py::arithmetic())
    .value("NONE", OrderFlag::NONE)
    .value("FILL_OR_KILL", OrderFlag::FILL_OR_KILL)
    .value("ALL_OR_NONE", OrderFlag::ALL_OR_NONE)
    .value("IMMEDIATE_OR_CANCEL", OrderFlag::IMMEDIATE_OR_CANCEL)
    .export_values();

  /*******************************
   * OrderBook
   ******************************/
  using namespace aat::core;
  py::class_<OrderBook>(m, "OrderBookCpp")
    .def(py::init<Instrument&>())
    .def(py::init<Instrument&, ExchangeType&>())
    .def(py::init<Instrument&, ExchangeType&, std::function<void(std::shared_ptr<Event>)>>())
    .def("__repr__", &OrderBook::toString)
    .def(
      "__iter__", [](const OrderBook& o) { return py::make_iterator(o.begin(), o.end()); },
      py::keep_alive<0, 1>()) /* Essential: keep object alive while iterator exists */
    .def("setCallback", &OrderBook::setCallback)
    .def("add", &OrderBook::add)
    .def("cancel", &OrderBook::cancel)
    .def("topOfBook", &OrderBook::topOfBookMap)
    .def("spread", &OrderBook::spread)
    .def("level", (std::vector<std::shared_ptr<PriceLevel>>(OrderBook::*)(double) const) & OrderBook::level)
    .def("level", (std::vector<double>(OrderBook::*)(std::uint64_t) const) & OrderBook::level)
    .def("levels", &OrderBook::levelsMap);

  /*******************************
   * Exchange
   ******************************/
  py::class_<ExchangeType>(m, "ExchangeTypeCpp")
    .def(py::init<const str_t&>())
    .def("__init__", [](py::object obj) { return ExchangeType(obj.cast<str_t>()); })
    .def("__repr__", &ExchangeType::toString);

  /*******************************
   * Instrument
   ******************************/
  py::class_<Instrument>(m, "InstrumentCpp")
    .def(py::init<const str_t&, InstrumentType&>())
    .def(py::init<const str_t&>())
    .def("__repr__", &Instrument::toString)
    .def("__eq__", &Instrument::operator==);

  /*******************************
   * Models
   ******************************/
  py::class_<Data, std::shared_ptr<Data>>(m, "DataCpp")
    .def(py::init<uint_t, timestamp_t, double, double, Side, DataType, Instrument, ExchangeType, double>())
    .def("__repr__", &Data::toString)
    .def("__eq__", &Data::operator==)
    .def("__lt__", &Data::operator<)  // NOLINT
    .def("toJson", &Data::toJson)
    .def("perspectiveSchema", &Data::perspectiveSchema)
    .def_readwrite("id", &Data::id)
    .def_readwrite("timestamp", &Data::timestamp)
    .def_readwrite("volume", &Data::volume)
    .def_readwrite("price", &Data::price)
    .def_readonly("side", &Data::side)
    .def_readonly("type", &Data::type)
    .def_readonly("instrument", &Data::instrument)
    .def_readonly("exchange", &Data::exchange)
    .def_readwrite("filled", &Data::filled);

  py::class_<Event>(m, "EventCpp")
    .def(py::init<EventType, std::shared_ptr<Data>>(), py::arg("type").none(false),
      py::arg("target").none(true))  // allow None, convert to nullptr
    .def(py::init<EventType, std::shared_ptr<Trade>>())
    .def(py::init<EventType, std::shared_ptr<Order>>())
    .def("__repr__", &Event::toString)
    .def("toJson", &Event::toJson)
    .def_readonly("type", &Event::type)
    .def_readonly("target", &Event::target);

  py::class_<Order, Data, std::shared_ptr<Order>>(m, "OrderCpp")
    .def(py::init<uint_t, timestamp_t, double, double, Side, Instrument, ExchangeType, double, OrderType, OrderFlag,
      std::shared_ptr<Order>, double>())
    .def("__repr__", &Order::toString)
    .def("toJson", &Order::toJson)
    .def("perspectiveSchema", &Order::perspectiveSchema)
    .def_readwrite("id", &Order::id)
    .def_readwrite("timestamp", &Order::timestamp)
    .def_readwrite("volume", &Order::volume)
    .def_readwrite("price", &Order::price)
    .def_readonly("side", &Order::side)
    .def_readonly("type", &Order::type)
    .def_readonly("instrument", &Order::instrument)
    .def_readonly("exchange", &Order::exchange)
    .def_readwrite("filled", &Order::filled)
    .def_readonly("order_type", &Order::order_type)
    .def_readonly("flag", &Order::flag)
    .def_readonly("stop_target", &Order::stop_target)
    .def_readwrite("notional", &Order::notional);

  py::class_<Trade, Data, std::shared_ptr<Trade>>(m, "TradeCpp")
    .def(py::init<uint_t, timestamp_t, double, double, Side, Instrument, ExchangeType, double,
      std::deque<std::shared_ptr<Order>>, std::shared_ptr<Order>>())
    .def("__repr__", &Trade::toString)
    .def("slippage", &Trade::slippage)
    .def("transactionCost", &Trade::transactionCost)
    .def("toJson", &Trade::toJson)
    .def("perspectiveSchema", &Trade::perspectiveSchema)
    .def_readwrite("id", &Trade::id)
    .def_readwrite("timestamp", &Trade::timestamp)
    .def_readwrite("volume", &Trade::volume)
    .def_readwrite("price", &Trade::price)
    .def_readonly("side", &Trade::side)
    .def_readonly("type", &Trade::type)
    .def_readonly("instrument", &Trade::instrument)
    .def_readonly("exchange", &Trade::exchange)
    .def_readwrite("filled", &Trade::filled)
    .def_readwrite("maker_orders", &Trade::maker_orders)
    .def_readwrite("taker_order", &Trade::taker_order);

  /*******************************
   * Helpers
   ******************************/
  // NONE
}
