#pragma once
#pragma once
#include <vector>

template <typename Tx, typename Ty>
class Interpolator {

protected:
	std::vector<Tx> Xs;
	std::vector<Ty> Ys;

public:
	Interpolator(std::vector<Tx> xx, std::vector<Ty> yy) : Xs{ xx }, Ys{ yy }{
		if (Xs.size() != Ys.size()) {
			throw std::invalid_argument("received diff size x and y");
		}
	}

	virtual Ty interp(const Tx& x_star)
	{
		std::cout << "virtual interp called \n";
		return Ys[0];
	}
};


template <typename Tx, typename Ty>
class LinearInterpolator : public Interpolator<Tx, Ty> {

public:
	LinearInterpolator(std::vector<Tx> xx, std::vector<Ty> yy) : Interpolator<Tx, Ty>(xx, yy) {}

	Ty interp(const Tx& x_star) {

		auto x2_it = std::upper_bound(this->Xs.begin(), this->Xs.end(), x_star);
		if (x2_it == this->Xs.begin())
		{
			// left extrapolation
			return this->Ys[0];
		}
		else if (x2_it == this->Xs.end() && x_star > this->Xs.back())
		{
			// right extrapolation
			return this->Ys.back();
		}
		else
		{
			int ind_2 = x2_it - this->Xs.begin();
			int ind_1 = ind_2 - 1;

			return (x_star - this->Xs[ind_1]) / (this->Xs[ind_2] - this->Xs[ind_1]) * (this->Ys[ind_2] - this->Ys[ind_1]) + this->Ys[ind_1];
		}

	}

};