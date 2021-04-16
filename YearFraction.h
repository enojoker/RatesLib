#pragma once
# include <boost/date_time/gregorian/gregorian.hpp>

typedef boost::gregorian::date date;
typedef boost::gregorian::date_duration date_dur;

enum DCC {ACT360, ACT365, ACTACT_ISDA, A30360};

double get_yf_act360(const date& start, const date& end) {
	date_dur dd = end - start;
	return (double)dd.days() / 360;

}

double get_yf_act365(const date& start, const date& end) {
	date_dur dd = end - start;
	return (double)dd.days() / 365;
}

double get_yf_30A360(const date& start, const date& end) {
	int y1 = start.year();
	int m1 = start.month();
	int d1 = start.day();
	int y2 = end.year();
	int m2 = end.month();
	int d2 = end.day();

	d1 = std::min(d1, 30);

	if (d1 == 30) {
		d2 = std::min(d2, 30);
	}

	return (double) (360 * (y2 - y1) + 30 * (m2 - m1) + (d2 - d1)) / 360;

}


double get_yf_actact(const date& start, const date& end) {
	return 0;
}

typedef double(*dcc_func) (const date& start, const date& end);


dcc_func get_yf_func(DCC dcc) {
	switch (dcc)
	{
	case ACT360:
		return &get_yf_act360;
	case ACT365:
		return &get_yf_act365;
	case ACTACT_ISDA:
		return &get_yf_actact;
	case A30360:
		return &get_yf_30A360;
	default:
		return &get_yf_act360;
	}

}