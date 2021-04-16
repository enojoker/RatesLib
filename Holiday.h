#pragma once
#include <string>
#include <set>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>

typedef boost::gregorian::date date;
typedef boost::gregorian::date_duration date_dur;

class Holiday {

private:
	std::string holiday_code;
	std::set<date> holidays;

public:
	Holiday() {
		holiday_code = "US_SIFMA";
		std::fstream file;
		file.open("Holidays.txt", std::ios::in);
		if (file.is_open()) {
			std::string tp;
			while (getline(file, tp)) {
				holidays.insert(boost::gregorian::from_string(tp));
			}
			file.close();
		}
	}

	bool is_holiday(const date& day) const {
		auto it = this->holidays.find(day);
		return it != this->holidays.end();
	}

};

date busday_adj(const date& sd, const Holiday& holidays) {
	date current = sd;
	date_dur one_day_dur = date_dur(1);
	while (holidays.is_holiday(current) || current.day_of_week() == 0 || current.day_of_week() == 6) {
		current = current + one_day_dur;
	}
	return current;
}

date bus_day_shift(const date& sd, const int& shift, const Holiday& holidays) {
	date current = sd;
	if (shift == 0) {
		return busday_adj(current, holidays);
	}

	for (int i = 0; i < shift; i++) {
		current = busday_adj(current, holidays);
		current = current + date_dur(1);
	}

	current = busday_adj(current, holidays);
	return current;
}