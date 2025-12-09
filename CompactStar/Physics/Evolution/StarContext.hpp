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
 * @file StarContext.hpp
 * @brief Read-only bridge to Core structures (NStar, EOS, RotationSolver)
 * 		  for the evolution module.
 *
 * StarContext centralizes access to geometry, composition, and metric quantities
 * derived from a precomputed neutron-star profile. It *does not* run TOV or rotation;
 * it only exposes cached, interpolation-friendly views needed during RHS evaluations.
 *
 * @ingroup PhysicsEvolution
 */
#ifndef CompactStar_Physics_Evolution_StarContext_H
#define CompactStar_Physics_Evolution_StarContext_H

#include <cstddef>
#include <string>
#include <vector>

namespace Zaki
{
namespace Vector
{
class DataSet;
class DataColumn;
} // namespace Vector
} // namespace Zaki

namespace CompactStar::Core
{
class NStar;		  // Core/NStar.hpp
class RotationSolver; // Core/RotationSolver.hpp (if needed later)
} // namespace CompactStar::Core
namespace CompactStar::EOS
{
class Model;
} // namespace CompactStar::EOS

namespace CompactStar
{
namespace Physics
{
namespace Evolution
{

//==============================================================
//                      StarContext Class
//==============================================================
/**
 * @class StarContext
 * @brief Immutable, per-star adapter exposing cached geometry and composition.
 *
 * Constructed from an existing @c CompactStar::Core::NStar and (optionally) an EOS object.
 * Provides fast getters for radius grid, metric factors \f(e^{\nu}, e^{\Lambda}\f),
 * and commonly used columns (density, proton fraction, etc.) by forwarding to Core
 * datasets. Geometry-only computations that do not change during evolution can be
 * cached here at construction time (done in GeometryCache).
 */
class StarContext
{
  public:
	/// Default constructor (invalid context).
	StarContext() = default;

	/**
	 * @brief Construct from a precomputed neutron-star profile.
	 * @param ns   Reference to an initialized @c CompactStar::Core::NStar.
	 * @param eos  Optional pointer to an EOS object for composition queries (may be null).
	 */
	StarContext(const CompactStar::Core::NStar &ns,
				const CompactStar::EOS::Model *eos = 0);

	/// @name Basic geometry and grids
	///@{
	/** @brief Number of radial samples in the profile grid. */
	std::size_t Size() const;

	/** @brief Radius column (km). */
	const Zaki::Vector::DataColumn *Radius() const;

	/** @brief Metric exponent \f$\nu(r)\f$ where \f$g_{tt}=e^{2\nu}\f$. */
	const Zaki::Vector::DataColumn *Nu() const;

	/** @brief \f$\Lambda(r)\f$ if present (derived from mass and radius otherwise). */
	const Zaki::Vector::DataColumn *Lambda() const;

	/** @brief Gravitational mass profile (M_\f$\odot\f$ or internal units). */
	const Zaki::Vector::DataColumn *Mass() const;
	///@}

	/// @name Thermodynamic background (optional; forwarded from Core/EOS)
	///@{
	/** @brief Total baryon number density \f$n_b(r)\f$ (fm^{-3}) if available. */
	const Zaki::Vector::DataColumn *BaryonDensity() const;

	/** @brief Proton fraction \f$Y_p(r)\f$ or nullptr if not available. */
	const Zaki::Vector::DataColumn *ProtonFraction() const;
	///@}

	/// @name Global scalars
	///@{
	/** @brief Circumferential radius at the surface (km). */
	double RadiusSurface() const;

	/** @brief Gravitational mass at the surface (same units as Mass column). */
	double MassSurface() const;

	/** @brief Surface redshift factor \f$e^{\nu(R)}\f$. */
	double ExpNuSurface() const;
	///@}

  private:
	const CompactStar::Core::NStar *m_ns = 0;
	const CompactStar::EOS::Model *m_eos = 0;

	// Non-owning cached pointers to frequently accessed columns (set in ctor).
	const Zaki::Vector::DataColumn *m_r = 0;
	const Zaki::Vector::DataColumn *m_nu = 0;
	const Zaki::Vector::DataColumn *m_lam = 0;
	const Zaki::Vector::DataColumn *m_m = 0;
	const Zaki::Vector::DataColumn *m_nb = 0;
	const Zaki::Vector::DataColumn *m_yp = 0;
};
//==============================================================
} // namespace Evolution
//==============================================================
} // namespace Physics
//==============================================================
} // namespace CompactStar
//==============================================================
#endif /* CompactStar_Physics_Evolution_StarContext_H */