/**
 * @file Pulsar.hpp
 * @brief Compact, modern pulsar container built on StarProfile + modular physics.
 *
 * This version of the Pulsar class is **not** the old monolithic one.
 * It is a thin orchestrator that:
 *  - owns (or is given) a structural profile (`StarProfile`)
 *  - exposes a non-owning `StarProfileView` for physics algorithms
 *  - stores observed / kinematic quantities in a `SpinState`
 *  - stores thermal quantities in a `ThermalState`
 *  - caches BNV-related diagnostics in a `BNVState`
 *  - optionally knows about the EOS (`CompOSE_EOS`)
 *  - optionally remembers the sequence point (`SeqPoint`) and its parent sequence
 *
 * All actual *physics* (spin-derived quantities, thermal luminosities, BNV spin-down
 * bounds, neutrino luminosities, etc.) lives in the lightweight headers:
 *
 *  - `CompactStar/Physics/Driver/Spin/`
 *  - `CompactStar/Physics/Driver/Thermal/`
 *  - `CompactStar/Physics/Driver/Chem/`
 *
 * so that the Pulsar class does not grow a giant public interface again.
 *
 * @ingroup Core
 * @author
 *   Mohammadreza Zakeri <M.Zakeri@eku.edu>
 *
 * @date   Nov 1, 2025
 * @version 2.0
 *
 * @par Design
 * This class is meant to sit **on top of** the new profile infrastructure
 * (`StarProfile`, `StarProfileView`) and the new physics modules. It should
 * not re-implement Urca emissivities, temperature profiles, or spin-down limits.
 * It only wires data together and provides convenience wrappers.
 */

#ifndef CompactStar_Core_Pulsar_H
#define CompactStar_Core_Pulsar_H

#include <string>

#include "CompactStar/Core/Prog.hpp"
#include "CompactStar/Core/SeqPoint.hpp"
#include "CompactStar/Core/StarProfile.hpp"
#include "CompactStar/EOS/CompOSE_EOS.hpp"

// modular physics/state headers
#include "CompactStar/Physics/State/BNVState.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

namespace CompactStar::Core
{

/**
 * @class Pulsar
 * @brief High-level pulsar object that bundles structure, spin, thermal, and BNV state.
 *
 * ### Responsibilities
 * - Owns (or imports) a **structural profile** (`StarProfile`) and exposes it as a view.
 * - Stores **observed spin parameters** in a `SpinState`.
 * - Stores **thermal parameters** in a `ThermalState`.
 * - Stores **BNV diagnostics** in a `BNVState`.
 * - Optionally attaches an **EOS**.
 * - Optionally remembers its **sequence point** (`SeqPoint`) and the parent **sequence profile**
 *   (for mass–radius sequences, evolutionary tracks, etc.).
 *
 * ### Non-Responsibilities
 * - Does **not** do raw TOV integration (that’s still in `TOVSolver`).
 * - Does **not** implement DUrca/MUrca formulas directly (that’s in `Physics/Thermal.hpp`).
 * - Does **not** hard-code plotting.
 *
 * The idea is to keep this class small and make it the thing our
 * higher-level code passes around.
 */
class Pulsar : public Prog
{
  private:
	// ------------------------------------------------------------
	//  Structural data
	// ------------------------------------------------------------
	/**
	 * @brief Owned structural profile of the star.
	 *
	 * This is the “new style” profile, not the old hard-coded DataSet
	 * with r/m/eps/nu columns. All derived physics should go through
	 * `view_` below.
	 */
	StarProfile prof_;

	/**
	 * @brief Non-owning view of the structural profile.
	 *
	 * This is what we pass to `Spin::DipoleFieldEstimate(...)`,
	 * `Thermal::SurfacePhotonLuminosity(...)`, `BNV::SpinDownLimit(...)`, etc.
	 * `view_.p` must be set whenever `prof_` is filled.
	 */
	StarProfileView view_;

