#pragma once
#include <string>
#include <boost/algorithm/string.hpp>

double tenor_to_year(std::string tenor) {
	boost::to_upper(tenor);
	if (tenor.back() == 'Y') {
		tenor.pop_back();
		double num = stod(tenor);
		return num;
	}
	else if (tenor.back() == 'M') {
		tenor.pop_back();
		double num = stod(tenor);
		return num / 12;

	}
	else {
	// TODO throw an error
		return 0;
	}
	

}