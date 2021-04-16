// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <boost/math/differentiation/autodiff.hpp>
#include "Instrument.h"
#include "Interpolator.h"
#include "Holiday.h"
#include "Utility.h"
#include "YearFraction.h"

using namespace boost::math::differentiation;
typedef boost::gregorian::date date;


//class FixedCashFlow {
//private:
//    double _flow;
//
//public:
//    double accural_yf;
//    double coupon_rate;
//    double payment_date;
//    double notional;
//
//    FixedCashFlow() {
//        accural_yf = 1.0;
//        coupon_rate = 0.02;
//        notional = 1000000.0;
//
//    }
//
//    double get_flow() {
//        _flow = notional * accural_yf * coupon_rate;
//        return _flow;
//    }
//
//};
//
//double get_dsc() {
//    return 0.9;
//}
//
//template <typename T>
//T plus(T const& x, T const& y) {
//
//    return x + y;
//}
//
//static class FixedFlowCalculator {
//
//private:
//    const static unsigned Order = 2;                  // Highest order derivative to be calculated.
//    
//    template <typename T>
//    static T _price(double const& flow, T const& DF) {
//        return flow * DF;
//    }
//
//public:
//    auto static price(FixedCashFlow& cf) {
//        auto const df = make_fvar<double, Order>(get_dsc());
//        auto const pv = _price(cf.get_flow(), df);
//        return pv;
//    }
//};


//int main()
//{
//    deriv2 deriv_obj = deriv2(2.0);
//    deriv_obj.show_value();
//    std::cout << "Hello World!\n";
//
//    FixedCashFlow a_cash = FixedCashFlow();
//    std::cout << "my flow is: " << a_cash.get_flow() << std::endl;
//    auto pv = FixedFlowCalculator::price(a_cash);
//
//    std::cout << "PV = " << pv.derivative(0) << std::endl;
//    std::cout << "dPV/dDF = " << pv.derivative(1) << std::endl;
//
//    auto pv2 = plus(pv,pv);
//    std::cout << "PV = " << pv2.derivative(0) << std::endl;
//    std::cout << "dPV/dDF = " << pv2.derivative(1) << std::endl;
//
//    //using namespace boost::math::differentiation;
//
//    //constexpr unsigned Order = 5;                  // Highest order derivative to be calculated.
//    //auto const x = make_fvar<double, Order>(2.0);  // Find derivatives at x=2.
//    //auto const y = fourth_power(x);
//    //for (unsigned i = 0; i <= Order; ++i)
//    //    std::cout << "y.derivative(" << i << ") = " << y.derivative(i) << std::endl;
//
//}

// Equations and function/variable names are from
// https://en.wikipedia.org/wiki/Greeks_(finance)#Formulas_for_European_option_Greeks
// Standard normal probability density function
template<typename X>
X phi(const X& x)
{
    return boost::math::constants::one_div_root_two_pi<double>() * exp(-0.5 * x * x);
}
// Standard normal cumulative distribution function
template<typename X>
X Phi(const X& x)
{
    return 0.5 * erfc(-boost::math::constants::one_div_root_two<double>() * x);
}
enum CP { call, put };
// Assume zero annual dividend yield (q=0).
using namespace boost::math::differentiation;

template<typename Price, typename Sigma, typename Tau, typename Rate>
promote<Price, Sigma, Tau, Rate>
black_scholes_option_price(CP cp, double K, const Price& S, const Sigma& sigma, const Tau& tau, const Rate& r)
{
    using namespace std;
    const auto d1 = (log(S / K) + (r + sigma * sigma / 2) * tau) / (sigma * sqrt(tau));
    const auto d2 = (log(S / K) + (r - sigma * sigma / 2) * tau) / (sigma * sqrt(tau));
    if (cp == call)
        return S * Phi(d1) - exp(-r * tau) * K * Phi(d2);
    else
        return exp(-r * tau) * K * Phi(-d2) - S * Phi(-d1);
}


