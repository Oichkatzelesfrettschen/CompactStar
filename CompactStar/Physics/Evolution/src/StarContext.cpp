// -*- lsst-c++ -*-
/**
 * @file StarContext.cpp
 * @brief See StarContext.hpp. Thin adapters over NStar/EOS.
 */

#include "CompactStar/Physics/Evolution/StarContext.hpp"

#include "CompactStar/Core/NStar.hpp" // NStar, SeqPoint
// Concrete EOS base header
#include "CompactStar/EOS/Model.hpp"

#include <Zaki/Vector/DataSet.hpp>

namespace CompactStar::Physics
{
namespace Evolution
{
//==============================================================
//                   StarContext Class
//==============================================================
//--------------------------------------------------------------
StarContext::StarContext(const CompactStar::NStar &ns,
						 const CompactStar::EOS::Model *eos)
	: m_ns(&ns), m_eos(eos)
{
	// Cache commonly used columns (non-owning)
	// NStar API (from your header):
	//   Zaki::Vector::DataColumn* GetRadius();
	//   Zaki::Vector::DataColumn* GetNu();
	//   Zaki::Vector::DataColumn* GetMass();
	//   Zaki::Vector::DataColumn* GetRho();
	// Some models may expose species columns via GetRho_i(label)

	// NOTE: NStar getters return non-const pointers; we store as const.
	m_r = const_cast<CompactStar::NStar &>(ns).Profile().GetRadius();
	m_nu = const_cast<CompactStar::NStar &>(ns).Profile().GetMetricNu();
	m_m = const_cast<CompactStar::NStar &>(ns).Profile().GetMass();
	m_nb = const_cast<CompactStar::NStar &>(ns).Profile().GetBaryonDensity();

	// e^{Lambda} is not explicitly exposed in NStar.hpp.
	// We'll leave m_elam = nullptr for now; GeometryCache can derive or stub it.
	m_elam = nullptr;

	// Proton fraction may be available either from EOS or as a species ratio; leave null by default.
	m_yp = nullptr;
}

//--------------------------------------------------------------
std::size_t StarContext::Size() const
{
	// Prefer to infer size from the radius column if present.
	if (m_r)
	{
		// Assuming DataColumn provides Size().
		return m_r->Size();
	}
	return 0;
}
//--------------------------------------------------------------

const Zaki::Vector::DataColumn *StarContext::Radius() const { return m_r; }
const Zaki::Vector::DataColumn *StarContext::Nu() const { return m_nu; }
const Zaki::Vector::DataColumn *StarContext::ExpLambda() const { return m_elam; }
const Zaki::Vector::DataColumn *StarContext::Mass() const { return m_m; }
const Zaki::Vector::DataColumn *StarContext::BaryonDensity() const { return m_nb; }
const Zaki::Vector::DataColumn *StarContext::ProtonFraction() const { return m_yp; }

//--------------------------------------------------------------
double StarContext::RadiusSurface() const
{
	// NStar exposes the sequence point; use its R (km).
	return m_ns ? m_ns->GetSequence().r : 0.0;
}

//--------------------------------------------------------------
double StarContext::MassSurface() const
{
	// Gravitational mass from sequence point (units per your core).
	return m_ns ? m_ns->GetSequence().m : 0.0;
}

//--------------------------------------------------------------
double StarContext::ExpNuSurface() const
{
	if (!m_ns)
		return 1.0;
	// const double R = m_ns->GetSequence().r;
	// GetNu returns ν; we need e^{ν} at R.
	const double nu_R = m_ns->Profile().GetMetricNu()->operator[](-1);
	return std::exp(nu_R);
}
//--------------------------------------------------------------

} // namespace Evolution
} // namespace CompactStar::Physics