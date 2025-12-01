#include "CompactStar/Physics/State/ThermalState.hpp"
#include <stdexcept>

using CompactStar::Physics::State::ThermalState;

//--------------------------------------------------------------
// PackTo
//--------------------------------------------------------------
void ThermalState::PackTo(double *dest) const
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
void ThermalState::UnpackFrom(const double *src)
{
	const std::size_t n = values_.size();
	if (n == 0)
	{
		throw std::runtime_error(
			"ThermalState::UnpackFrom: called before Resize(). "
			"State size is zero and cannot be unpacked.");
	}

	for (std::size_t i = 0; i < n; ++i)
	{
		values_[i] = src[i];
	}
}
//--------------------------------------------------------------