int main()
{
    const double K = 100.0; // Strike price.
    auto const S = make_fvar<double, 3>(105); // Stock price.
    auto const sigma = make_fvar<double, 0, 3>(5); // Volatility.
    auto const tau = make_fvar<double, 0, 0, 1>(30.0 / 365);  // Time to expiration in years. (30 days).
    auto const r = make_fvar<double, 0, 0, 0, 1>(1.25 / 100); // Interest rate.

    const auto call_price = black_scholes_option_price(call, K, S, sigma, tau, r);
    const auto put_price = black_scholes_option_price(put, K, S, sigma, tau, r);
    // Compare automatically calculated greeks by autodiff with formulas for greeks.
    // https://en.wikipedia.org/wiki/Greeks_(finance)#Formulas_for_European_option_Greeks

    const double d1 = static_cast<double>((log(S / K) + (r + sigma * sigma / 2) * tau) / (sigma * sqrt(tau)));
    const double d2 = static_cast<double>((log(S / K) + (r - sigma * sigma / 2) * tau) / (sigma * sqrt(tau)));
    const double formula_call_delta = +Phi(+d1);
    const double formula_put_delta = -Phi(-d1);
    const double formula_vega = static_cast<double>(S * phi(d1) * sqrt(tau));
    const double formula_call_theta = static_cast<double>(-S * phi(d1) * sigma / (2 * sqrt(tau)) - r * K * exp(-r * tau) * Phi(+d2));
    const double formula_put_theta = static_cast<double>(-S * phi(d1) * sigma / (2 * sqrt(tau)) + r * K * exp(-r * tau) * Phi(-d2));
    const double formula_call_rho = static_cast<double>(+K * tau * exp(-r * tau) * Phi(+d2));
    const double formula_put_rho = static_cast<double>(-K * tau * exp(-r * tau) * Phi(-d2));
    const double formula_gamma = static_cast<double>(phi(d1) / (S * sigma * sqrt(tau)));
    const double formula_vanna = static_cast<double>(-phi(d1) * d2 / sigma);
    const double formula_charm = static_cast<double>(phi(d1) * (d2 * sigma * sqrt(tau) - 2 * r * tau) / (2 * tau * sigma * sqrt(tau)));
    const double formula_vomma = static_cast<double>(S * phi(d1) * sqrt(tau) * d1 * d2 / sigma);
    const double formula_veta = static_cast<double>(-S * phi(d1) * sqrt(tau) * (r * d1 / (sigma * sqrt(tau)) - (1 + d1 * d2) / (2 * tau)));
    const double formula_speed = static_cast<double>(-phi(d1) * (d1 / (sigma * sqrt(tau)) + 1) / (S * S * sigma * sqrt(tau)));
    const double formula_zomma = static_cast<double>(phi(d1) * (d1 * d2 - 1) / (S * sigma * sigma * sqrt(tau)));
    const double formula_color =
        static_cast<double>(-phi(d1) / (2 * S * tau * sigma * sqrt(tau)) * (1 + (2 * r * tau - d2 * sigma * sqrt(tau)) * d1 / (sigma * sqrt(tau))));
    const double formula_ultima = -formula_vega * static_cast<double>((d1 * d2 * (1 - d1 * d2) + d1 * d1 + d2 * d2) / (sigma * sigma));
    std::cout << std::setprecision(std::numeric_limits<double>::digits10)
        << "autodiff black-scholes call price = " << call_price.derivative(0, 0, 0, 0) << '\n'
        << "autodiff black-scholes put  price = " << put_price.derivative(0, 0, 0, 0) << '\n'
        << "\n## First-order Greeks\n"
        << "autodiff call delta = " << call_price.derivative(1, 0, 0, 0) << '\n'
        << " formula call delta = " << formula_call_delta << '\n'
        << "autodiff call vega  = " << call_price.derivative(0, 1, 0, 0) << '\n'
        << " formula call vega  = " << formula_vega << '\n'
        << "autodiff call theta = " << -call_price.derivative(0, 0, 1, 0) << '\n' // minus sign due to tau = T-time
        << " formula call theta = " << formula_call_theta << '\n'
        << "autodiff call rho   = " << call_price.derivative(0, 0, 0, 1) << '\n'
        << " formula call rho   = " << formula_call_rho << '\n'
        << '\n'
        << "autodiff put delta = " << put_price.derivative(1, 0, 0, 0) << '\n'
        << " formula put delta = " << formula_put_delta << '\n'
        << "autodiff put vega  = " << put_price.derivative(0, 1, 0, 0) << '\n'
        << " formula put vega  = " << formula_vega << '\n'
        << "autodiff put theta = " << -put_price.derivative(0, 0, 1, 0) << '\n'
        << " formula put theta = " << formula_put_theta << '\n'
        << "autodiff put rho   = " << put_price.derivative(0, 0, 0, 1) << '\n'
        << " formula put rho   = " << formula_put_rho << '\n'
        << "\n## Second-order Greeks\n"
        << "autodiff call gamma = " << call_price.derivative(2, 0, 0, 0) << '\n'
        << "autodiff put  gamma = " << put_price.derivative(2, 0, 0, 0) << '\n'
        << "      formula gamma = " << formula_gamma << '\n'
        << "autodiff call vanna = " << call_price.derivative(1, 1, 0, 0) << '\n'
        << "autodiff put  vanna = " << put_price.derivative(1, 1, 0, 0) << '\n'
        << "      formula vanna = " << formula_vanna << '\n'
        << "autodiff call charm = " << -call_price.derivative(1, 0, 1, 0) << '\n'
        << "autodiff put  charm = " << -put_price.derivative(1, 0, 1, 0) << '\n'
        << "      formula charm = " << formula_charm << '\n'
        << "autodiff call vomma = " << call_price.derivative(0, 2, 0, 0) << '\n'
        << "autodiff put  vomma = " << put_price.derivative(0, 2, 0, 0) << '\n'
        << "      formula vomma = " << formula_vomma << '\n'
        << "autodiff call veta = " << call_price.derivative(0, 1, 1, 0) << '\n'
        << "autodiff put  veta = " << put_price.derivative(0, 1, 1, 0) << '\n'
        << "      formula veta = " << formula_veta << '\n'
        << "\n## Third-order Greeks\n"
        << "autodiff call speed = " << call_price.derivative(3, 0, 0, 0) << '\n'
        << "autodiff put  speed = " << put_price.derivative(3, 0, 0, 0) << '\n'
        << "      formula speed = " << formula_speed << '\n'
        << "autodiff call zomma = " << call_price.derivative(2, 1, 0, 0) << '\n'
        << "autodiff put  zomma = " << put_price.derivative(2, 1, 0, 0) << '\n'
        << "      formula zomma = " << formula_zomma << '\n'
        << "autodiff call color = " << call_price.derivative(2, 0, 1, 0) << '\n'
        << "autodiff put  color = " << put_price.derivative(2, 0, 1, 0) << '\n'
        << "      formula color = " << formula_color << '\n'
        << "autodiff call ultima = " << call_price.derivative(0, 3, 0, 0) << '\n'
        << "autodiff put  ultima = " << put_price.derivative(0, 3, 0, 0) << '\n'
        << "      formula ultima = " << formula_ultima << '\n'
        ;



    auto v = std::vector<double>{ 1.0, 2.0, 5.0, 10.0 };
    auto v2 = std::vector<double>{ 10.0, 20.0, 50.0, 100.0 };
    auto interp_1 = LinearInterpolator<double, double>(v, v2);
    std::cout << "interp function: " << interp_1.interp(2.5) << std::endl;
    std::cout << "interp function: " << interp_1.interp(12.5) << std::endl;
    std::cout << "interp function: " << interp_1.interp(0.5) << std::endl;
    Holiday us_holiday;
    date day1{ 2021, 5, 31 };
    std::cout << "is " << day1 << " a holiday: " << us_holiday.is_holiday(day1) << std::endl;
    date day2 = bus_day_shift(day1, 30, us_holiday);
    std::cout << "So after adj current date is " << day2  << std::endl;

    std::string aa = "1y";
    std::string bb = "6m";

    std::cout << "test function 1y: " << tenor_to_year(aa) << std::endl;
    std::cout << "test function 6m: " << tenor_to_year(bb) << std::endl;

    FixedCashFlow cf(1000000.00, 0.02, day1, day2, DCC::A30360);
    CashFlowSchedule cfs(1000000.0, 0.02, day1, 2, "10Y");
    cfs.show();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
