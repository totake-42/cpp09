#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <string>

class BitcoinExchange
{
private:
	std::map<std::string, double> _database;

	std::string trimSpace(const std::string &str) const;
	bool isValidDate(const std::string &date) const;
	bool isValidDouble(const std::string &str, double &value) const;

public:
	BitcoinExchange();
	BitcoinExchange(const BitcoinExchange &other);
	~BitcoinExchange();
	BitcoinExchange &operator=(const BitcoinExchange &other);

	bool loadDatabase(const std::string &filename);
	bool processInputFile(const std::string &filename);
};

#endif