	/**
	 * @brief Pulsar mass as a quantity with uncertainty.
	 */
	Zaki::Math::Quantity mp; /**< Pulsar mass (M_sun). */
	// ------------------------------------------------------------
	//  Sequence / EOS
	// ------------------------------------------------------------
	/**
	 * @brief Sequence point along a mass–radius or central-density sequence.
	 *
	 * Useful when this pulsar was *picked* from a sequence and we want to
	 * remember where it came from.
	 */
	SeqPoint seq_point_;

	/**
	 * @brief Pointer to the full sequence dataset, if available.
	 *
	 * This is non-owning on purpose — usually this lives in a higher-level
	 * object or was read from disk once.
	 */
	const Zaki::Vector::DataSet *seq_profile_ = nullptr;

	/**
	 * @brief EOS attached to this pulsar (optional).
	 */
	CompOSE_EOS *eos_ = nullptr;

	// ------------------------------------------------------------
	//  Physics state
	// ------------------------------------------------------------
	/**
	 * @brief Observed/kinematic spin parameters of the pulsar.
	 *
	 * Contains (P), (dot{P}), proper motion, and distance.
	 */
	Physics::State::SpinState spin_;

	/**
	 * @brief Thermal state: core, blanket, and surface temperatures [K].
	 */
	Physics::State::ThermalState thermal_;

	/**
	 * @brief Cached BNV-related quantities.
	 *
	 * e.g. \f$\eta_I\f$ and spin-down BNV bound.
	 */
	Physics::State::BNVState bnv_;

  public:
	// ------------------------------------------------------------
	//  Ctors / Dtor
	// ------------------------------------------------------------

	/**
	 * @brief Default constructor.
	 *
	 * Creates an empty pulsar with an invalid view. We must later
	 * call an import / construct method to populate the profile and
	 * set the view.
	 */
	Pulsar();

	/**
	 * @brief Construct a Pulsar with a name (for logging via Prog).
	 * @param name Pulsar identifier.
	 */
	explicit Pulsar(const std::string &name);

	/**
	 * @brief Construct from name, mass, and an explicit spin state.
	 *
	 * This is the “new” form: structure info (mass) stays here,
	 * spin info lives in a SpinState.
	 */
	Pulsar(const std::string &name,
		   const Zaki::Math::Quantity &mass,
		   const Physics::State::SpinState &spin);

	/**
	 * @brief Backward-compatible constructor (old style).
	 *
	 * Lets us do:
	 *   Pulsar("J0348...", {2.01,0.04}, {P, dP}, {Pdot, dPdot})
	 *
	 * Internally this just builds a SpinState and forwards to the
	 * (name, mass, SpinState) ctor.
	 */
	Pulsar(const std::string &name,
		   const Zaki::Math::Quantity &mass,
		   const Zaki::Math::Quantity &spin_p,
		   const Zaki::Math::Quantity &spin_pdot);

	/**
	 * @brief Default destructor.
	 */
	~Pulsar() = default;

	// ------------------------------------------------------------
	//  EOS / profile setup
	// ------------------------------------------------------------

	/**
	 * @brief Attach an EOS object to this pulsar.
	 *
	 * @param e Pointer to a CompOSE_EOS instance. Not owned.
	 */
	void AttachEOS(CompOSE_EOS *e) { eos_ = e; }

	/**
	 * @brief Get the attached EOS (may be nullptr).
	 * @return Pointer to EOS.
	 */
	CompOSE_EOS *GetEOS() const { return eos_; }

	/**
	 * @brief Build or load the structural profile using a model name.
	 *
	 * This should:
	 *  1. fill `prof_` from TOV / disk
	 *  2. set `view_.p = &prof_`
	 *
	 * @param model_name EOS/sequence/model identifier.
	 * @param in_dir     Directory to read from (optional).
	 * @return Number of radial points in the resulting profile.
	 */
	int FindProfile(const std::string &model_name,
					const Zaki::String::Directory &in_dir = "");

