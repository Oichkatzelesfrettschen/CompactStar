// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025
 * Mohammadreza Zakeri
 *
 * MIT License — see LICENSE at repo root.
 */
/**
 * @defgroup PhysicsState Physics state module
 * @brief Physical states of the star (thermal, spin, chem, BNV).
 */
/**
 * @file State.hpp
 * @brief Abstract ODE state interface for physics sub-states.
 *
 * This file defines:
 *   - StepMeta: metadata describing an attempted integration step.
 *   - State   : base class for all physics state blocks contributing
 *               dynamic DOFs to the global ODE vector.
 *
 * Directory-based namespace:
 *      CompactStar::Physics::State
 *
 * Subclasses:
 *      ChemState, ThermalState, SpinState, BNVState, ...
 *
 * These define the physical variables involved in neutron-star evolution.
 * The Evolution system flattens and integrates these states, but does not
 * define or own their internal structure.
 *
 * @ingroup PhysicsState
 */

#ifndef CompactStar_Physics_State_State_H
#define CompactStar_Physics_State_State_H

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include <Zaki/Util/Instrumentor.hpp> // Z_LOG_*, PROFILE_FUNCTION
#include <Zaki/Vector/DataSet.hpp>	  // For ExportColumns()

namespace CompactStar::Physics::State
{

//======================================================================
//                             StepMeta
//======================================================================

/**
 * @struct StepMeta
 * @brief Diagnostic metadata for a single attempted integration step.
 *
 * Emitted to observers and loggers. Does *not* modify the physical state.
 */
struct StepMeta
{
	/// Coordinate time (s) measured at infinity.
	double t = 0.0;

	/// Step size attempted by the integrator.
	double dt = 0.0;

	/// Monotone step counter.
	long step = 0;
};

//======================================================================
//                               State
//======================================================================

/**
 * @class State
 * @brief Abstract base class for any dynamic physics state block.
 *
 * Requirements for subclasses:
 *   - Provide a contiguous block of doubles via Data().
 *   - Provide the number of DOFs via Size().
 *   - Implement Resize(N) before integration.
 *   - Implement Clear() to reinitialize safely.
 *
 * Optional:
 *   - Implement ExportColumns() for debug I/O.
 *   - Override SanityCheck() to enforce physical bounds (T>0, etc.).
 */
class State
{
  public:
	virtual ~State() = default;

	// ------------------------------------------------------------------
	// Mandatory interface
	// ------------------------------------------------------------------

	/// Short state name for logging (e.g. "ChemState").
	virtual const char *Name() const = 0;

	/// Number of scalar degrees of freedom.
	virtual std::size_t Size() const = 0;

	/// Mutable pointer to underlying contiguous memory.
	virtual double *Data() = 0;

	/// Const pointer to underlying contiguous memory.
	virtual const double *Data() const = 0;

	/**
	 * @brief Resize internal storage to N DOFs.
	 *
	 * Must be called BEFORE integration.
	 * After integration begins, changing size is undefined behavior.
	 */
	virtual void Resize(std::size_t N) = 0;

	/**
	 * @brief Number of spatial grid cells represented.
	 *
	 * Many states map 1 DOF per cell; override for multi-DOF-per-cell.
	 */
	virtual std::size_t GridSize() const { return Size(); }

	/// Reset contents to a well-defined state (usually all zeros).
	virtual void Clear() = 0;

	// ------------------------------------------------------------------
	// Diagnostics
	// ------------------------------------------------------------------

	/**
	 * @brief Check for NaNs/Inf in the state vector and log results.
	 */
	virtual void SanityCheck() const
	{
		PROFILE_FUNCTION();

		const std::size_t n = Size();
		const double *y = Data();

		if (!y || n == 0)
		{
			Z_LOG_WARNING(std::string("SanityCheck: empty state in ") + Name());
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
			Z_LOG_ERROR(std::string("SanityCheck[") + Name() + "]: NaN detected");
		if (has_inf)
			Z_LOG_ERROR(std::string("SanityCheck[") + Name() + "]: Inf detected");
		if (!has_nan && !has_inf)
			Z_LOG_INFO(std::string("SanityCheck[") + Name() + "]: OK (" +
					   std::to_string(n) + " DOFs)");
	}

	// ------------------------------------------------------------------
	// Optional I/O
	// ------------------------------------------------------------------

	/**
	 * @brief Append columns to a DataSet for debug or file output.
	 *
	 * @param ds     Target dataset.
	 * @param prefix Column prefix (e.g. "chem.", "therm.").
	 */
	virtual void ExportColumns(Zaki::Vector::DataSet &ds,
							   const std::string &prefix) const
	{
		(void)ds;
		(void)prefix;
		// Base implementation: no-op.
	}
};

} // namespace CompactStar::Physics::State

#endif // CompactStar_Physics_State_State_H