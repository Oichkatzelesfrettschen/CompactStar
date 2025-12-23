# CompactStar

**CompactStar** is a high-performance, modular C++ framework for modeling the microphysics, structure, and evolution of compact stars—neutron stars, hybrid stars, and dark–visible admixed configurations.

Originally designed for dense-matter astrophysics research, the codebase has expanded into a full computational platform with:

- relativistic stellar structure solvers (TOV + Hartle),
- thermal + chemical evolution engines,
- baryon-number–violating (BNV) processes and exotic heating channels,
- spin evolution,
- sequences, contours, and bulk EOS analysis,
- a modernized Core module providing math, physics, datasets, file I/O, and analysis utilities.

CompactStar is written in **C++17**, designed for **flexibility, reproducibility, and extensibility**, and integrates cleanly into HPC workflows.

---

# Design Philosophy

CompactStar is built around the following principles:

- **Physics-first abstractions**: physical subsystems (spin, thermal, chemical, BNV)
  are modeled explicitly and evolve independently via tagged state blocks.
- **Strict separation of concerns**:
  - Core: structure, EOS, geometry, datasets
  - Physics/State: degrees of freedom
  - Physics/Driver: RHS contributions
  - Evolution: integration, diagnostics, orchestration
- **Schema-driven diagnostics**: diagnostics are self-describing, versioned,
  and machine-readable.
- **No hidden global state**: all evolution is driven through explicit context objects.

---

# Drivers vs Microphysics

CompactStar distinguishes between:

- **Microphysics**:

  - reaction rates
  - matrix elements
  - particle models
  - EOS-level inputs

- **Drivers**:
  - convert microphysics into time derivatives
  - operate on State blocks
  - are stateless apart from configuration
  - are pluggable and composable

Microphysics never modifies state directly.
All time evolution enters the system exclusively through Drivers.

---

# Extending CompactStar

To add a new physical process:

1. Define a new State block if new DOFs are required
2. Implement a Driver that:
   - declares its StateTag dependencies
   - contributes to RHS accumulation
   - optionally exposes diagnostics
3. Register the driver with the EvolutionSystem
4. (Optional) add diagnostics catalog entries

## No changes to the integrator or solver core are required.

# Core Capabilities

### Stellar Structure

- **TOVSolver**: static Tolman–Oppenheimer–Volkoff integration for any tabulated or analytical EOS.
- **TOVSolver_Thread**: multithreaded parallel wrapper for generating full sequences.
- **HartleSolver**: slow-rotation solver for frame-dragging and O(Ω²) structure perturbations.
- Computes mass–radius curves, moment of inertia, compactness, redshift, and structural profiles.

### Equations of State

- Supports tabulated, analytical, and hybrid/multiphase EOS formats.
- Built-in EOS readers, interpolation, thermodynamic derivatives.
- Clean interfaces for adding microphysical modules (effective masses, symmetry energy, thresholds).

### Thermal & Chemical Evolution

- Fully implements the Reisenegger–Fernández rotochemical heating formalism.
- Time evolution of:
  - internal temperature T∞,
  - chemical imbalances η,
  - neutrino luminosities,
  - heating vs cooling balance,
  - photon output.

### BNV Heating (Baryon Number Violation)

- Dedicated BNVState and reaction-rate infrastructure.
- User-defined particle-physics models plug in cleanly via the Driver abstraction.
- Supports energy injection, matter depletion, and thermal feedback.

### Spin Evolution

- Magnetic-dipole braking, custom torque laws, and coupling to chemical/thermal evolution.
- Integrates naturally with Hartle structure corrections.

### Sequences, Contours & Analysis

- Sequence generator for:
  - mass–radius,
  - I(M), M(R),
  - baryon-mass contours,
  - Kepler frequency estimates.
- Contour and profile exporter (csv, json, dataset format).
- Tools for generating stability curves and parameter-space scans.

### Core Mathematics, Physics, and Datasets

Provided by the new Core module:

- Custom Vector, Dataset, Profile, and Table types.
- Numerical methods (ODE solvers, root finders, bracketing, interpolation).
- Physics helpers (Fermi momenta, chemical potentials, redshift relations).
- ZLOG-based logging system.
- File I/O utilities for exporting star profiles, evolution histories, and grids.

---

# Typical Workflow

1. Build a stellar profile using Core (e.g. `NStar::SolveTOV_Profile`)
2. Construct `StarContext` and `GeometryCache`
3. Define evolved state blocks (ThermalState, SpinState, etc.)
4. Register state blocks and packing order
5. Attach physics drivers (cooling, heating, torques, …)
6. Assemble `EvolutionSystem`
7. Attach observers (Diagnostics, TimeSeries)
8. Integrate forward in time