	/**
	 * @brief Import an already computed profile from disk.
	 *
	 * Exactly like `FindProfile(...)` but without doing the TOV solve.
	 *
	 * @param model_name EOS/sequence/model identifier.
	 * @param in_dir     Directory to read from.
	 */
	void ImportProfile(const std::string &model_name,
					   const Zaki::String::Directory &in_dir = "");

	/**
	 * @brief Expose the internal structural profile (const).
	 * @return Const pointer to `StarProfile`.
	 */
	const StarProfile *GetProfile() const { return &prof_; }

	/**
	 * @brief Get a non-owning view of the profile.
	 *
	 * Prefer to pass this to physics functions.
	 *
	 * @return Copy of `StarProfileView` (cheap).
	 */
	StarProfileView GetProfileView() const { return view_; }

	/**
	 * @brief Get the Mass object
	 *
	 * @return Zaki::Math::Quantity
	 */
	Zaki::Math::Quantity GetMass() const { return mp; }

	/**
	 * @brief Set the Mass object
	 *
	 * @param in_mp
	 */
	void SetMass(const Zaki::Math::Quantity &in_mp) { mp = in_mp; }

	/**
	 * @brief Return spin period \(P\) in seconds.
	 */
	Zaki::Math::Quantity GetSpinP() const
	{
		return spin_.P;
	}

	/**
	 * @brief Return spin period derivative \(\dot{P}\) in s/s.
	 */
	Zaki::Math::Quantity GetSpinPDot() const
	{
		return spin_.Pdot;
	}

	// distance lives in SpinState, but we expose it here
	/**
	 * @brief Get distance from the Solar System Barycentre (kpc).
	 */
	Zaki::Math::Quantity GetDistance() const
	{
		return spin_.d;
	}

	/**
	 * @brief Return proper motion \(\mu\) in mas/yr.
	 */
	Zaki::Math::Quantity GetProperMotion() const
	{
		return spin_.mu;
	}

	/**
	 * @brief Return fractional spin-change rate \(\dot{P}/P\) in 1/s.
	 */
	Zaki::Math::Quantity GetSpinPDot_over_P() const
	{
		// guard against P = 0 just in case
		if (spin_.P.val == 0.0)
		{
			return {0.0, 0.0};
		}
		return spin_.Pdot / spin_.P;
	}

	// ------------------------------------------------------------
	//  Sequence interface
	// ------------------------------------------------------------

	/**
	 * @brief Set which sequence point this pulsar corresponds to.
	 * @param sp Sequence point.
	 */
	void SetSeqPoint(const SeqPoint &sp) { seq_point_ = sp; }

	/**
	 * @brief Get the sequence point of this pulsar.
	 * @return Sequence point (by value).
	 */
	SeqPoint GetSeqPoint() const { return seq_point_; }

	/**
	 * @brief Attach the parent sequence dataset (non-owning).
	 * @param ds Pointer to the sequence dataset.
	 */
	void SetSeqProfile(const Zaki::Vector::DataSet *ds) { seq_profile_ = ds; }

	/**
	 * @brief Get the parent sequence dataset (may be nullptr).
	 * @return Pointer to dataset.
	 */
	const Zaki::Vector::DataSet *GetSeqProfile() const { return seq_profile_; }

	// ------------------------------------------------------------
	//  Spin interface
	// ------------------------------------------------------------

	/**
	 * @brief Set the spin period \(P\) of the pulsar.
	 * @param P Spin period [s].
	 */
	void SetSpinP(const Zaki::Math::Quantity &P) { spin_.P = P; }

	/**
	 * @brief Set the spin period derivative \(\dot{P}\).
	 * @param Pd Spin period derivative [s/s].
	 */
	void SetSpinPDot(const Zaki::Math::Quantity &Pd) { spin_.Pdot = Pd; }

	/**
	 * @brief Set the distance from the SSB.
	 * @param d Distance [kpc].
	 */
	void SetDistance(const Zaki::Math::Quantity &d) { spin_.d = d; }

	/**
	 * @brief Set the proper motion \(\mu\).
	 * @param mu Proper motion [mas/yr].
	 */
	void SetProperMotion(const Zaki::Math::Quantity &mu) { spin_.mu = mu; }

