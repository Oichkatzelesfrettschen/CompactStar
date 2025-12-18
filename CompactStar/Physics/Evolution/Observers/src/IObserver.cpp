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

#include "CompactStar/Physics/Evolution/Observers/IObserver.hpp"

namespace CompactStar::Physics::Evolution::Observers
{

IObserver::~IObserver() = default;

void IObserver::OnStart(const RunInfo &run,
						const Evolution::StateVector &Y0,
						const Evolution::DriverContext &ctx)
{
	(void)run;
	(void)Y0;
	(void)ctx;
}

void IObserver::OnFinish(const FinishInfo &fin,
						 const Evolution::StateVector &Yf,
						 const Evolution::DriverContext &ctx)
{
	(void)fin;
	(void)Yf;
	(void)ctx;
}

std::string IObserver::Name() const
{
	return "IObserver";
}

} // namespace CompactStar::Physics::Evolution::Observers