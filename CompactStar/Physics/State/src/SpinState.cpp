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

#include "CompactStar/Physics/State/SpinState.hpp"
#include <stdexcept>

using CompactStar::Physics::State::SpinState;

//--------------------------------------------------------------
// PackTo
//--------------------------------------------------------------
void SpinState::PackTo(double *dest) const
{
	// Copy the internal contiguous DOF vector into dest.
	// Assumes dest has space for at least Size() entries.
	const std::size_t n = values_.size();
	for (std::size_t i = 0; i < n; ++i)
	{
		dest[i] = values_[i];
	}
}

//--------------------------------------------------------------
// UnpackFrom
//--------------------------------------------------------------
void SpinState::UnpackFrom(const double *src)
{
	const std::size_t n = values_.size();
	if (n == 0)
	{
		throw std::runtime_error(
			"SpinState::UnpackFrom: called before Resize(). "
			"State size is zero and cannot be unpacked.");
	}

	for (std::size_t i = 0; i < n; ++i)
	{
		values_[i] = src[i];
	}
}
//--------------------------------------------------------------