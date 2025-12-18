// -*- lsst-c++ -*-
/*
 * CompactStar test main: coupled Spin + Thermal evolution
 *
 * NEW PATH:
 *   NStar::SolveTOV_Profile(...) -> StarProfile -> StarContext(StarProfile) -> GeometryCache
 *
 * Then:
 *   ThermalState + PhotonCooling
 *   SpinState    + MagneticDipole
 *   EvolutionSystem + GSLIntegrator
 */

#include <cstdio>
#include <gsl/gsl_errno.h>
// #include <gsl/gsl_strerror.h>
#include <iostream>
#include <memory>
#include <vector>

#include <Zaki/String/Directory.hpp>
#include <Zaki/Util/Logger.hpp>

// -------------------------------------------------------
// Evolution infrastructure
// -------------------------------------------------------
#include "CompactStar/Physics/Evolution/EvolutionConfig.hpp"
#include "CompactStar/Physics/Evolution/EvolutionSystem.hpp"
#include "CompactStar/Physics/Evolution/GeometryCache.hpp"
#include "CompactStar/Physics/Evolution/Integrator/GSLIntegrator.hpp"
#include "CompactStar/Physics/Evolution/Observers/DiagnosticsObserver.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateLayout.hpp"
#include "CompactStar/Physics/Evolution/StatePacking.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
// -------------------------------------------------------
// State blocks
// -------------------------------------------------------
#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/Tags.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

// -------------------------------------------------------
// Drivers
// -------------------------------------------------------
#include "CompactStar/Physics/Driver/Spin/MagneticDipole.hpp"
#include "CompactStar/Physics/Driver/Thermal/PhotonCooling.hpp"

// -------------------------------------------------------
// Core: build the star profile the NEW way
// -------------------------------------------------------
#include "CompactStar/Core/NStar.hpp"

// -------------------------------------------------------
// GSL error handler (so GSL does not abort the program)
// -------------------------------------------------------
static void
my_gsl_error_handler(const char *reason,
					 const char *file,
					 int line,
					 int gsl_errno)
{
	std::fprintf(stderr,
				 "GSL ERROR: %s:%d: %s (%s)\n",
				 file, line, reason, gsl_strerror(gsl_errno));
	// keep going — this is a debug main
}

