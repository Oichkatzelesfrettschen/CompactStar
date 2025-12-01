#include "CompactStar/Physics/State/ChemState.hpp"
#include <stdexcept>

using CompactStar::Physics::State::ChemState;

//--------------------------------------------------------------
// PackTo
//--------------------------------------------------------------
void ChemState::PackTo(double *dest) const
{
	const std::size_t n = eta_.size();
	for (std::size_t i = 0; i < n; ++i)
	{
		dest[i] = eta_[i];
	}
}

//--------------------------------------------------------------
// UnpackFrom
//--------------------------------------------------------------
void ChemState::UnpackFrom(const double *src)
{
	const std::size_t n = eta_.size();
	if (n == 0)
	{
		throw std::runtime_error(
			"ChemState::UnpackFrom: called before Resize(). "
			"State size is zero and cannot be unpacked.");
	}

	for (std::size_t i = 0; i < n; ++i)
	{
		eta_[i] = src[i];
	}
}
//--------------------------------------------------------------