	/**
	 * @brief Get the full spin/kinematic state.
	 * @return SpinState by value.
	 */
	Physics::State::SpinState GetSpinState() const { return spin_; }

	/**
	 * @brief Convenience wrapper for the characteristic age \(\tau_c = P/(2\dot{P})\).
	 *
	 * Delegates to `CompactStar::Physics::Driver::Spin::CharacteristicAge(...)`.
	 *
	 * @return Characteristic age in seconds (0.0 if \(\dot{P}=0\)).
	 */
	double GetCharacteristicAge() const;
	// {
	// 	return Spin::CharacteristicAge(spin_);
	// }

	/**
	 * @brief Convenience wrapper for estimated dipole field.
	 *
	 * Delegates to `CompactStar::Physics::Driver::Spin::DipoleFieldEstimate(...)` **if** the
	 * profile view is valid; otherwise returns 0.
	 *
	 * @return Estimated equatorial dipole field [G].
	 */
	double GetDipoleFieldEstimate() const;
	// {
	// 	return view_.valid() ? Spin::DipoleFieldEstimate(spin_, view_) : 0.0;
	// }

	// ------------------------------------------------------------
	//  Thermal interface
	// ------------------------------------------------------------

	/**
	 * @brief Set the thermal state (core / blanket / surface temperatures).
	 * @param t Thermal state.
	 */
	void SetThermalState(const Physics::State::ThermalState &t) { thermal_ = t; }

	/**
	 * @brief Get the current thermal state.
	 * @return ThermalState by value.
	 */
	Physics::State::ThermalState GetThermalState() const { return thermal_; }

	/**
	 * @brief Redshifted photon luminosity from the surface.
	 *
	 * Delegates to `CompactStar::Physics::Driver::Thermal::SurfacePhotonLuminosity(...)`
	 * if the structural view is valid.
	 *
	 * @return \(L_{\infty}\) in [erg s\(^{-1}\)], or 0.0 if invalid.
	 */
	// double GetSurfacePhotonLuminosity() const
	// {
	// 	return view_.valid() ? Thermal::SurfacePhotonLuminosity(view_, thermal_) : 0.0;
	// }

	/**
	 * @brief Total neutrino luminosity (DUrca + MUrca), redshifted.
	 *
	 * Delegates to `CompactStar::Thermal::TotalNeutrinoLuminosity(...)`.
	 *
	 * @return \(L_{\nu,\infty}\) in [erg s\(^{-1}\)], or 0.0 if invalid.
	 */
	// double GetTotalNeutrinoLuminosity() const
	// {
	// 	return view_.valid() ? Thermal::TotalNeutrinoLuminosity(view_, thermal_) : 0.0;
	// }

	// ------------------------------------------------------------
	//  BNV interface
	// ------------------------------------------------------------

	/**
	 * @brief Recompute and store the BNV spin-down limit for this pulsar.
	 *
	 * Calls `CompactStar::BNV::SpinDownLimit(view_, spin_)` and stores the
	 * result in `bnv_.spin_down_limit`.
	 *
	 * @return The newly computed BNV spin-down limit [1/yr].
	 */
	// double ComputeBNVSpinDownLimit()
	// {
	// 	if (!view_.valid())
	// 		return 0.0;
	// 	bnv_.spin_down_limit = BNV::SpinDownLimit(view_, spin_);
	// 	return bnv_.spin_down_limit;
	// }

	/**
	 * @brief Get the last cached BNV spin-down limit.
	 *
	 * If we never called `ComputeBNVSpinDownLimit()`, this will be 0.
	 *
	 * @return BNV spin-down limit [1/yr].
	 */
	double GetBNVSpinDownLimit() const
	{
		// return bnv_.spin_down_limit;
		// Safe even if BNVState has not been resized yet.
		if (bnv_.NumComponents() < 2)
			return 0.0;
		return bnv_.SpinDownLimit();
	}
};

} // namespace CompactStar::Core

#endif /* CompactStar_Core_Pulsar_H */