//==============================================================
int main()
{
	using namespace CompactStar;

	Zaki::String::Directory dir(__FILE__);
	std::cout << "[debug] this file dir        = " << dir << "\n";
	std::cout << "[debug] this file parent dir = " << dir.ParentDir() << "\n";

	// -------------------------------------------------------
	// 0) GSL + logging
	// -------------------------------------------------------
	gsl_set_error_handler(&my_gsl_error_handler);
	Zaki::Util::LogManager::SetLogLevels(Zaki::Util::LogLevel::Info);
	Zaki::Util::LogManager::SetBlackWhite(true);
	Zaki::Util::LogManager::SetLogFile(dir + "spin_therm_evol_2_main.log");
	// -------------------------------------------------------
	// 1) Build a star the NEW way: SolveTOV_Profile -> StarProfile
	// -------------------------------------------------------
	Zaki::String::Directory eos_root =
		dir.ParentDir().ParentDir() + "/EOS/CompOSE/";
	std::string eos_name = "DS(CMF)-1_with_crust";

	Zaki::String::Directory eos_file =
		eos_root + eos_name + "/" + eos_name + ".eos";

	Zaki::String::Directory base_results_dir = dir.ParentDir() + "/results";
	Zaki::String::Directory out_dir = "spin_therm_evol_2";

	Core::NStar ns;
	ns.SetWrkDir(base_results_dir);

	const double target_M = 1.8; // Msun
	const int n_rows = ns.SolveTOV_Profile(eos_file, target_M, out_dir);

	if (n_rows <= 0)
	{
		Z_LOG_ERROR("SolveTOV_Profile failed (n_rows <= 0).");
		return 1;
	}

	// Optional: export the profile (handy for sanity checks)
	ns.Export(out_dir + "/NStar_Profile.tsv");

	// -------------------------------------------------------
	// 2) StarContext + GeometryCache from *StarProfile*
	// -------------------------------------------------------
	Physics::Evolution::StarContext starCtx(ns.Profile());
	Physics::Evolution::GeometryCache geo(starCtx);

	// Validate GeometryCache contents
	if (geo.R().Size() == 0 || geo.Exp2Nu().Size() == 0)
	{
		Z_LOG_ERROR("GeometryCache is empty (R or Exp2Nu size is 0).");
		return 1;
	}

	// -------------------------------------------------------
	// 3) Evolution configuration
	// -------------------------------------------------------
	Physics::Evolution::Config cfg;

	cfg.couple_spin = true;
	cfg.n_eta = 0;

	cfg.stepper = Physics::Evolution::StepperType::RKF45;
	cfg.rtol = 1e-6;
	cfg.atol = 1e-10;
	cfg.max_steps = 1000000;
	cfg.dt_save = 1.0e5;

	// -------------------------------------------------------
	// 4) EvolutionSystem::Context wiring
	// -------------------------------------------------------
	Physics::Evolution::DriverContext ctx;
	ctx.star = &starCtx;
	ctx.geo = &geo;
	ctx.envelope = nullptr; // later
	ctx.cfg = &cfg;

	// -------------------------------------------------------
	// 5) Dynamic states: Thermal + Spin
	// -------------------------------------------------------
	Physics::State::ThermalState thermal;
	thermal.Resize(1);
	thermal.SetTinf(1.0e8); // K

	Physics::State::SpinState spin;
	spin.Resize(1);
	spin.Omega() = 100.0; // rad/s

	// -------------------------------------------------------
	// 6) Register blocks into StateVector
	// -------------------------------------------------------
	Physics::Evolution::StateVector stateVec;
	stateVec.Register(Physics::State::StateTag::Thermal, thermal);
	stateVec.Register(Physics::State::StateTag::Spin, spin);

	// -------------------------------------------------------
	// 7) Define packing order
	// -------------------------------------------------------
	Physics::Evolution::StateLayout layout;
	layout.Configure(
		stateVec,
		{Physics::State::StateTag::Thermal,
		 Physics::State::StateTag::Spin});

	const std::size_t dim = layout.TotalSize();

	// -------------------------------------------------------
	// 8) RHS accumulator
	// -------------------------------------------------------
	Physics::Evolution::RHSAccumulator rhs;
	rhs.Configure(Physics::State::StateTag::Thermal, thermal.Size());
	rhs.Configure(Physics::State::StateTag::Spin, spin.Size());

	// -------------------------------------------------------
	// 9) Drivers
	// -------------------------------------------------------
	Physics::Driver::Spin::MagneticDipole::Options spinOpts;
	spinOpts.braking_index = 3.0;
	spinOpts.K_prefactor = 1e-15;
	spinOpts.use_moment_of_inertia = false;

	auto spinDriver =
		std::make_shared<Physics::Driver::Spin::MagneticDipole>(spinOpts);

	Physics::Driver::Thermal::PhotonCooling::Options thermOpts;
	thermOpts.surface_model =
		Physics::Driver::Thermal::PhotonCooling::Options::SurfaceModel::ApproxFromTinf;

	// PhotonCooling uses GeometryCache (4πR^2 e^{2ν}) internally when ctx.geo is set.
	// radiating_fraction is a *dimensionless* prefactor (e.g. hot-spot fraction).
	thermOpts.radiating_fraction = 1.0;
	thermOpts.C_eff = 1.0e40;
	thermOpts.global_scale = 1.0;

	auto thermalDriver =
		std::make_shared<Physics::Driver::Thermal::PhotonCooling>(thermOpts);

	std::vector<Physics::Evolution::DriverPtr> drivers;
	drivers.push_back(spinDriver);
	drivers.push_back(thermalDriver);

	// -------------------------------------------------------
	// 10) Build EvolutionSystem
	// -------------------------------------------------------
	Physics::Evolution::EvolutionSystem system(
		ctx,
		stateVec,
		rhs,
		layout,
		std::move(drivers));

	// -------------------------------------------------------
	// 11) Observer: Diagnostics
	// -------------------------------------------------------
	namespace PhyEvolObs = Physics::Evolution::Observers;
	PhyEvolObs::DiagnosticsObserver::Options dopts;
	dopts.output_path = base_results_dir + "/" + out_dir + "/diagnostics.jsonl";
	dopts.record_every_n_steps = 1000; // Record every 1000 steps
	dopts.record_at_start = true;	   // common choice

	std::vector<const Physics::Driver::Diagnostics::IDriverDiagnostics *> diag_drivers;

	if (auto *p = dynamic_cast<const Physics::Driver::Diagnostics::IDriverDiagnostics *>(spinDriver.get()))
		diag_drivers.push_back(p);

	if (auto *p = dynamic_cast<const Physics::Driver::Diagnostics::IDriverDiagnostics *>(thermalDriver.get()))
		diag_drivers.push_back(p);

	auto diag = std::make_shared<PhyEvolObs::DiagnosticsObserver>(dopts, diag_drivers);

	system.AddObserver(diag);
	// -------------------------------------------------------
	// 12) Pack initial state -> y[]
	// -------------------------------------------------------
	std::vector<double> y(dim);
	Physics::Evolution::PackStateVector(stateVec, layout, y.data());

	std::cout << "Initial conditions:\n";
	std::cout << "  Tinf  = " << thermal.Tinf() << "\n";
	std::cout << "  Omega = " << spin.Omega() << "\n";

	// -------------------------------------------------------
	// 13) Integrate
	// -------------------------------------------------------
	Physics::Evolution::GSLIntegrator integrator(system, cfg, dim);

	const double t0 = 0.0;
	const double t1 = 1.0e10; // s

	const bool ok = integrator.Integrate(t0, t1, y.data());
	if (!ok)
	{
		std::cerr << "GSLIntegrator: integration failed or max_steps exceeded.\n";
		return 1;
	}

	// -------------------------------------------------------
	// 14) Unpack final state and print
	// -------------------------------------------------------
	Physics::Evolution::UnpackStateVector(stateVec, layout, y.data());

	std::cout << "Final conditions (t = " << t1 << " s):\n";
	std::cout << "  Tinf  = " << thermal.Tinf() << "\n";
	std::cout << "  Omega = " << spin.Omega() << "\n";

	std::cout << "[debug] done.\n";
	return 0;
}