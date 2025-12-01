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
 * @file StatePacking.hpp
 * @brief Helpers for packing/unpacking State blocks to/from flat ODE arrays.
 *
 * These free functions provide the glue between:
 *   - the tagged State blocks stored in StateVector,
 *   - the layout information in StateLayout, and
 *   - the flat y[] / dydt[] arrays used by GSL.
 *
 * They rely on the virtual PackTo / UnpackFrom methods provided by the
 * abstract State base class and implemented by each concrete State block
 * (SpinState, ThermalState, ChemState, BNVState, ...).
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_StatePacking_H
#define CompactStar_Physics_Evolution_StatePacking_H

#include <cstddef>

#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StateLayout.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"

namespace CompactStar::Physics::Evolution
{

/**
 * @brief Pack all active State blocks into the flat ODE vector y[].
 *
 * For each active tag in @p layout, this:
 *   1. obtains the corresponding State& from @p state,
 *   2. determines its block size from @p layout,
 *   3. calls State::PackTo() to write into y[offset..offset+size).
 *
 * @param state   Composite view over all registered State blocks.
 * @param layout  Layout describing offsets/sizes for each tag in y[].
 * @param y       Output pointer to the first element of the flat ODE vector.
 *                Must have capacity at least layout.TotalSize().
 */
void PackStateVector(const StateVector &state,
					 const StateLayout &layout,
					 double *y);

/**
 * @brief Unpack the flat ODE vector y[] back into the State blocks.
 *
 * For each active tag in @p layout, this:
 *   1. obtains the corresponding State& from @p state,
 *   2. determines its block size from @p layout,
 *   3. calls State::UnpackFrom() on the slice y[offset..offset+size).
 *
 * @param state   Composite view over all registered State blocks.
 * @param layout  Layout describing offsets/sizes for each tag in y[].
 * @param y       Input pointer to the first element of the flat ODE vector.
 *                Must have length at least layout.TotalSize().
 */
void UnpackStateVector(StateVector &state,
					   const StateLayout &layout,
					   const double *y);

/**
 * @brief Scatter contributions from RHSAccumulator into dydt[].
 *
 * After drivers have written dY/dt contributions into @p rhs (one block
 * per StateTag), this function copies those blocks into the flat RHS
 * vector dydt[] using the offsets/sizes stored in @p layout.
 *
 * Only tags that are active in @p layout and configured in @p rhs are
 * copied. For safety, a mismatch between block sizes will throw.
 *
 * @param rhs      RHSAccumulator containing per-tag dY/dt blocks.
 * @param layout   Layout describing offsets/sizes for each tag in dydt[].
 * @param dydt     Output pointer to the first element of the flat RHS vector.
 *                 Must have capacity at least layout.TotalSize().
 */
void ScatterRHSFromAccumulator(const RHSAccumulator &rhs,
							   const StateLayout &layout,
							   double *dydt);

} // namespace CompactStar::Physics::Evolution

#endif /* CompactStar_Physics_Evolution_StatePacking_H */