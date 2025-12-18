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
		// throw std::runtime_error(
		// 	"ThermalState::UnpackFrom: called before Resize(). "
		// 	"State size is zero and cannot be unpacked.");

		Z_LOG_ERROR(
			"Called before Resize(). "
			"State size is zero and cannot be unpacked.");
		return;
	}

	for (std::size_t i = 0; i < n; ++i)
	{
		values_[i] = src[i];
	}
}
// ------------------------------------------------------------------
// Primary thermal DOF convention
// ------------------------------------------------------------------
double &ThermalState::LnTinfOverTref()
{
	if (values_.empty())
	{
		Z_LOG_ERROR("values_ is empty; call Resize(1) first.");
		static double dummy = 0.0;
		return dummy;
	}
	return values_.at(0);
}

//--------------------------------------------------------------
const double &ThermalState::LnTinfOverTref() const
{
	if (values_.empty())
	{
		Z_LOG_ERROR("values_ is empty; call Resize(1) first.");
		static double dummy = 0.0;
		return dummy;
	}
	return values_.at(0);
}

//--------------------------------------------------------------
double ThermalState::Tinf() const
{
	if (values_.empty())
	{
		Z_LOG_ERROR("values_ is empty; call Resize(1) first.");
		return 0.0;
	}

	return Tref_K() * std::exp(values_.at(0));
}

//--------------------------------------------------------------
void ThermalState::SetTinf(double T_K)
{
	if (!(T_K > 0.0))
	{
		Z_LOG_ERROR("T_K must be > 0.");
		return;
	}

	if (values_.empty())
	{
		Z_LOG_ERROR("values_ is empty; call Resize(1) first.");
		return;
	}

	values_.at(0) = std::log(T_K / Tref_K());
}

//--------------------------------------------------------------