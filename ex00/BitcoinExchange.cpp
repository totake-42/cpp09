#include "BitcoinExchange.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>   // For std::isnan and std::isinf
#include <cerrno>  // For errno
#include <cstdlib> // For std::strtod

BitcoinExchange::BitcoinExchange()
{
}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &other)
{
	_database = other._database;
}

BitcoinExchange::~BitcoinExchange()
{
}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &other)
{
	if (this != &other)
	{
		_database = other._database;
	}
	return *this;
}

std::string BitcoinExchange::trimSpace(const std::string &str) const
{
	if (str.empty())
		return "";

	size_t start = 0;
	size_t end = str.length() - 1;

	while (start < end && std::isspace(static_cast<unsigned char>(str[start])))
		start++;
	while (end > start && std::isspace(static_cast<unsigned char>(str[end])))
		end--;

	return str.substr(start, end - start + 1);
}

bool BitcoinExchange::isValidDate(const std::string &date) const
{
	if (date.length() != 10 || date[4] != '-' || date[7] != '-')
		return (false);

	size_t i = 0;
	while (i < date.length())
	{
		if (i == 4 || i == 7)
		{
			i++;
			continue;
		}
		if (!std::isdigit(static_cast<unsigned char>(date[i])))
			return false;
		i++;
	}

	int year;
	int month;
	int day;
	char hyphen1;
	char hyphen2;

	std::istringstream iss(date);
	iss >> year >> hyphen1 >> month >> hyphen2 >> day;

	int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	bool isLeapYear = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
	if (isLeapYear)
		daysInMonth[1] = 29;

	if (month < 1 || month > 12)
		return false;
	if (day < 1 || day > daysInMonth[month - 1])
		return false;

	return true;
}

bool BitcoinExchange::isValidDouble(const std::string &str, double &value) const
{
	char *end;
	errno = 0;
	value = std::strtod(str.c_str(), &end);

	if (str.empty() || *end != '\0' || errno != 0)
		return false;

	if (std::isnan(value) || std::isinf(value))
		return false;

	return true;
}

bool BitcoinExchange::loadDatabase(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file)
	{
		std::cout << "Error: could not open database file." << std::endl;
		return (false);
	}

	std::string line;
	if (!std::getline(file, line))
	{
		std::cout << "Error: could not read the first line of the database file." << std::endl;
		return (false);
	}

	if (line != "date,exchange_rate")
	{
		std::cout << "Error: invalid first line of the database file." << std::endl;
		return (false);
	}

	while (std::getline(file, line))
	{
		size_t comma = line.find(',');
		if (comma == std::string::npos)
		{
			std::cout << "Error: invalid line format in the database file." << std::endl;
			return (false);
		}

		std::string date = trimSpace(line.substr(0, comma));
		std::string rate_str = trimSpace(line.substr(comma + 1));

		double rate;
		if (!isValidDate(date) || !isValidDouble(rate_str, rate) || rate < 0)
		{
			std::cout << "Error: invalid date or rate in the database file." << std::endl;
			return (false);
		}
		_database[date] = rate;
	}

	if (_database.empty())
	{
		std::cout << "Error: database is empty." << std::endl;
		return (false);
	}

	return (true);
}

bool BitcoinExchange::processInputFile(const std::string &filename)
{
	int emptyFlag = 0;

	std::ifstream file(filename.c_str());
	if (!file)
	{
		std::cout << "Error: could not open file." << std::endl;
		return (false);
	}

	std::string line;
	if (!std::getline(file, line))
	{
		std::cout << "Error: could not read the first line." << std::endl;
		return (false);
	}

	if (line != "date | value")
	{
		std::cout << "Error: invalid first line." << std::endl;
		return (false);
	}

	while (std::getline(file, line))
	{
		emptyFlag = 1;
		size_t sep = line.find('|');
		if (sep == std::string::npos)
		{
			std::cout << "Error: bad input => " << line << std::endl;
			continue;
		}
		std::string date = trimSpace(line.substr(0, sep));
		std::string value_str = trimSpace(line.substr(sep + 1));

		double value;
		if (!isValidDate(date) || !isValidDouble(value_str, value))
		{
			std::cout << "Error: bad input => " << line << std::endl;
			continue;
		}

		if (value < 0)
		{
			std::cout << "Error: not a positive number." << std::endl;
			continue;
		}

		if (value > 1000)
		{
			std::cout << "Error: too large a number." << std::endl;
			continue;
		}

		std::map<std::string, double>::const_iterator it = _database.upper_bound(date);
		if (it == _database.begin())
		{
			std::cout << "Error: bad input => " << line << std::endl;
			continue;
		}
		--it;

		std::cout << date << " => " << value << " = " << value * it->second << std::endl;
	}

	if (emptyFlag == 0)
	{
		std::cout << "Error: input file is empty." << std::endl;
		return (false);
	}

	return (true);
}
