#pragma once
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
 * @file IObserver.hpp
 * @brief Core observer interface for Evolution module.
 *
 * Observers are passive callbacks invoked by the integrator / evolution driver
 * to record or export information (time series, checkpoints, diagnostics, logs).
 *
 * Key design points:
 *  - Observers see the full composite state (Evolution::StateVector) and
 *    the read-only runtime context (Evolution::DriverContext).
 *  - Observers MUST NOT mutate the ODE state or context.
 *  - Observers are scheduled by the evolution loop (e.g., dt_save). The interface
 *    does not prescribe scheduling logic; it just provides hooks.
 *
 * Typical call pattern:
 *  1) obs->OnStart(run, Y0, ctx)
 *  2) During integration, at each "save point":
 *        obs->OnSample(sample, Y, ctx)
 *     (optionally, future: obs->OnStep(step, Y, ctx) for every integrator step)
 *  3) obs->OnFinish(result, Y_final, ctx)
 *
 * @ingroup Evolution
 */

#include <cstdint>
#include <string>

namespace CompactStar::Physics::Evolution
{
class StateVector;
class DriverContext;
} // namespace CompactStar::Physics::Evolution

namespace CompactStar::Physics::Evolution::Observers
{

/**
 * @brief Run-level metadata for observer initialization.
 *
 * This is intentionally small and stable. Add fields only if they are
 * truly run-level (not per-sample).
 */
struct RunInfo
{
	/// Optional label for the run (useful in filenames / metadata).
	std::string tag;

	/// Output directory chosen by the driver (observers may create subdirs).
	std::string output_dir;

	/// The integrator's notion of initial time.
	double t0 = 0.0;

	/// The integrator's notion of target/final time (if known).
	double tf = 0.0;
};

/**
 * @brief Snapshot metadata for a recorded sample.
 *
 * A "sample" is typically emitted every dt_save (or at explicit output times).
 * This is not necessarily each internal integrator step.
 */
struct SampleInfo
{
	/// Simulation time at the sample.
	double t = 0.0;

	/// Monotonic sample counter: 0,1,2,... (not necessarily equal to integrator step count).
	std::uint64_t sample_index = 0;

	/// Optional: integrator internal step counter if you want to expose it (0 if unknown).
	std::uint64_t step_index = 0;
};

/**
 * @brief Outcome of the integration for observer finalization.
 */
struct FinishInfo
{
	/// Time at which the integration terminated (success or failure).
	double t_final = 0.0;

	/// True if the evolution reached the intended target condition (e.g. t >= tf).
	bool ok = false;

	/// If !ok, a brief reason string (may be empty).
	std::string message;
};

/**
 * @brief Base interface for evolution observers.
 *
 * Observers should be cheap to construct; heavy resources should be opened in OnStart.
 */
class IObserver
{
  public:
	virtual ~IObserver();

	/**
	 * @brief Called once before integration begins.
	 *
	 * Use this for:
	 *  - opening output files,
	 *  - writing headers/metadata,
	 *  - allocating datasets/buffers,
	 *  - validating required context dependencies.
	 *
	 * @param run Run-level metadata.
	 * @param Y0  Initial state snapshot (read-only).
	 * @param ctx Driver context (read-only).
	 */
	virtual void OnStart(const RunInfo &run,
						 const Evolution::StateVector &Y0,
						 const Evolution::DriverContext &ctx);

	/**
	 * @brief Called whenever the evolution loop decides to record a sample.
	 *
	 * This is the primary callback for:
	 *  - time-series recording,
	 *  - checkpoints (if checkpointing on dt_save),
	 *  - diagnostics snapshots,
	 *  - lightweight logging.
	 *
	 * @param s   Sample metadata.
	 * @param Y   Current state snapshot (read-only).
	 * @param ctx Driver context (read-only).
	 */
	virtual void OnSample(const SampleInfo &s,
						  const Evolution::StateVector &Y,
						  const Evolution::DriverContext &ctx) = 0;

	/**
	 * @brief Called once after the integration ends (success or failure).
	 *
	 * Use this for:
	 *  - flushing/closing files,
	 *  - writing final metadata,
	 *  - emitting summary lines.
	 *
	 * @param fin Finalization info (ok flag, final time, optional message).
	 * @param Yf  Final state snapshot (read-only).
	 * @param ctx Driver context (read-only).
	 */
	virtual void OnFinish(const FinishInfo &fin,
						  const Evolution::StateVector &Yf,
						  const Evolution::DriverContext &ctx);

	/**
	 * @brief Optional human-readable observer name for logs.
	 */
	[[nodiscard]] virtual std::string Name() const;
};

} // namespace CompactStar::Physics::Evolution::Observers