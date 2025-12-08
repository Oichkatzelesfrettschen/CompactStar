// -*- lsst-c++ -*-
/*
 * CompactStar test main: coupled Spin + Thermal evolution
 *
 * Uses:
 *  - SpinState + MagneticDipole driver  (dΩ/dt)
 *  - ThermalState + PhotonCooling driver (dT∞/dt)
 *
 * This is primarily an infrastructure test:
 *  - multi-block StateLayout ({Thermal, Spin}),
 *  - multiple drivers writing into RHSAccumulator,
 *  - packing/unpacking via StatePacking,
 *  - GSLIntegrator for time-stepping.
 */

#include <iostream>
#include <memory>
#include <vector>

#include "CompactStar/Physics/Evolution/EvolutionConfig.hpp"
#include "CompactStar/Physics/Evolution/EvolutionSystem.hpp"
#include "CompactStar/Physics/Evolution/Integrator/GSLIntegrator.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateLayout.hpp"
#include "CompactStar/Physics/Evolution/StatePacking.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"

#include "CompactStar/Physics/Driver/Spin/MagneticDipole.hpp"
#include "CompactStar/Physics/Driver/Thermal/PhotonCooling.hpp"

#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/Tags.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

int main()
{
	using namespace CompactStar;

	// --------------------------------------------------------------
	// 1) Evolution configuration
	// --------------------------------------------------------------
	Physics::Evolution::Config cfg;

	// Enable spin in the state vector (so MagneticDipole has something to act on).
	cfg.couple_spin = true;

	// No chemical imbalances for this test.
	cfg.n_eta = 0;

	// Simple integrator settings for a short test run.
	cfg.stepper = Physics::Evolution::StepperType::RKF45;
	cfg.rtol = 1e-6;
	cfg.atol = 1e-10;
	cfg.max_steps = 1000000;
	cfg.dt_save = 1.0e5; // not used directly by GSLIntegrator yet, but kept for consistency

	// --------------------------------------------------------------
	// 2) Static context (star, geometry, envelope)
	// --------------------------------------------------------------
	//
	// For now we don't need any of these in the drivers we’re using, so we
	// can leave them as nullptr. Later, when you wire in real microphysics
	// and envelope models, you’ll fill these.
	Physics::Evolution::EvolutionSystem::Context ctx;
	ctx.star = nullptr;		// e.g. pointer to StarContext built from NStar + EOS
	ctx.geo = nullptr;		// e.g. GeometryCache
	ctx.envelope = nullptr; // e.g. Thermal::IEnvelope implementation
	ctx.cfg = &cfg;

	// --------------------------------------------------------------
	// 3) Dynamic state blocks: Thermal + Spin
	// --------------------------------------------------------------

	// --- Thermal block: 1 DOF = T∞ ---------------------------------
	Physics::State::ThermalState thermal;
	thermal.Resize(1);
	thermal.Tinf() = 1.0e8; // K, rough initial redshifted internal temperature

	// Optional: set T_surf if you use DirectTSurf model later.
	// For now we use ApproxFromTinf in PhotonCooling::Options, so this is not needed.
	// thermal.T_surf = 1.0e6;

	// --- Spin block: 1 DOF = Ω -------------------------------------
	Physics::State::SpinState spin;
	spin.Resize(1);
	spin.Omega() = 100.0; // initial spin frequency [rad/s], arbitrary

	// --------------------------------------------------------------
	// 4) StateVector: register both blocks with tags
	// --------------------------------------------------------------
	Physics::Evolution::StateVector stateVec;
	stateVec.Register(Physics::State::StateTag::Thermal, thermal);
	stateVec.Register(Physics::State::StateTag::Spin, spin);

	// --------------------------------------------------------------
	// 5) Layout: decide packing order in y[]
	// --------------------------------------------------------------
	//
	// Here we choose y = [ Thermal block, then Spin block ].
	// You could swap the order if you prefer.
	Physics::Evolution::StateLayout layout;
	layout.Configure(
		stateVec,
		{Physics::State::StateTag::Thermal,
		 Physics::State::StateTag::Spin});

	const std::size_t dim = layout.TotalSize();

	// --------------------------------------------------------------
	// 6) RHSAccumulator: configure blocks for both tags
	// --------------------------------------------------------------
	Physics::Evolution::RHSAccumulator rhs;
	rhs.Configure(Physics::State::StateTag::Thermal, thermal.Size());
	rhs.Configure(Physics::State::StateTag::Spin, spin.Size());

	// --------------------------------------------------------------
	// 7) Drivers: MagneticDipole (Spin) + PhotonCooling (Thermal)
	// --------------------------------------------------------------

	// --- Spin driver: MagneticDipole --------------------------------
	Physics::Driver::Spin::MagneticDipole::Options spinOpts;
	spinOpts.braking_index = 3.0;
	spinOpts.K_prefactor = 1e-15; // arbitrary; tweak to see visible evolution
	spinOpts.use_moment_of_inertia = false;

	auto spinDriver =
		std::make_shared<Physics::Driver::Spin::MagneticDipole>(spinOpts);

	// --- Thermal driver: PhotonCooling ------------------------------
	Physics::Driver::Thermal::PhotonCooling::Options thermOpts;

	// Use simple T_surf ≈ T∞ mapping for now.
	thermOpts.surface_model =
		Physics::Driver::Thermal::PhotonCooling::Options::SurfaceModel::ApproxFromTinf;

	// Effective area and heat capacity are just toy values here; adjust to keep
	// dT/dt in a reasonable range.
	thermOpts.area_eff = 1.0; // in code units; real code would use ~4πR^2 z^2
	thermOpts.C_eff = 1.0e40; // big C_eff → slow cooling for demonstration
	thermOpts.global_scale = 1.0;

	auto thermalDriver =
		std::make_shared<Physics::Driver::Thermal::PhotonCooling>(thermOpts);

	// --- Driver list -------------------------------------------------
	std::vector<Physics::Evolution::DriverPtr> drivers;
	drivers.push_back(spinDriver);
	drivers.push_back(thermalDriver);

	// --------------------------------------------------------------
	// 8) EvolutionSystem: glue together context, state, RHS, layout, drivers
	// --------------------------------------------------------------
	Physics::Evolution::EvolutionSystem system(
		ctx,
		stateVec,
		rhs,
		layout,
		std::move(drivers));

	// --------------------------------------------------------------
	// 9) Pack initial state into flat y[]
	// --------------------------------------------------------------
	std::vector<double> y(dim);
	Physics::Evolution::PackStateVector(stateVec, layout, y.data());

	std::cout << "Initial conditions:\n";
	std::cout << "  Tinf  = " << thermal.Tinf() << "\n";
	std::cout << "  Omega = " << spin.Omega() << "\n";

	// --------------------------------------------------------------
	// 10) GSL integrator and evolve
	// --------------------------------------------------------------
	Physics::Evolution::GSLIntegrator integrator(system, cfg, dim);

	const double t0 = 0.0;
	const double t1 = 1.0e10; // s; adjust as desired

	const bool ok = integrator.Integrate(t0, t1, y.data());
	if (!ok)
	{
		std::cerr << "GSLIntegrator: integration failed or max_steps exceeded.\n";
		return 1;
	}

	// --------------------------------------------------------------
	// 11) Unpack final state back into blocks and print
	// --------------------------------------------------------------
	Physics::Evolution::UnpackStateVector(stateVec, layout, y.data());

	std::cout << "Final conditions (t = " << t1 << " s):\n";
	std::cout << "  Tinf  = " << thermal.Tinf() << "\n";
	std::cout << "  Omega = " << spin.Omega() << "\n";

	return 0;
}