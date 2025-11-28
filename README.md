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

# Project Structure

```
CompactStar/
├── Core/                           # Math, physics primitives, TOV/Hartle solvers, star builders
│   ├── src/                        # Implementations for Core classes
│   └── (various headers)           # NStar, Pulsar, RotationSolver, StarBuilder, etc.
│
├── EOS/                            # Equation of state models and infrastructure
│   └── src/                        # EOS implementations (CompOSE, Fermi gas, sigma-omega models)
│
├── Physics/                        # Full evolution engine (thermal + spin + chemical + BNV coupling)
│   ├── Driver/                     # Thermal, chemical, and spin drivers (pluggable modules)
│   │   ├── Chem/                   # Rotochemical, weak restoration, BNV chemical sources
│   │   ├── Spin/                   # Spin-down, accretion, BNV torque models
│   │   └── Thermal/                # Cooling/heating channels
│   ├── Evolution/                  # Integrators, observers, system assembly
│   │   ├── Integrator/             # GSL integrator bindings
│   │   └── Observers/              # Logging, checkpointing, diagnostics
│   └── State/                      # ThermalState, SpinState, ChemState, BNVState
│
├── Microphysics/                   # Reaction-level particle physics (BNV, Urca, etc.)
│   ├── BNV/                        # Baryon-number-violating models
│   │   ├── Analysis/               # BNV diagnostic tools, sequences, decay analysis
│   │   ├── Channels/               # BNV reaction channels
│   │   └── Internal/               # BNV particle/field definitions
│   └── Rates/                      # Urca and related microphysical rates
│
├── Extensions/                     # Optional add-ons beyond the core framework
│   ├── LightDM/                    # Light dark-matter scalar density models
│   └── MixedStar/                  # Dark–visible mixed star analysis extensions
│
├── CMakeLists.txt                  # Build system
└── (other top-level support files)
```

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
