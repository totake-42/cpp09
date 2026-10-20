#include "RPN.hpp"
#include <cctype>
#include <climits>
#include <list>
#include <sstream>
#include <stack>
#include <stdexcept>

RPN::RPN() {}

RPN::RPN(const RPN &other) { (void)other; }

RPN::~RPN() {}

RPN &RPN::operator=(const RPN &other) {
  (void)other;
  return (*this);
}

int RPN::calculate(const std::string &expression) const {
  std::stack<int, std::list<int> > values;
  std::istringstream stream(expression);
  std::string token;

  while (stream >> token) {
    if (token.length() != 1)
      throw std::runtime_error("Invalid token in expression");

    char value = token[0];

    if (std::isdigit(static_cast<unsigned char>(value))) {
      values.push(value - '0');
      continue;
    }

    if (value != '+' && value != '-' && value != '*' && value != '/')
      throw std::runtime_error("Invalid token in expression");

    if (values.size() < 2)
      throw std::runtime_error("Insufficient values in expression");

    int right = values.top();
    values.pop();

    int left = values.top();
    values.pop();

    int result = 0;

    if (value == '+') {
      if ((right > 0 && left > INT_MAX - right) ||
          (right < 0 && left < INT_MIN - right))
        throw std::overflow_error("Integer overflow");
      result = left + right;
    } else if (value == '-') {
      if ((right < 0 && left > INT_MAX + right) ||
          (right > 0 && left < INT_MIN + right))
        throw std::overflow_error("Integer overflow");
      result = left - right;
    } else if (value == '*') {
      if ((left > 0 && ((right > 0 && left > INT_MAX / right) ||
                        (right < 0 && right < INT_MIN / left))) ||
          (left < 0 && ((right > 0 && left < INT_MIN / right) ||
                        (right < 0 && right < INT_MAX / left))))
        throw std::overflow_error("Integer overflow");
      result = left * right;
    } else if (value == '/') {
      if (right == 0)
        throw std::runtime_error("Division by zero");
      if (left == INT_MIN && right == -1)
        throw std::overflow_error("Integer overflow");
      result = left / right;
    }

    values.push(result);
  }

  if (values.size() != 1)
    throw std::runtime_error("Invalid expression");

  return values.top();
}
