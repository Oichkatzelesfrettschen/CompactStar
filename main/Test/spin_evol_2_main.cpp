#include <memory>
#include <vector>

#include "CompactStar/Core/NStar.hpp"
#include "CompactStar/EOS/Model.hpp"

#include "CompactStar/Physics/Evolution/EvolutionConfig.hpp"
#include "CompactStar/Physics/Evolution/EvolutionSystem.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateLayout.hpp"
#include "CompactStar/Physics/Evolution/StatePacking.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"

#include "CompactStar/Physics/Driver/Spin/MagneticDipole.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"

#include <gsl/gsl_odeiv2.h>

int main()
{
	using namespace CompactStar;

	// 1) Build star + EOS (however you already do this)
	// Core::NStar star = /* ... existing builder ... */;
	// EOS::Model eos = /* ... */;
	// Physics::Evolution::StarContext starCtx(star, &eos);
	// Physics::Evolution::GeometryCache geo(starCtx);

	// 2) Config
	Physics::Evolution::Config cfg;
	cfg.couple_spin = true;
	cfg.n_eta = 0;

	// envelope (you already have some Thermal::IEnvelope impl)
	Physics::Thermal::IEnvelope *envelope = nullptr;

	// 3) EvolutionSystem::Context
	Physics::Evolution::EvolutionSystem::Context ctx;
	ctx.star = nullptr; // &starCtx;
	ctx.geo = nullptr;	// &geo;
	ctx.envelope = envelope;
	ctx.cfg = &cfg;

	// 4) States
	Physics::State::SpinState spin;
	spin.Resize(1);		  // one DOF = Ω
	spin.Omega() = 100.0; // initial spin (example)

	Physics::Evolution::StateVector stateVec;
	stateVec.Register(Physics::State::StateTag::Spin, spin);

	// 5) Layout
	Physics::Evolution::StateLayout layout;
	layout.Configure(stateVec, {Physics::State::StateTag::Spin});

	// 6) RHS accumulator
	Physics::Evolution::RHSAccumulator rhs;
	rhs.Configure(Physics::State::StateTag::Spin, spin.Size());

	// 7) Drivers
	Physics::Driver::Spin::MagneticDipole::Options opts;
	opts.braking_index = 3.0;
	opts.K_prefactor = 1e-15; // whatever you use
	opts.use_moment_of_inertia = false;

	std::vector<Physics::Evolution::DriverPtr> drivers;
	drivers.push_back(std::make_shared<Physics::Driver::Spin::MagneticDipole>(opts));

	// 8) EvolutionSystem
	Physics::Evolution::EvolutionSystem system(ctx, stateVec, rhs, layout, std::move(drivers));

	// 9) GSL system wrapper
	gsl_odeiv2_system gsl_sys;
	gsl_sys.function = [](double t, const double y[], double dydt[], void *params) -> int
	{
		auto *sys = static_cast<Physics::Evolution::EvolutionSystem *>(params);
		return (*sys)(t, y, dydt);
	};
	gsl_sys.jacobian = nullptr;
	gsl_sys.dimension = layout.TotalSize();
	gsl_sys.params = &system;

	// 10) Allocate and initialize y[]
	std::vector<double> y(layout.TotalSize());
	Physics::Evolution::PackStateVector(stateVec, layout, y.data());

	// 11) Create a GSL driver and integrate…
	//     (use your existing GSL wrapper or straight gsl_odeiv2_driver)

	return 0;
}