---

# Diagnostics and Provenance

CompactStar treats diagnostics as first-class, schema-defined data products.

Every physical quantity emitted during evolution is:

- explicitly named,
- unit-labeled,
- source-typed (`state`, `computed`, `cache`),
- cadence-controlled (always, on change, once per run),
- versioned via a diagnostics catalog.

Diagnostics are emitted in three complementary forms:

- **JSONL packets**  
  Full-fidelity diagnostic records, suitable for post-processing, validation,
  and long-term archival.

- **Schema catalogs (JSON)**  
  Machine-readable descriptions of all diagnostics, including units,
  descriptions, and provenance. These act as a contract between physics
  drivers and observers.

- **Compact time-series tables (CSV/TSV)**  
  Plot-friendly, schema-driven tables derived directly from the catalog,
  guaranteeing consistency between raw diagnostics and reduced outputs.

This design ensures:

- reproducibility across runs and code versions,
- safe post-processing without hard-coded column assumptions,
- clear provenance for every reported physical quantity.

---

# Project Structure

```
CompactStar/
├── CMakeLists.txt                  # Top-level build
│
├── Core/                           # Star construction + stellar structure solvers + core utilities
│   ├── CMakeLists.txt
│   ├── src/                        # Implementations for Core classes
│   └── (headers)                   # NStar, Pulsar, MixedStar, TOVSolver(+Thread), RotationSolver,
│                                   # StarBuilder, StarProfile, Analysis, TaskManager, etc.
│
├── EOS/                            # Equation-of-state models and infrastructure
│   ├── CMakeLists.txt
│   ├── src/                        # EOS implementations (CompOSE, Fermi gas, sigma-omega(-rho), polytropes, etc.)
│   └── (headers)                   # Baryon/Lepton/Particle/Model, CoulombLattice, CompOSE_EOS, SigmaOmega*, ...
│
├── Microphysics/                   # Reaction-level particle/nuclear microphysics
│   ├── CMakeLists.txt
│   ├── Rates/                      # Shared microphysical rate tables / parametrizations (e.g., Urca.hpp)
│   └── BNV/                        # Baryon-number-violating microphysics package
│       ├── CMakeLists.txt
│       ├── Analysis/               # BNV diagnostic tools, sequences, decay analysis
│       │   ├── CMakeLists.txt
│       │   └── src/
│       ├── Channels/               # Concrete BNV reaction channels
│       │   ├── CMakeLists.txt
│       │   └── src/
│       └── Internal/               # Internal BNV particle/field definitions
│           ├── CMakeLists.txt
│           └── src/
│
├── Physics/                        # Evolution engine (state + drivers + integrators + diagnostics)
│   ├── CMakeLists.txt
│   ├── State/                      # Evolved state blocks and tags
│   │   ├── CMakeLists.txt
│   │   ├── src/
│   │   └── (headers)               # ThermalState, SpinState, ChemState, BNVState, Tags, base State.hpp
│   │
│   ├── Driver/                     # Pluggable RHS “drivers” (thermal/spin/chem) + coupling + diagnostics interfaces
│   │   ├── CMakeLists.txt
│   │   ├── IDriver.hpp
│   │   ├── Coupling.hpp
│   │   ├── Diagnostics/            # Driver diagnostics interface (IDriverDiagnostics)
│   │   │   └── DriverDiagnostics.hpp
│   │   ├── Thermal/                # Photon/Neutrino cooling, heating from chemistry, etc.
│   │   │   ├── CMakeLists.txt
│   │   │   ├── src/
│   │   │   └── (headers)           # PhotonCooling(+Details), NeutrinoCooling(+Details), HeatingFromChem, ...
│   │   ├── Spin/                   # Spin evolution drivers (e.g., MagneticDipole, AccretionTorque, BNV torques)
│   │   │   ├── CMakeLists.txt
│   │   │   ├── src/
│   │   │   └── (headers)
│   │   └── Chem/                   # Chemical evolution drivers (rotochemical, weak restoration, BNV sources)
│   │       ├── CMakeLists.txt
│   │       └── (headers)
│   │
│   └── Evolution/                  # System assembly, integrators, observers, diagnostics, run helpers
│       ├── CMakeLists.txt
│       ├── src/                    # Evolution core implementations
│       ├── Integrator/             # Numerical integrators (GSL bindings)
│       │   ├── CMakeLists.txt
│       │   └── src/
│       ├── Observers/              # Diagnostics/time series/checkpoint observers
│       │   ├── CMakeLists.txt
│       │   └── src/
│       ├── Diagnostics/            # JSONL diagnostics packets + catalog schema/contracts
│       │   ├── CMakeLists.txt
│       │   └── src/
│       └── Run/                    # Run-level orchestration helpers (paths, builders, observer factories)
│           ├── CMakeLists.txt
│           ├── RunPaths.hpp
│           ├── RunBuilder.hpp
│           ├── RunObservers.hpp
│           └── src/
│
├── Extensions/                     # Optional add-ons beyond the core framework
│   ├── CMakeLists.txt
│   ├── LightDM/                    # Light dark-matter scalar density extension
│   │   ├── CMakeLists.txt
│   │   └── src/
│   └── MixedStar/                  # Dark–visible mixed star analysis extension(s)
│       ├── CMakeLists.txt
│       └── src/
│
└── (other top-level support files)
```

