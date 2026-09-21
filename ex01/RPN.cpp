#include "RPN.hpp"
#include <cctype>
#include <stack>
#include <stdexcept>
#include <sstream>

RPN::RPN()
{
}

RPN::RPN(const RPN &other)
{
	(void)other;
}

RPN::~RPN()
{
}

RPN &RPN::operator=(const RPN &other)
{
	(void)other;
	return (*this);
}

int RPN::calculate(const std::string &expression) const
{
	std::stack<int> values;
	std::istringstream stream(expression);
	std::string token;

	while (stream >> token)
	{
		if (token.length() != 1)
			throw std::runtime_error("Invalid token in expression");

		char value = token[0];

		if (std::isdigit(static_cast<unsigned char>(value)))
		{
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

		if (value == '/' && right == 0)
			throw std::runtime_error("Division by zero");

		int result = 0;

		if (value == '+')
			result = left + right;
		else if (value == '-')
			result = left - right;
		else if (value == '*')
			result = left * right;
		else if (value == '/')
			result = left / right;

		values.push(result);
	}

	if (values.size() != 1)
		throw std::runtime_error("Invalid expression");

	return values.top();
}
