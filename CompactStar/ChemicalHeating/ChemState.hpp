// // -*- lsst-c++ -*-
// /*
//  * CompactStar
//  * See License file at the top of the source tree.
//  *
//  * Copyright (c) 2025 Mohammadreza Zakeri
//  *
//  * Permission is hereby granted, free of charge, to any person obtaining a copy
//  * of this software and associated documentation files (the "Software"), to deal
//  * in the Software without restriction, including without limitation the rights
//  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  * copies of the Software, and to permit persons to whom the Software is
//  * furnished to do so, subject to the following conditions:
//  *
//  * The above copyright notice and this permission notice shall be included in all
//  * copies or substantial portions of the Software.
//  *
//  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  * SOFTWARE.
//  */

// /**
//  * @file EvolutionState.hpp
//  * @brief Time-evolution state vector for chemical/thermal evolution.
//  *
//  * Holds the minimal ODE state: redshifted internal temperature T^∞, an array of
//  * chemical imbalances \f$\eta_i\f$ (e.g., \f$\eta_{npe}, \eta_{np\mu}\f$), and
//  * optionally the spin frequency \f$\Omega\f$ if coupled.
//  *
//  * All temperatures here are **redshifted** unless documented otherwise.
//  *
//  * @ingroup ChemicalHeating
//  * @author
//  *  Mohammadreza Zakeri (M.Zakeri@eku.edu)
//  */
// #ifndef CompactStar_ChemicalHeating_EvolutionState_H
// #define CompactStar_ChemicalHeating_EvolutionState_H

// #include <string>
// #include <vector>

// //==============================================================
// namespace CompactStar
// {
// //==============================================================
// namespace ChemicalHeating
// {

// //==============================================================
// //                      EvolutionState Struct
// //==============================================================
// /**
//  * @struct EvolutionState
//  * @brief ODE state and metadata carried between integration steps.
//  */
// struct EvolutionState
// {
// 	/** @brief Coordinate time (s) measured by a distant observer. */
// 	double t = 0.0;

// 	/** @brief Redshifted (isothermal-core) temperature \f$T^\infty\f$ in K. */
// 	double Tinf = 0.0;

// 	/**
// 	 * @brief Chemical imbalance vector \f$\boldsymbol{\eta}\f$ in energy units (e.g., erg).
// 	 *
// 	 * Ordering and meaning are configured in Config; typical entries include
// 	 * \f$\eta_{npe}\f$ and \f$\eta_{np\mu}\f$.
// 	 */
// 	std::vector<double> eta;

// 	/**
// 	 * @brief Spin frequency \f$\Omega\f$ in rad/s.
// 	 * Optional—used only if the evolution is coupled to a torque law.
// 	 */
// 	double Omega = 0.0;

// 	/** @brief Step counter (monotone, for diagnostics). */
// 	unsigned long step = 0;

// 	/** @brief Free-form label for provenance or run ID. */
// 	std::string tag;
// };
// //==============================================================
// } // namespace ChemicalHeating
// //==============================================================
// } // namespace CompactStar
// //==============================================================

// #endif /* CompactStar_ChemicalHeating_EvolutionState_H */

// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * MIT License (see LICENSE at repo root)
 */

/**
 * @file State.hpp
 * @brief Abstract ODE state interface + small diagnostics for evolution modules.
 *
 * This header defines:
 *  - StepMeta: lightweight metadata for integration steps.
 *  - State   : minimal, physics-agnostic interface exposing a contiguous
 *              block of doubles for the integrator (GSL, etc.).
 *
 * Design rules:
 *  - **No physics here.** State is just storage + shape. Microphysics lives in Models.
 *  - **Contiguous memory.** Integrators expect y as a flat double*.
 *  - **No hidden resizes during stepping.** Size is fixed before integration.
 *  - **Logging-first diagnostics.** Sanity helpers emit Z_LOG_* instead of throwing by default.
 *
 * Typical usage:
 *  - ChemicalState, ThermalState, SpinState each derive from State and implement
 *    storage layout and accessors. Evolution classes assemble RHS using Models + Drivers.
 *
 * @ingroup EvolutionCore
 * @author
 *  Mohammadreza Zakeri (M.Zakeri@eku.edu)
 */
#ifndef COMPACTSTAR_EVOLUTION_STATE_HPP
#define COMPACTSTAR_EVOLUTION_STATE_HPP

#include <cmath>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