---

# State Interface Architecture

CompactStar’s evolution engine treats each physical subsystem  
(Spin, Thermal, Chem, BNV, …) as a **state block** derived from a single
abstract base class `State`. This base class defines a small, uniform
interface:

- `Size()`
- `Data()`
- `PackTo()`
- `UnpackFrom()`
- `Resize()`
- `Clear()`

but **it intentionally does not define storage**.  
Each derived state type owns its own internal representation.

---

## Why the base class does _not_ own `values_`

Different state types have very different physical and numerical needs:

- **SpinState** stores Ω-like variables in a simple vector
- **ThermalState** may evolve one or many temperature components
- **ChemState** evolves a vector of η chemical imbalances
- **BNVState** may evolve anything from 1 to many exotic parameters
- Future modules may require 2D grids, GPU buffers, or nested structures

If the base class forced a single storage container (e.g. `std::vector<double>`),
it would prevent advanced future models or force awkward workarounds.

Leaving storage fully up to the derived class gives:

- full flexibility for arbitrary physics models, present and future
- decoupling between “how data is stored” and “how evolution operates”
- identical external interface for all states
- zero-cost abstraction: the solver only sees contiguous segments via `Data()`

Every state remains free to choose **any internal representation**, as long as it
can supply a contiguous DOF view to the evolution engine.

---

## Packing and Unpacking: How Evolution Works

During time evolution, the framework assembles one global ODE vector:

```
y = [ Spin DOFs | Thermal DOFs | Chem DOFs | BNV DOFs | ... ]
```

Each state block implements:

- **PackTo(y, offset)**: copy internal DOFs into the global vector
- **UnpackFrom(y, offset)**: restore DOFs after the solver updates `y[]`

This enables:

- arbitrary ordering of state blocks
- custom block sizes
- clean separation between the physics model and GSL/Sundials interfaces
- modular drivers that specify dependencies via `StateTag`

# Numerical Considerations

- Evolution uses adaptive explicit Runge–Kutta (RKF45 via GSL)
- State blocks are packed contiguously for solver compatibility
- Stiff processes should be modeled carefully (implicit solvers may be added later)
- Conservation laws are enforced at the driver level, not globally

---

## Summary

This architecture:

- maximizes extensibility
- cleanly supports heterogeneous physics blocks
- keeps the evolution solver generic and future-proof
- avoids premature commitment to a fixed layout
- simplifies driver implementation

For additional detail, see the comments in  
`CompactStar/Physics/State/State.hpp`.

---

# Features

- Tabulated + analytical EOS support
- Relativistic structure (TOV + Hartle)
- Rotochemical heating
- BNV heating models
- Spin–thermal–chemical coupling
- Stellar sequences and contour tools
- High-level API for parameter scans
- Lightweight dependencies (C++17, GSL)

---

# Building

```
mkdir build
cd build
cmake ..
make -j
```

Optional flags:

- `-DUSE_OPENMP=ON`
- `-DUSE_PYTHON=ON`
- `-DCS_ENABLE_PROFILING=ON`

---

# Documentation

CompactStar uses **Doxygen** documentation.  
To build:

```
mkdir build && cd build
cmake .. -DBUILD_DOCS=ON
make docs
```

---

# Author

**Mohammadreza Zakeri**  
Assistant Professor of Physics  
Eastern Kentucky University  
📧 M.Zakeri@eku.edu

---

# Citation

If you use CompactStar in published research, please cite this repository and the author.
