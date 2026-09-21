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
	bool isValidRate(const std::string &rate_str, double &rate) const;

public:
	bool loadDatabase(const std::string &filename);
};

#endif