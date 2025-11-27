// -*- lsst-c++ -*-
/**
 * @file GeometryCache.cpp
 * @brief See GeometryCache.hpp. Precompute radial weights.
 */

#include "CompactStar/ChemicalHeating/GeometryCache.hpp"
#include "CompactStar/ChemicalHeating/StarContext.hpp"

#include <Zaki/Vector/DataSet.hpp>
#include <cmath>

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
//                   GeometryCache Class
//==============================================================
GeometryCache::GeometryCache(const StarContext &ctx)
{
	// Pull columns (non-owning) and copy to POD vectors for hot-loop access.
	const auto *rcol = ctx.Radius();
	const auto *ncol = ctx.Nu();
	const auto *lcol = ctx.ExpLambda(); // may be null in Phase 1

	const std::size_t N = ctx.Size();
	m_r.resize(N);
	m_wC.resize(N);
	m_wNu.resize(N);

	for (std::size_t i = 0; i < N; ++i)
	{
		const double r_km = (*rcol)[i];
		const double nu = (*ncol)[i];
		const double e_nu = std::exp(nu);

		// Temporary fallback if e^Lambda is not provided.
		const double e_L = (lcol) ? (*lcol)[i] : 1.0;

		// Weights (Phase 1; unit factors refined later)
		// wC  = 4π r^2 e^{Λ}                 (heat capacity integrals)
		// wNu = 4π r^2 e^{Λ + 2ν}            (luminosities at infinity)
		const double r2 = r_km * r_km;

		m_r[i] = r_km;
		m_wC[i] = 4.0 * M_PI * r2 * e_L;
		m_wNu[i] = 4.0 * M_PI * r2 * e_L * (e_nu * e_nu);
	}
}
//==============================================================
std::size_t GeometryCache::Size() const { return m_r.size(); }
const std::vector<double> &GeometryCache::WC() const { return m_wC; }
const std::vector<double> &GeometryCache::WNu() const { return m_wNu; }
const std::vector<double> &GeometryCache::Radius() const { return m_r; }
//==============================================================
} // namespace ChemicalHeating
} // namespace CompactStar