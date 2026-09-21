#include "BitcoinExchange.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

std::string BitcoinExchange::trimSpace(const std::string &str) const
{
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

bool BitcoinExchange::isValidRate(const std::string &rate_str, double &rate) const
{
	char *end;
	errno = 0;
	rate = std::strtod(rate_str.c_str(), &end);

	if (rate_str.empty() || *end != '\0' || errno != 0)
		return false;

	return true;
}

bool BitcoinExchange::loadDatabase(const std::string &filename)
{
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

	if (line != "date,exchange_rate")
	{
		std::cout << "Error: invalid first line." << std::endl;
		return (false);
	}

	while (std::getline(file, line))
	{
		size_t comma = line.find(',');
		if (comma == std::string::npos)
		{
			std::cout << "Error: invalid line format." << std::endl;
			return (false);
		}

		std::string date = trimSpace(line.substr(0, comma));
		std::string rate_str = trimSpace(line.substr(comma + 1));

		double rate;
		if (!isValidDate(date) || !isValidRate(rate_str, rate))
		{
			std::cout << "Error: invalid date or rate." << std::endl;
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
	std::ifstream file(filename.c_str());
	if (!file)
	{
		std::cout << "Error: could not open file." << std::endl;
		return (false);
	}
}