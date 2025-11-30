// -*- lsst-c++ -*-
/*
 * CompactStar
 * Simple spin-evolution test using MagneticDipole driver.
 *
 * This is a stand-alone example main() that:
 *   - constructs a SpinState with one evolved DOF (Ω),
 *   - wraps it in a Physics::Evolution::StateVector,
 *   - prepares a Physics::Evolution::RHSAccumulator,
 *   - applies Physics::Driver::Spin::MagneticDipole in a simple
 *     explicit-Euler time loop,
 *   - prints t and Ω(t) to stdout.
 *
 * Drop this somewhere like:
 * and add a small executable target in CMake if desired.
 */

#include <cmath>
#include <iostream>

#include "CompactStar/Physics/Driver/Spin/MagneticDipole.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

int main()
{
	using namespace CompactStar;
	using Physics::State::SpinState;
	using Physics::State::StateTag;
	namespace Evol = Physics::Evolution;
	namespace SpinDrv = Physics::Driver::Spin;

	// ---------------------------------------------------------------------
	// 1. Set up SpinState with one evolved DOF: Ω
	// ---------------------------------------------------------------------
	SpinState spin;

	// One scalar DOF (index 0) for the evolved angular frequency Ω.
	spin.Resize(1);

	// Choose an initial period P0 and convert to Ω0 = 2π / P0.
	const double P0 = 0.1; // [s]
	const double two_pi = 2.0 * M_PI;
	const double Omega0 = two_pi / P0;

	spin.Omega() = Omega0;

	// Optional: set observational quantities (not used by the driver yet).
	spin.P = {P0, 0.0};
	spin.Pdot = {1.0e-15, 0.0}; // arbitrary small positive Pd
	spin.mu = {0.0, 0.0};
	spin.d = {1.0, 0.0}; // 1 kpc, just as a placeholder

	// ---------------------------------------------------------------------
	// 2. Wrap in StateVector and prepare RHSAccumulator
	// ---------------------------------------------------------------------
	Evol::StateVector Y;
	Y.Register(StateTag::Spin, spin);

	Evol::RHSAccumulator dYdt;
	dYdt.Configure(StateTag::Spin, spin.Size()); // one component: Ω

	// ---------------------------------------------------------------------
	// 3. Build MagneticDipole driver and context
	// ---------------------------------------------------------------------
	SpinDrv::MagneticDipole::Options opts;
	opts.braking_index = 3.0;			// canonical dipole
	opts.K_prefactor = 1.0e-10;			// toy value; tune as needed
	opts.use_moment_of_inertia = false; // not using ctx for K yet

	SpinDrv::MagneticDipole driver(opts);

	// Assumes StarContext is default-constructible; if not, adapt as needed.
	Evol::StarContext ctx;

	// ---------------------------------------------------------------------
	// 4. Simple explicit-Euler evolution loop
	// ---------------------------------------------------------------------
	double t = 0.0;				 // [s]
	const double t_end = 1.0e11; // [s] ~ 3 kyr for example
	const double dt = 1.0e6;	 // [s] step size

	const std::size_t n_steps = static_cast<std::size_t>((t_end - t) / dt);

	std::cout << "# t[s]  Omega[rad/s]\n";

	for (std::size_t step = 0; step <= n_steps; ++step)
	{
		// Output current time and spin frequency
		std::cout << "(" << t << ", " << spin.Omega() << ")\t";

		// Clear RHS, ask driver to contribute, then update Ω
		dYdt.Clear();
		driver.AccumulateRHS(t, Y, dYdt, ctx);

		const auto &spin_rhs = dYdt.Block(StateTag::Spin);
		if (!spin_rhs.empty())
		{
			const double dOmega_dt = spin_rhs[0];
			spin.Omega() += dOmega_dt * dt;
		}

		t += dt;
	}

	return 0;
}