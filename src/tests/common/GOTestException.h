/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTEXCEPTION_H
#define GOTESTEXCEPTION_H
#include <cpptrace.hpp>
#include <exception>
#include <iostream>
#include <string>

class GOTestException : public std::exception {
private:
  std::string message;

public:
  GOTestException(std::string msg) : message(msg) {}
  std::string what() {
    std::string returned_message = message;
    returned_message = returned_message + "\n";

    return returned_message;
  }
};
#endif