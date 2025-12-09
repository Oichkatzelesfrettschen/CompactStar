// main.cpp — spin-only test of EvolutionSystem + GSLIntegrator

#include <iostream>
#include <memory>
#include <vector>

#include "CompactStar/Core/NStar.hpp"
#include "CompactStar/EOS/Model.hpp"

#include "CompactStar/Physics/Evolution/EvolutionConfig.hpp" // struct Config
#include "CompactStar/Physics/Evolution/EvolutionSystem.hpp"
#include "CompactStar/Physics/Evolution/Integrator/GSLIntegrator.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateLayout.hpp"
#include "CompactStar/Physics/Evolution/StatePacking.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"

#include "CompactStar/Physics/Driver/Spin/MagneticDipole.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"

int main()
{
	using namespace CompactStar;

	// ---------------------------------------------------------------------
	// 1) Build config (spin-only test)
	// ---------------------------------------------------------------------
	Physics::Evolution::Config cfg;
	cfg.couple_spin = true;								  // we want Ω in the ODE state
	cfg.n_eta = 0;										  // no chemical DOFs here
	cfg.dt_save = 1.0e5;								  // arbitrary
	cfg.stepper = Physics::Evolution::StepperType::RKF45; // or EulerExplicit/MSBDF

	// For this minimal spin-only test, StarContext / GeometryCache / envelope
	// are not actually used by MagneticDipole. We keep them as nullptr for now.
	Physics::Thermal::IEnvelope *envelope = nullptr;

	Physics::Evolution::EvolutionSystem::Context ctx;
	ctx.star = nullptr;		 // TODO: hook to real StarContext
	ctx.geo = nullptr;		 // TODO: hook to real GeometryCache
	ctx.envelope = envelope; // may remain nullptr until thermal is wired
	ctx.cfg = &cfg;

	// ---------------------------------------------------------------------
	// 2) States: just SpinState (one DOF = Ω)
	// ---------------------------------------------------------------------
	Physics::State::SpinState spin;
	spin.Resize(1);		  // one DOF
	spin.Omega() = 100.0; // initial Ω

	Physics::Evolution::StateVector stateVec;
	stateVec.Register(Physics::State::StateTag::Spin, spin);

	// ---------------------------------------------------------------------
	// 3) Layout: only Spin block in y[]
	// ---------------------------------------------------------------------
	Physics::Evolution::StateLayout layout;
	layout.Configure(stateVec, {Physics::State::StateTag::Spin});

	const std::size_t dim = layout.TotalSize();

	// ---------------------------------------------------------------------
	// 4) RHS accumulator: configure Spin block
	// ---------------------------------------------------------------------
	Physics::Evolution::RHSAccumulator rhs;
	rhs.Configure(Physics::State::StateTag::Spin, spin.Size());

	// ---------------------------------------------------------------------
	// 5) Drivers: magnetic dipole torque
	// ---------------------------------------------------------------------
	Physics::Driver::Spin::MagneticDipole::Options opts;
	opts.braking_index = 3.0;
	opts.K_prefactor = 1e-15; // tweak to taste
	opts.use_moment_of_inertia = false;

	std::vector<Physics::Evolution::DriverPtr> drivers;
	drivers.push_back(
		std::make_shared<Physics::Driver::Spin::MagneticDipole>(opts));

	// ---------------------------------------------------------------------
	// 6) EvolutionSystem: glue context + state + RHS + drivers
	// ---------------------------------------------------------------------
	Physics::Evolution::EvolutionSystem system(
		ctx,
		stateVec,
		rhs,
		layout,
		std::move(drivers));

	// ---------------------------------------------------------------------
	// 7) Allocate y[] and pack initial state
	// ---------------------------------------------------------------------
	std::vector<double> y(dim);
	Physics::Evolution::PackStateVector(stateVec, layout, y.data());

	std::cout << "Initial Omega = " << spin.Omega() << "\n";

	// ---------------------------------------------------------------------
	// 8) GSLIntegrator: construct and integrate
	// ---------------------------------------------------------------------
	Physics::Evolution::GSLIntegrator integrator(system, cfg, dim);

	const double t0 = 0.0;
	const double t1 = 1.0e10; // total integration time

	const bool ok = integrator.Integrate(t0, t1, y.data());
	if (!ok)
	{
		std::cerr << "Integration failed or stopped early.\n";
		return 1;
	}

	// ---------------------------------------------------------------------
	// 9) Unpack final y[] back into StateVector and inspect SpinState
	// ---------------------------------------------------------------------
	Physics::Evolution::UnpackStateVector(stateVec, layout, y.data());

	std::cout << "Final  Omega = " << spin.Omega() << "\n";

	return 0;
}