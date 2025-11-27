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
 * @file Config.hpp
 * @brief User-configurable options for chemical/thermal evolution runs.
 *
 * Includes integrator tolerances, enabled physics channels, envelope and gap
 * choices, output cadence, and initial conditions (sizes only; values live in EvolutionState).
 *
 * @ingroup ChemicalHeating
 */
#ifndef CompactStar_ChemicalHeating_Config_H
#define CompactStar_ChemicalHeating_Config_H

#include <cstddef>
#include <string>
#include <vector>

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
/**
 * @enum StepperType
 * @brief Available ODE steppers (backend is GSL in Phase 1+).
 */
enum class StepperType
{
	RKF45, /*!< Explicit Runge–Kutta–Fehlberg 4(5) — diagnostics/exploration. */
	MSBDF  /*!< Multi-step BDF — good default for stiffness at late times.   */
};

//==============================================================
/**
 * @enum EnvelopeModel
 * @brief Surface boundary models mapping \f$T_b \to T_s\f$.
 */
enum class EnvelopeModel
{
	Iron,
	Accreted,
	Custom /*!< Provide a custom callable in Phase 2+. */
};
//==============================================================

//==============================================================
/**
 * @struct Config
 * @brief Configures a single evolution run.
 */
struct Config
{
	// ---- Integrator ----------------------------------------------------------
	StepperType stepper = StepperType::MSBDF; /*!< Time stepper choice. */
	double rtol = 1e-6;						  /*!< Relative tolerance.  */
	double atol = 1e-10;					  /*!< Absolute tolerance.  */
	std::size_t max_steps = 1000000;		  /*!< Safety cap on steps. */

	// ---- Output --------------------------------------------------------------
	double dt_save = 1.0e2; /*!< Spacing of saved samples (s). */
	bool save_intermediate = true;

	// ---- Physics toggles -----------------------------------------------------
	bool use_isothermal_core = true;	 /*!< Assume isothermal interior (standard). */
	bool enable_MU = true;				 /*!< Modified Urca emissivity/reactions.    */
	bool enable_DU = true;				 /*!< Direct Urca emissivity/reactions.      */
	bool enable_PBF = false;			 /*!< Pair-breaking/formation emission.      */
	bool enable_BNV = false;			 /*!< Baryon-number violating processes.     */
	bool enable_rotochem_driver = false; /*!< Spin-down driver for \f$\eta\f$.   */
	bool couple_spin = false;			 /*!< Include \f$\Omega\f$ in the state.  */

	// ---- Envelope and gaps ---------------------------------------------------
	EnvelopeModel envelope = EnvelopeModel::Iron;
	double envelope_xi = 0.0; /*!< Light-element column parameter (if used). */
	bool superfluid_n = false;
	bool superfluid_p = false;

	// ---- Chemical imbalances -------------------------------------------------
	std::size_t n_eta = 1; /*!< Number of \f$\eta_i\f$ components. */

	// ---- Units policy (documentation only; conversions handled in impl) ------
	std::string unit_policy = "cgs_with_Gc1";

	// ---- Misc ----------------------------------------------------------------
	std::string run_label; /*!< Free-form label for outputs. */
};
//==============================================================

} // namespace ChemicalHeating
//==============================================================
} // namespace CompactStar
//==============================================================

#endif /* CompactStar_ChemicalHeating_Config_H */