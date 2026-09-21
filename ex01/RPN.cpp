#include "RPN.hpp"
#include <cctype>
#include <stack>
#include <stdexcept>

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

	std::size_t i = 0;
	while (i < expression.length())
	{
		char token = expression[i];

		if (std::isspace(static_cast<unsigned char>(token)))
		{
			i++;
			continue;
		}

		if (std::isdigit(static_cast<unsigned char>(token)))
		{
			values.push(token - '0');
			i++;
			continue;
		}

		if (token != '+' && token != '-' && token != '*' && token != '/')
			throw std::runtime_error("Invalid token in expression");

		if (values.size() < 2)
			throw std::runtime_error("Insufficient values in expression");

		int right = values.top();
		values.pop();

		int left = values.top();
		values.pop();

		if (token == '/' && right == 0)
			throw std::runtime_error("Division by zero");

		int result = 0;

		if (token == '+')
			result = left + right;
		else if (token == '-')
			result = left - right;
		else if (token == '*')
			result = left * right;
		else if (token == '/')
			result = left / right;

		values.push(result);
		i++;
	}

	if (values.size() != 1)
		throw std::runtime_error("Invalid expression");

	return values.top();
}
