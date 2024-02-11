/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include <boost/stacktrace.hpp>
#include <exception>
#include <iostream>

class GOTestException : public std::exception {
private:
  char *message;

public:
  GOTestException(char *msg) : message(msg) {}
  std::string what() {
    std::string returned_message = message;
    returned_message = returned_message + "\n";
    std::cerr << boost::stacktrace::stacktrace();
    return returned_message;
  }
};