#include <Zaki/Util/Instrumentor.hpp> // Z_LOG_INFO/WARNING/ERROR, PROFILE_FUNCTION
#include <Zaki/Vector/DataSet.hpp>	  // Forward-compatible with your DataSet columns

namespace CompactStar
{

/**
 * @brief Metadata about the current integration step.
 *
 * Used by observers/hook points; does not affect the ODE state itself.
 */
struct StepMeta
{
	/** Coordinate time (s) measured at infinity. */
	double t = 0.0;
	/** Attempted step size (s). */
	double dt = 0.0;
	/** Monotone step counter. */
	long step = 0;
};

/**
 * @brief Abstract base for all ODE “states” (chemical, thermal, spin, …).
 *
 * Concrete subclasses must:
 *  - provide a **contiguous** storage exposed via Data()/Size(),
 *  - implement fixed-size setup via Resize(N) (where N is usually the radial grid size),
 *  - implement NaN/range checks in SanityCheck().
 *
 * Optional (but recommended):
 *  - ExportColumns(): append labeled columns to a DataSet for debug/IO.
 */
class State
{
  public:
	virtual ~State() = default;

	/// Short human-readable identifier (e.g., "ChemicalState").
	virtual const char *Name() const = 0;

	/// Number of scalar degrees of freedom (the length of the flat y-vector).
	virtual std::size_t Size() const = 0;

	/// Pointer to the first element of the contiguous state vector (mutable).
	virtual double *Data() = 0;

	/// Pointer to the first element of the contiguous state vector (const).
	virtual const double *Data() const = 0;

	/**
	 * @brief Resize the state to match a spatial grid of length @p N.
	 *
	 * For multi-DOF-per-cell layouts, implement your own mapping but keep
	 * the *contiguous* invariant for Data()/Size().
	 *
	 * Must be called **before** integration begins. Resizing during stepping is undefined.
	 */
	virtual void Resize(std::size_t N) = 0;

	/**
	 * @brief Number of spatial cells represented (if applicable).
	 *
	 * Default returns Size(); override when you pack multiple DOFs per cell.
	 */
	virtual std::size_t GridSize() const { return Size(); }

	/**
	 * @brief Clear contents to a well-defined state (usually zeros).
	 *
	 * Implementations should not deallocate; keep capacity to avoid churn.
	 */
	virtual void Clear() = 0;

	/**
	 * @brief Lightweight diagnostics. Emit Z_LOG_* for anomalies.
	 *
	 * Default implementation checks for NaNs/inf and logs a warning. Override
	 * to add range/unit checks (e.g., T>0, |η|<η_max, etc.).
	 */
	virtual void SanityCheck() const
	{
		PROFILE_FUNCTION();
		const std::size_t n = Size();
		const double *y = Data();
		if (!y || n == 0)
		{
			Z_LOG_WARNING(std::string("State::SanityCheck: empty state in ") + Name());
			return;
		}

		bool has_nan = false, has_inf = false;
		for (std::size_t i = 0; i < n; ++i)
		{
			if (std::isnan(y[i]))
			{
				has_nan = true;
				break;
			}
			if (!std::isfinite(y[i]))
			{
				has_inf = true;
				break;
			}
		}
		if (has_nan)
			Z_LOG_ERROR(std::string("SanityCheck[") + Name() + "]: NaN detected.");
		if (has_inf)
			Z_LOG_ERROR(std::string("SanityCheck[") + Name() + "]: Inf detected.");
		if (!has_nan && !has_inf)
			Z_LOG_INFO(std::string("SanityCheck[") + Name() + "]: OK (n=" + std::to_string(n) + ").");
	}

	/**
	 * @brief Append state-owned columns to a DataSet for debugging/IO.
	 *
	 * @param ds      DataSet to append into.
	 * @param prefix  Column name prefix (e.g., "chem." or "therm.").
	 *
	 * The base implementation is a no-op. Concrete states should:
	 *  - create semantically labeled columns (e.g., "eta", "Tinf", "Omega"),
	 *  - ensure each appended column has **the same row count** as the radius grid,
	 *  - avoid exporting transient/internal buffers.
	 */
	virtual void ExportColumns(Zaki::Vector::DataSet &ds, const std::string &prefix) const
	{
		(void)ds;
		(void)prefix; // no-op in base
	}
};

} // namespace CompactStar

#endif // COMPACTSTAR_EVOLUTION_STATE_HPP