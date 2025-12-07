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
 * @file GSLIntegrator.cpp
 * @brief Implementation of GSLIntegrator using gsl_odeiv2_driver.
 */

#include "CompactStar/Physics/Evolution/Integrator/GSLIntegrator.hpp"

#include <stdexcept>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>

#include "CompactStar/Physics/Evolution/EvolutionConfig.hpp"
#include "CompactStar/Physics/Evolution/EvolutionSystem.hpp"

namespace CompactStar
{
namespace Physics
{
namespace Evolution
{

//------------------------------------------------------------------
// Adapter: EvolutionSystem → gsl_odeiv2_system::function
//------------------------------------------------------------------
namespace
{

int gsl_rhs(double t, const double y[], double dydt[], void *params)
{
	auto *sys = static_cast<EvolutionSystem *>(params);
	if (!sys)
	{
		return GSL_EBADFUNC;
	}

	// EvolutionSystem::operator() already returns 0 on success,
	// non-zero on failure. GSL expects 0 for success.
	const int rc = sys->operator()(t, y, dydt);
	return (rc == 0) ? GSL_SUCCESS : GSL_EBADFUNC;
}

/**
 * @brief Map Config::stepper to a gsl_odeiv2_step_type.
 *
 * If the stepper is not recognized, defaults to gsl_odeiv2_step_rkf45.
 */
const gsl_odeiv2_step_type *SelectStepper(StepperType type)
{
	switch (type)
	{
	case StepperType::MSBDF:
		// Multistep BDF (stiff) – matches your default Config.
		return gsl_odeiv2_step_msbdf;

	case StepperType::RKF45:
		// Classic nonstiff Runge–Kutta–Fehlberg (4,5).
		return gsl_odeiv2_step_rkf45;

	case StepperType::RK8PD:
		// Dormand–Prince 8(5,3) – high order explicit RK.
		return gsl_odeiv2_step_rk8pd;

	case StepperType::RKCK:
		// Cash–Karp RK45.
		return gsl_odeiv2_step_rkck;

	case StepperType::RK2:
		// Simple 2nd-order RK.
		return gsl_odeiv2_step_rk2;

	default:
		// Fallback: reasonably robust nonstiff stepper.
		return gsl_odeiv2_step_rkf45;
	}
}

} // anonymous namespace

//------------------------------------------------------------------
// GSLIntegrator::GSLIntegrator
//------------------------------------------------------------------
GSLIntegrator::GSLIntegrator(const EvolutionSystem &sys, const Config &cfg)
	: m_sys(&sys), m_cfg(&cfg)
{
	if (!m_sys)
	{
		throw std::runtime_error("GSLIntegrator: EvolutionSystem pointer must not be null.");
	}
	if (!m_cfg)
	{
		throw std::runtime_error("GSLIntegrator: Config pointer must not be null.");
	}
}

//------------------------------------------------------------------
// GSLIntegrator::Integrate
//------------------------------------------------------------------
bool GSLIntegrator::Integrate(double t0, double t1, double *y, std::size_t n) const
{
	if (!m_sys || !m_cfg || !y)
		return false;

	if (t1 <= t0 || n == 0)
		return true; // nothing to do

	// --------------------------------------------------------------
	// 1. Set up gsl_odeiv2_system
	// --------------------------------------------------------------
	gsl_odeiv2_system sys{};
	sys.function = &gsl_rhs;
	sys.jacobian = nullptr; // finite-difference Jacobian (or add later)
	sys.dimension = static_cast<size_t>(n);
	sys.params = const_cast<EvolutionSystem *>(m_sys);

	// --------------------------------------------------------------
	// 2. Choose stepper and tolerances from Config
	// --------------------------------------------------------------
	const gsl_odeiv2_step_type *step_type = SelectStepper(m_cfg->stepper);

	const double rtol = (m_cfg->rtol > 0.0) ? m_cfg->rtol : 1.0e-6;
	const double atol = (m_cfg->atol > 0.0) ? m_cfg->atol : 1.0e-10;

	// Initial step guess:
	//    a small fraction of the interval by default.
	double h_init = (t1 - t0) * 1.0e-3;
	if (h_init <= 0.0)
	{
		h_init = 1.0; // fallback
	}

	gsl_odeiv2_driver *driver =
		gsl_odeiv2_driver_alloc_y_new(&sys,
									  step_type,
									  h_init,
									  rtol,
									  atol);

	if (!driver)
	{
		return false;
	}

	// Optional: we can (crudely) enforce max_steps by splitting [t0, t1]
	// into chunks and calling gsl_odeiv2_driver_apply repeatedly, counting
	// our own "macro-steps". This is *not* the same as the internal GSL
	// step counter, but it's a reasonable safety guard.
	double t = t0;
	std::size_t steps = 0;
	const std::size_t max_steps =
		(m_cfg->max_steps > 0) ? m_cfg->max_steps : static_cast<std::size_t>(1000000);

	int status = GSL_SUCCESS;

	while (t < t1)
	{
		// One apply() to advance as far as GSL wants toward t1.
		status = gsl_odeiv2_driver_apply(driver, &t, t1, y);

		if (status != GSL_SUCCESS)
		{
			break;
		}

		++steps;
		if (steps > max_steps)
		{
			status = GSL_EMAXITER; // exceeded user-specified cap
			break;
		}
	}

	gsl_odeiv2_driver_free(driver);

	return (status == GSL_SUCCESS);
}

} // namespace Evolution
} // namespace Physics
} // namespace CompactStar