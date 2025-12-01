// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025
 * Mohammadreza Zakeri
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
 * @file SpinState.hpp
 * @brief Spin/kinematic state block + observables for a compact star.
 *
 * This class serves two closely related roles:
 *  - provides a **dynamic state block** for spin evolution (e.g. Ω),
 *    exposed to the ODE integrator via the State base interface;
 *  - stores **observational spin/kinematic quantities** (P, Pdot, μ, d)
 *    as Zaki::Math::Quantity for use in BNV bounds, torque models, etc.
 *
 * Directory-based namespace: CompactStar::Physics::State
 *
 * @ingroup PhysicsState
 *
 * @author
 *   Mohammadreza Zakeri (M.Zakeri@eku.edu)
 */

#ifndef CompactStar_Physics_State_SpinState_H
#define CompactStar_Physics_State_SpinState_H

#include <cstddef>
#include <vector>

#include <Zaki/Math/Math_Core.hpp>

#include "CompactStar/Physics/State/State.hpp"

namespace CompactStar::Physics::State
{

/**
 * @class SpinState
 * @brief Spin evolution state + observational spin/kinematic parameters.
 *
 * ### Dynamic DOFs
 * The internal `values_` vector represents the **evolved spin degrees of
 * freedom** in a contiguous layout. A typical minimal configuration is
 * a single component interpreted as the spin frequency Ω [rad/s].
 *
 * You can extend this to multiple components (e.g. Ω, \dot{Ω}, or
 * additional spin-related variables) without touching the evolution core.
 *
 * ### Observational fields
 * The `P`, `Pdot`, `mu`, and `d` members store measured or inferred
 * pulsar parameters:
 *  - P      — period [s]
 *  - Pdot   — period derivative [s/s]
 *  - mu     — proper motion [mas/yr]
 *  - d      — distance from SSB [kpc]
 *
 * These are **not** part of the ODE state vector; they are side data
 * used by BNV utilities, torque prescriptions, etc.
 *
 * @note Storage (the `values_` vector) deliberately lives in this derived
 *       class rather than in the State base class.  This design allows
 *       future state types to use non-contiguous, multi-zone, or
 *       externally owned layouts while still exposing the uniform
 *       PackTo/UnpackFrom interface required by the evolution framework.
 *       See @ref CompactStar::Physics::State::State for details.
 */
class SpinState : public State
{
  public:
	// ------------------------------------------------------------------
	// Required State interface
	// ------------------------------------------------------------------

	/// Short human-readable identifier.
	const char *Name() const override { return "SpinState"; }

	/// Number of scalar spin DOFs in the dynamic block.
	std::size_t Size() const override { return values_.size(); }

	/// Mutable pointer to the first dynamic DOF (contiguous).
	double *Data() override
	{
		return values_.empty() ? nullptr : values_.data();
	}

	/// Const pointer to the first dynamic DOF (contiguous).
	const double *Data() const override
	{
		return values_.empty() ? nullptr : values_.data();
	}

	/**
	 * @brief Resize the dynamic spin DOF vector to @p N entries.
	 *
	 * Typical usage:
	 *  - N = 1 → single DOF interpreted as Ω (spin frequency) [rad/s].
	 *
	 * Contents are set to zero on resize. Must be called before integration.
	 */
	void Resize(std::size_t N) override
	{
		values_.assign(N, 0.0);
	}

	/// For this block, "grid size" coincides with number of DOFs.
	std::size_t GridSize() const override { return values_.size(); }

	/// Zero out all dynamic spin DOFs (capacity preserved).
	void Clear() override
	{
		for (double &x : values_)
			x = 0.0;
	}

	// ------------------------------------------------------------------
	// packing / unpacking
	// ------------------------------------------------------------------

	/**
	 * @brief Pack the dynamic spin DOFs into a contiguous buffer.
	 *
	 * The buffer @p dest must have space for at least Size() doubles.
	 * This simply copies the internal values_ vector into dest[0..Size()-1].
	 */
	void PackTo(double *dest) const override;

	/**
	 * @brief Unpack the dynamic spin DOFs from a contiguous buffer.
	 *
	 * The buffer @p src must contain at least Size() doubles. The internal
	 * values_ vector is resized to Size() if necessary and filled from src.
	 */
	void UnpackFrom(const double *src) override;

	// ------------------------------------------------------------------
	// Convenience accessors for dynamic DOFs
	// ------------------------------------------------------------------

	/// Number of dynamic spin components.
	std::size_t NumComponents() const { return values_.size(); }

	/// Mutable access to i-th dynamic component.
	double &Value(std::size_t i) { return values_.at(i); }

	/// Const access to i-th dynamic component.
	const double &Value(std::size_t i) const { return values_.at(i); }

	/**
	 * @brief Convenience accessor for the primary spin DOF.
	 *
	 * By convention, when NumComponents() >= 1, Value(0) is interpreted
	 * as the spin frequency Ω [rad/s].
	 */
	double &Omega() { return values_.at(0); }

	/// Const version of Omega().
	const double &Omega() const { return values_.at(0); }

	// ------------------------------------------------------------------
	// Observational / kinematic quantities (not part of Data())
	// ------------------------------------------------------------------

	/// Spin period \(P\) [s] (observed/inferred).
	Zaki::Math::Quantity P;

	/// Period derivative \(\dot{P}\) [s/s] (observed/inferred).
	Zaki::Math::Quantity Pdot;

	/// Proper motion \(\mu\) [mas/yr].
	Zaki::Math::Quantity mu;

	/// Distance from the Solar System barycenter \(d\) [kpc].
	Zaki::Math::Quantity d;

  private:
	/// Contiguous block of dynamic spin DOFs (e.g. Ω, Ω̇, ...).
	std::vector<double> values_;
};

} // namespace CompactStar::Physics::State

#endif /* CompactStar_Physics_State_SpinState_H */