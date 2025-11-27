// -*- lsst-c++ -*-
/*
 * CompactStar — ChemicalHeating
 *
 * Phase 1 integrator: a small RK4 stepping loop so we can run end-to-end now.
 * We will replace this with a gsl_odeiv2 driver in Phase 2 while preserving the API.
 */

#include "CompactStar/ChemicalHeating/IntegratorGSL.hpp"
#include "CompactStar/ChemicalHeating/Config.hpp"
#include "CompactStar/ChemicalHeating/EvolutionState.hpp"
#include "CompactStar/ChemicalHeating/EvolutionSystem.hpp"
#include "CompactStar/ChemicalHeating/Observers.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace CompactStar
{
namespace ChemicalHeating
{

namespace
{
// =============================================================
// [ZAKI-QUESTION]: Packing order agreed? y[0]=Tinf, then eta[0..n-1], then Omega (optional).
static inline void pack_state(const EvolutionState &s, const Config &cfg, std::vector<double> &y)
{
	const std::size_t Neta = (cfg.n_eta > 0 ? cfg.n_eta : std::size_t(1));
	y.resize(1 + Neta + (cfg.couple_spin ? 1 : 0));
	y[0] = s.Tinf;
	for (std::size_t k = 0; k < Neta; ++k)
	{
		const double val = (k < s.eta.size() ? s.eta[k] : 0.0);
		y[1 + k] = val;
	}
	if (cfg.couple_spin)
		y[1 + Neta] = s.Omega;
}
// =============================================================
static inline void unpack_state(const std::vector<double> &y, const Config &cfg, EvolutionState &s)
{
	const std::size_t Neta = (cfg.n_eta > 0 ? cfg.n_eta : std::size_t(1));
	s.Tinf = y[0];
	s.eta.resize(Neta);
	for (std::size_t k = 0; k < Neta; ++k)
		s.eta[k] = y[1 + k];
	if (cfg.couple_spin)
		s.Omega = y[1 + Neta];
}

//==============================================================
//                   RK4 Struct
//==============================================================
struct RK4
{
	template <class F>
	static void step(F &&f, double t, double h, std::vector<double> &y, std::vector<double> &k, std::vector<double> &tmp)
	{
		const std::size_t n = y.size();
		k.resize(n);
		tmp.resize(n);

		// k1 = f(t, y)
		f(t, y.data(), k.data());
		for (std::size_t i = 0; i < n; ++i)
			tmp[i] = y[i] + 0.5 * h * k[i];

		// k2
		std::vector<double> k2(n);
		f(t + 0.5 * h, tmp.data(), k2.data());
		for (std::size_t i = 0; i < n; ++i)
			tmp[i] = y[i] + 0.5 * h * k2[i];

		// k3
		std::vector<double> k3(n);
		f(t + 0.5 * h, tmp.data(), k3.data());
		for (std::size_t i = 0; i < n; ++i)
			tmp[i] = y[i] + h * k3[i];

		// k4
		std::vector<double> k4(n);
		f(t + h, tmp.data(), k4.data());

		// y_{n+1}
		for (std::size_t i = 0; i < n; ++i)
		{
			y[i] += (h / 6.0) * (k[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
		}
	}
};

} // anonymous namespace

//==============================================================
//                   IntegratorGSL Class
//==============================================================
IntegratorGSL::IntegratorGSL(const EvolutionSystem &sys, const Config &cfg)
	: m_sys(&sys), m_cfg(&cfg)
{
}
//--------------------------------------------------------------
bool IntegratorGSL::integrate(double t0, double t1, EvolutionState &state, Observer &obs) const
{
	// [ZAKI-QUESTION]: For the walking skeleton, we do fixed dt based on dt_save.
	// When we switch to GSL, we’ll respect rtol/atol and still sample at dt_save.
	const double dt_save = (m_cfg ? m_cfg->dt_save : 1.0e2);
	const std::size_t Neta = (m_cfg && m_cfg->n_eta > 0) ? m_cfg->n_eta : std::size_t(1);

	std::vector<double> y, k, tmp;
	pack_state(state, *m_cfg, y);

	double t = t0;
	state.t = t;

	// Initial sample
	{
		Derived d; // Phase-1: zeros; we’ll compute Ts/Lnu/H later when micro is wired.
		obs.on_sample(state, d);
	}

	// Fixed-step loop (Phase-1)
	while (t < t1)
	{
		const double h = std::min(dt_save, t1 - t);

		// One RK4 step (using EvolutionSystem functor)
		RK4::step(
			[this](double tt, const double *yin, double *yout)
			{
				// Forward to EvolutionSystem::operator()
				return this->m_sys->operator()(tt, yin, yout);
			},
			t, h, y, k, tmp);

		t += h;
		unpack_state(y, *m_cfg, state);
		state.t = t;
		state.step += 1;

		Derived d; // zeros for now
		// [ZAKI-QUESTION]: Do you want Ts_local computed here via an envelope model
		// once we pass it through EvolutionSystem’s Context? Right now we can’t see that Context.
		obs.on_sample(state, d);
	}

	obs.on_finish(state, /*ok=*/true);
	return true;
}
//--------------------------------------------------------------
//==============================================================
} // namespace ChemicalHeating
} // namespace CompactStar