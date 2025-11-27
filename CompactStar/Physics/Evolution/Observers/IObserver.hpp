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
 * @file Observers.hpp
 * @brief Observer interfaces and a basic recorder for time-series outputs.
 *
 * Observers receive snapshots during integration and write to your existing
 * Zaki::Vector::DataSet columns (t, T^∞, T_s, L_ν^∞, L_γ^∞, H^∞, η_i, …).
 *
 * @ingroup ChemicalHeating
 */
#ifndef CompactStar_ChemicalHeating_Observers_H
#define CompactStar_ChemicalHeating_Observers_H

#include <string>
#include <vector>

namespace Zaki
{
namespace Vector
{
class DataSet;
}
} // namespace Zaki

namespace CompactStar
{
namespace ChemicalHeating
{
struct EvolutionState;
}
} // namespace CompactStar
//==============================================================
namespace CompactStar
{
//==============================================================
namespace ChemicalHeating
{

//==============================================================
//                       Derived Struct
//==============================================================
/**
 * @struct Derived
 * @brief Derived observables computed at each save point.
 *
 * This carries quantities not stored directly in @c EvolutionState but useful
 * for diagnostics and plotting (e.g., luminosities and heat capacity).
 */
struct Derived
{
	double Ts_local = 0.0;	 /*!< Surface temperature (K), local frame.          */
	double Lnu_inf = 0.0;	 /*!< Neutrino luminosity at infinity [erg s^-1].    */
	double Lgamma_inf = 0.0; /*!< Photon luminosity at infinity [erg s^-1].      */
	double H_inf = 0.0;		 /*!< Total internal heating at infinity [erg s^-1]. */
	double C_tot = 0.0;		 /*!< Total heat capacity [erg K^-1].                */
};
//==============================================================

//==============================================================
//                        Observer Class
//==============================================================
/**
 * @class Observer
 * @brief Callback interface invoked by the integrator.
 */
class Observer
{
  public:
	virtual ~Observer() {}

	/**
	 * @brief Called whenever the integrator saves a sample (every dt_save).
	 * @param state Current ODE state (t, T^∞, η, …).
	 * @param d     Derived diagnostics and luminosities.
	 */
	virtual void on_sample(const EvolutionState &state, const Derived &d) = 0;

	/**
	 * @brief Called once the integration finishes (success or failure).
	 * @param state Final state.
	 * @param ok    True if reached the target time.
	 */
	virtual void on_finish(const EvolutionState &state, bool ok)
	{
		(void)state;
		(void)ok;
	}
};
//==============================================================

//==============================================================
//                  TimeSeriesRecorder Class
//==============================================================
/**
 * @class TimeSeriesRecorder
 * @brief Writes samples to a @c Zaki::Vector::DataSet as columns.
 *
 * Column layout (created on first use):
 *  t[s], Tinf[K], Ts[K], Lnu_inf[erg/s], Lgamma_inf[erg/s], H_inf[erg/s],
 *  C_tot[erg/K], eta_0, eta_1, ...
 */
class TimeSeriesRecorder : public Observer
{
  public:
	/**
	 * @brief Construct with a target dataset.
	 * @param ds   Dataset to receive appended rows.
	 * @param tag  Optional run label to embed in metadata.
	 */
	explicit TimeSeriesRecorder(Zaki::Vector::DataSet &ds,
								const std::string &tag = std::string());

	/// @copydoc Observer::on_sample
	void on_sample(const EvolutionState &state, const Derived &d) override;

	/// @copydoc Observer::on_finish
	void on_finish(const EvolutionState &state, bool ok) override;

  private:
	Zaki::Vector::DataSet *m_ds = 0;
	std::string m_tag;
	bool m_initialized = false;
};
//==============================================================
} // namespace ChemicalHeating
//==============================================================
} // namespace CompactStar
//==============================================================

#endif /* CompactStar_ChemicalHeating_Observers_H */