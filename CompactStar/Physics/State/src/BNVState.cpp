#include "CompactStar/Physics/State/BNVState.hpp"
#include <stdexcept>

using CompactStar::Physics::State::BNVState;

//--------------------------------------------------------------
// PackTo
//--------------------------------------------------------------
void BNVState::PackTo(double *dest) const
{
	const std::size_t n = values_.size();
	for (std::size_t i = 0; i < n; ++i)
	{
		dest[i] = values_[i];
	}
}

//--------------------------------------------------------------
// UnpackFrom
//--------------------------------------------------------------
void BNVState::UnpackFrom(const double *src)
{
	const std::size_t n = values_.size();
	if (n == 0)
	{
		throw std::runtime_error(
			"BNVState::UnpackFrom: called before Resize(). "
			"State size is zero and cannot be unpacked.");
	}

	for (std::size_t i = 0; i < n; ++i)
	{
		values_[i] = src[i];
	}
}
//--------------------------------------------------------------