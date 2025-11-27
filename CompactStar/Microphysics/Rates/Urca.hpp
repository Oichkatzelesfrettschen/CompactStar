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
 * @file RateChannels.hpp
 * @brief Abstract interfaces for local reaction/emissivity channels and their set.
 *
 * Each channel provides contributions to:
 *  - neutrino emissivity \f$Q_\nu(r, T, \boldsymbol{\eta})\f$
 *  - internal heating \f$H(r, T, \boldsymbol{\eta})\f$
 *  - chemical-imbalance relaxation (accumulating into \f$\dot{\boldsymbol{\eta}}\f$)
 *
 * @ingroup ChemicalHeating
 */
#ifndef CompactStar_ChemicalHeating_RateChannels_H
#define CompactStar_ChemicalHeating_RateChannels_H

#include <cstddef>
#include <memory>
#include <vector>

namespace CompactStar
{
namespace ChemicalHeating
{
class GeometryCache;
class StarContext;
} // namespace ChemicalHeating
} // namespace CompactStar

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
//                      ReactionChannel Class
//==============================================================
/**
 * @class ReactionChannel
 * @brief Strategy interface for a single local microphysical process.
 */
class ReactionChannel
{
  public:
	virtual ~ReactionChannel() {}

	/**
	 * @brief Local neutrino emissivity \f$Q_\nu\f$ [erg cm^{-3} s^{-1}].
	 * @param i   Radial zone index.
	 * @param T   Local temperature (K).
	 * @param eta Chemical imbalances vector (size from Config::n_eta).
	 * @return Emissivity in cgs (or module-consistent) units.
	 */
	virtual double emissivity(std::size_t i, double T,
							  const std::vector<double> &eta) const = 0;

	/**
	 * @brief Local dissipative heating \f$H\f$ [erg cm^{-3} s^{-1}].
	 * @details Includes e.g. rotochemical/BVN dissipation mapped to heat.
	 */
	virtual double heating(std::size_t i, double T,
						   const std::vector<double> &eta) const = 0;

	/**
	 * @brief Accumulate local contributions to \f$\dot{\boldsymbol{\eta}}\f$.
	 * @param i        Radial zone index.
	 * @param T        Local temperature (K).
	 * @param eta      Chemical imbalances.
	 * @param ddt_eta  In/out accumulator (same size as @p eta).
	 */
	virtual void accumulate_deta_dt(std::size_t i, double T,
									const std::vector<double> &eta,
									std::vector<double> &ddt_eta) const = 0;
};

//==============================================================
//                      RateSet Class
//==============================================================
/**
 * @class RateSet
 * @brief Container of channels; no ownership of geometry or state.
 *
 * Implementation (Phase 1+) will provide helpers that loop channels and
 * (optionally) perform shell integrations with GeometryCache.
 */
class RateSet
{
  public:
	/** @brief Add a channel (takes ownership). */
	void add(std::unique_ptr<ReactionChannel> ch);

	/** @brief Access channels (const). */
	const std::vector<std::unique_ptr<ReactionChannel>> &channels() const;

  private:
	std::vector<std::unique_ptr<ReactionChannel>> m_channels;
};

} // namespace ChemicalHeating
//==============================================================
} // namespace CompactStar
//==============================================================
#endif /* CompactStar_ChemicalHeating_RateChannels_H */