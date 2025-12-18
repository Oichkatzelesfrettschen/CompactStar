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
 * @file DiagnosticPacket.hpp
 * @brief Runtime container for a single diagnostics “snapshot”.
 *
 * A DiagnosticPacket is a light-weight structure that stores:
 *  - metadata about where/when the snapshot was created (time, step, driver name),
 *  - a stable list of scalar values (key -> number + optional unit + description),
 *  - optional text blocks (contracts, warnings, notes).
 *
 * It is intended for:
 *  - terminal-friendly structured printing (via custom renderers),
 *  - JSON/JSONL export for tooling and regression tests.
 *
 * Notes on design:
 *  - This module intentionally stores *only doubles* as numeric payloads.
 *  - Unit strings are metadata only (no unit arithmetic); they support auditing.
 *  - Storage uses an ordered map (std::map) for deterministic output ordering.
 */

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace CompactStar::Physics::Evolution::Diagnostics
{

/**
 * @brief Cadence hint for recording this scalar.
 * - Always: write every sample.
 * - OnChange: write only if the value changed (within tolerance).
 * - OncePerRun: write once (step==0) and then never again.
 */
enum class Cadence
{
	Always,	   // write every diagnostic sample
	OnChange,  // write only if value changed (within tolerance)
	OncePerRun // write once (step==0) and then never again
};

/**
 * @brief Represents one scalar diagnostic entry.
 *
 * Keys should be stable and tooling-friendly (snake_case recommended).
 * The optional unit is a human/tool hint, not a type system.
 */
struct ScalarEntry
{
	/// Numeric value of the field.
	double value = 0.0;

	/// Optional unit string (e.g., "K", "erg/s", "cm^2", "rad/s").
	std::string unit;

	/// Optional one-line human description.
	std::string description;

	/// Optional “source” label (e.g., "computed", "input", "cache", "fallback").
	std::string source;

	/// True if the numeric value is finite (filled by caller or checked by validators).
	bool is_finite = true;

	/// Cadence hint for recording this scalar.
	Cadence cadence = Cadence::Always;
};

/**
 * @brief Single diagnostics snapshot with metadata and scalar fields.
 *
 * Typical usage:
 * @code
 * DiagnosticPacket pkt("PhotonCooling");
 * pkt.SetTime(t);
 * pkt.AddScalar("Tinf_K", Tinf, "K", "Redshifted internal temperature", "state");
 * pkt.AddScalar("A_inf_cm2", Ainf, "cm^2", "Redshifted area at infinity", "computed");
 * @endcode
 */
class DiagnosticPacket
{
  public:
	/**
	 * @brief Construct an empty packet.
	 * @param producer A short label for the producer (often a driver name).
	 */
	explicit DiagnosticPacket(std::string producer = "");

	/// @name Metadata setters
	/// @{

	/// Set the simulation time associated with this packet.
	void SetTime(double t);

	/// Set an integer step index (optional).
	void SetStepIndex(std::size_t step);

	/// Set a run identifier (optional; useful for multi-run aggregations).
	void SetRunId(std::string run_id);

	/// Override producer label (driver name, observer name, etc.).
	void SetProducer(std::string producer);

	/// @}

	/// @name Metadata getters
	/// @{

	[[nodiscard]] double Time() const { return time_; }
	[[nodiscard]] std::size_t StepIndex() const { return step_index_; }
	[[nodiscard]] const std::string &RunId() const { return run_id_; }
	[[nodiscard]] const std::string &Producer() const { return producer_; }

	/// @}

	/// @name Scalar fields
	/// @{

	/**
	 * @brief Add/replace a scalar.
	 *
	 * If the key already exists, it is overwritten (deterministic).
	 *
	 * @param key Stable key name (tooling-friendly).
	 * @param value Numeric value.
	 * @param unit Optional unit string.
	 * @param description Optional one-line description.
	 * @param source Optional provenance label (e.g., "computed", "state").
	 */
	void AddScalar(const std::string &key,
				   double value,
				   std::string unit = "",
				   std::string description = "",
				   std::string source = "");

	/**
	 * @brief Add/replace a scalar with cadence hint.
	 * If the key already exists, it is overwritten (deterministic).
	 * @param key Stable key name (tooling-friendly).
	 * @param value Numeric value.
	 * @param unit Optional unit string.
	 * @param description Optional one-line description.
	 * @param source Optional provenance label (e.g., "computed", "state").
	 * @param cadence Cadence hint for recording this scalar.
	 */
	void AddScalar(const std::string &key,
				   double value,
				   std::string unit,
				   std::string description,
				   std::string source,
				   Cadence cadence);

	/**
	 * @brief Returns true if the scalar exists.
	 */
	[[nodiscard]] bool HasScalar(const std::string &key) const;

	/**
	 * @brief Get a scalar entry by key.
	 * @throws std::out_of_range if the key does not exist.
	 */
	[[nodiscard]] const ScalarEntry &GetScalar(const std::string &key) const;

	/**
	 * @brief Return ordered scalar map (deterministic iteration order).
	 */
	[[nodiscard]] const std::map<std::string, ScalarEntry> &Scalars() const { return scalars_; }

	/// Remove all scalar entries.
	void ClearScalars();

	/// @}

	/// @name Text blocks (contracts / warnings / notes)
	/// @{

	/// Add a “contract” line (unit assumptions, conventions).
	void AddContractLine(std::string line);

	/// Add a warning (non-fatal diagnostic).
	void AddWarning(std::string line);

	/// Add an error (indicates diagnostic inconsistency).
	void AddError(std::string line);

	/// Add a note (freeform info).
	void AddNote(std::string line);

	[[nodiscard]] const std::vector<std::string> &ContractLines() const { return contract_lines_; }
	[[nodiscard]] const std::vector<std::string> &Warnings() const { return warnings_; }
	[[nodiscard]] const std::vector<std::string> &Errors() const { return errors_; }
	[[nodiscard]] const std::vector<std::string> &Notes() const { return notes_; }

	void ClearTextBlocks();

	/// @}

	/**
	 * @brief Run basic validations.
	 *
	 * This does not throw; it appends errors/warnings into the packet.
	 * Typical checks:
	 *  - non-finite values,
	 *  - empty producer (optional),
	 *  - reserved keys (optional).
	 */
	void ValidateBasic();

  private:
	double time_ = 0.0;
	std::size_t step_index_ = 0;

	std::string run_id_;
	std::string producer_;

	std::map<std::string, ScalarEntry> scalars_;

	std::vector<std::string> contract_lines_;
	std::vector<std::string> warnings_;
	std::vector<std::string> errors_;
	std::vector<std::string> notes_;
};

} // namespace CompactStar::Physics::Evolution::Diagnostics