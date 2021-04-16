#pragma once

#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include "YearFraction.h"
#include "Utility.h"
#include <memory>

typedef boost::gregorian::date date;
enum CCY {USD,JPY,GBP,AUD,EUR};

class Instrument {
protected:
	int _id;
	CCY _ccy;
};

class CashFlow : public Instrument {

protected:
	double _notional;
	date _accrual_start;
	date _accrual_end;
	date _payment_date;
	double _accural_yf;
	DCC _accural_dcc; // TODO update this to be enum;

	void calc_accural_yf() {
		dcc_func dcc_fp = get_yf_func(_accural_dcc);
		_accural_yf = dcc_fp(_accrual_start, _accrual_end);
	}

	CashFlow(const double& notional,
		const date& start,
		const date& end,
		const DCC& accural_dcc) : 
		_notional{ notional },
		_accrual_start{ start },
		_accrual_end{ end },
		_accural_dcc{ accural_dcc } {}

public:
	date get_acc_start() { return _accrual_start; }
	date get_acc_end() { return _accrual_end; }

	virtual double get_flow() = 0;
};

class FixedCashFlow : public CashFlow {
protected:
	double _fixed_rate;

public:
	// TODO constructor
	double get_flow() {
		return this->_notional *this-> _accural_yf * this-> _fixed_rate;

	}

	FixedCashFlow(const double& notional,
		const double& fixed_rate,
		const date& start,
		const date& end,
		const DCC& accural_dcc) :
		CashFlow(notional, start, end, accural_dcc), _fixed_rate{ fixed_rate }
	{
		// TODO busday modification 
		this->_payment_date = this->_accrual_end;
		this->calc_accural_yf();
	}
};

class CashFlowSchedule {

protected:
	date _start_date;
	date _end_date;
	int _payment_feq; // Annual:1, semi-Ann:2, quartly: 4, monthly: 12
	std::string _tenor;
	std::vector<std::unique_ptr<CashFlow>> CFs;
	//std::unique_ptr<CashFlow*> CFs;


public:
	CashFlowSchedule(double notional, double fixed_rate, date start_date, int feq, std::string tenor  ) : _start_date{ start_date }, _payment_feq{ feq }, _tenor{ tenor }
	{
		int num_flows = (int) tenor_to_year(tenor) * feq;
		auto interval = boost::gregorian::months(12 / feq);
		date sd = start_date;
		date ed = start_date + interval;
		// for now generate cashflow schedule foward wihtout worry about stubs
		for (int i = 0; i < num_flows; i++) {
			// do busday adj
			auto cf = std::make_unique<FixedCashFlow>(notional, fixed_rate, sd, ed, A30360);
			sd = ed;
			ed = ed + interval;			
			CFs.push_back(std::move(cf));
		}
	};

	void show() {
		for (auto&& cf : CFs)
			std::cout << "start: "<< cf->get_acc_start() << " end: "<< cf->get_acc_end() <<"  "<< cf->get_flow() << std::endl;
	}
};

class FixedCouponBond {

};