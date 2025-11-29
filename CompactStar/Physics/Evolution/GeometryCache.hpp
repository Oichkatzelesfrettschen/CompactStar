// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file GeometryCache.hpp
 * @brief Precomputed geometric weights and masks for fast radial integration.
 *
 * GeometryCache builds redshift/volume weights once per star:
 *  - \f$w_C(r)=4\pi r^2 e^{\Lambda(r)}\f$   (heat capacity integrals)
 *  - \f$w_\nu(r)=4\pi r^2 e^{\Lambda(r)+2\nu(r)}\f$ (luminosities at infinity)
 *
 * @ingroup Physics
 */
#ifndef CompactStar_Physics_Evolution_GeometryCache_H
#define CompactStar_Physics_Evolution_GeometryCache_H

#include <cstddef>
#include <vector>

// namespace CompactStar
// {
// namespace ChemicalHeating
// {
// class StarContext;
// }
// } // namespace CompactStar

//==============================================================
namespace CompactStar
{
namespace Physics
{
namespace Evolution
{

class StarContext;
//==============================================================
//                      GeometryCache Class
//==============================================================
/**
 * @class GeometryCache
 * @brief Stores radial weights and simple structure flags for integrals.
 */
class GeometryCache
{
  public:
	/**
	 * @brief Build weights from a @c StarContext.
	 * @param ctx Read-only star context (grid, metric).
	 */
	explicit GeometryCache(const StarContext &ctx);

	/** @brief Number of radial samples. */
	[[nodiscard]] std::size_t Size() const;

	/** @brief Weight for heat-capacity-like integrals: \f$w_C(r)\f$. */
	const std::vector<double> &WC() const;

	/** @brief Weight for redshifted luminosities: \f$w_\nu(r)\f$. */
	const std::vector<double> &WNu() const;

	/** @brief Radius grid (km). Provided for convenience. */
	const std::vector<double> &Radius() const;

  private:
	std::vector<double> m_r;
	std::vector<double> m_wC;
	std::vector<double> m_wNu;
};
//==============================================================
} // namespace Evolution
//==============================================================
} // namespace Physics
//==============================================================
} // namespace CompactStar
//==============================================================

#endif /* CompactStar_Physics_Evolution_GeometryCache_H */