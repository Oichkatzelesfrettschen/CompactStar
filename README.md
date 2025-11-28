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
├── CMakeLists.txt
├── Core/
│   ├── CMakeLists.txt
│   ├── Analysis.hpp
│   ├── Banner.hpp
│   ├── CompactStarConfig.h
│   ├── CompactStarConfig 2.h
│   ├── MixedStar.hpp
│   ├── NStar.hpp
│   ├── Prog.hpp
│   ├── Pulsar.hpp
│   ├── Pulsar_old.hpp
│   ├── RotationSolver.hpp
│   ├── SeqPoint.hpp
│   ├── StarBuilder.hpp
│   ├── StarProfile.hpp
│   ├── TaskManager.hpp
│   ├── TOVSolver.hpp
│   ├── TOVSolver_Thread.hpp
│   └── src/
│       ├── Analysis.cpp
│       ├── Banner.cpp
│       ├── MixedStar.cpp
│       ├── NStar.cpp
│       ├── Prog.cpp
│       ├── Pulsar.cpp
│       ├── Pulsar_old.cpp
│       ├── RotationSolver.cpp
│       ├── StarBuilder.cpp
│       ├── StarProfile.cpp
│       ├── TaskManager.cpp
│       ├── TOVSolver.cpp
│       └── TOVSolver_Old.cpp
├── EOS/
│   ├── CMakeLists.txt
│   ├── Baryon.hpp
│   ├── Common.hpp
│   ├── CompOSE_EOS.hpp
│   ├── CoulombLattice.hpp
│   ├── Fermi_Gas.hpp
│   ├── Fermi_Gas_Many.hpp
│   ├── Lepton.hpp
│   ├── Model.hpp
│   ├── Particle.hpp
│   ├── Polytrope.hpp
│   ├── SigmaOmega.hpp
│   ├── SigmaOmegaPar.hpp
│   ├── SigmaOmegaRho.hpp
│   ├── SigmaOmegaRho_nstar.hpp
│   └── src/
│       ├── Baryon.cpp
│       ├── Common.cpp
│       ├── CompOSE_EOS.cpp
│       ├── CoulombLattice.cpp
│       ├── Fermi_Gas.cpp
│       ├── Fermi_Gas_Many.cpp
│       ├── Lepton.cpp
│       ├── Model.cpp
│       ├── Particle.cpp
│       ├── Polytrope.cpp
│       ├── SigmaOmega.cpp
│       ├── SigmaOmegaPar.cpp
│       ├── SigmaOmegaRho.cpp
│       └── SigmaOmegaRho_nstar.cpp
├── Extensions/
│   ├── CMakeLists.txt
│   ├── LightDM/
│   │   ├── CMakeLists.txt
│   │   ├── LightDM_Scalar_Density.hpp
│   │   └── src/
│   │       └── LightDM_Scalar_Density.cpp
│   └── MixedStar/
│       ├── CMakeLists.txt
│       ├── DarkCore_Analysis.hpp
│       └── src/
│           └── DarkCore_Analysis.cpp
├── Microphysics/
│   ├── CMakeLists.txt
│   ├── BNV/
│   │   ├── CMakeLists.txt
│   │   ├── Analysis/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── BNV_Analysis.hpp
│   │   │   ├── BNV_Sequence.hpp
│   │   │   ├── Decay_Analysis.hpp
│   │   │   └── src/
│   │   │       ├── BNV_Analysis.cpp
│   │   │       ├── BNV_Sequence.cpp
│   │   │       └── Decay_Analysis.cpp
│   │   ├── Channels/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── BNV_B_Chi_Combo.hpp
│   │   │   ├── BNV_B_Chi_Photon.hpp
│   │   │   ├── BNV_B_Chi_Transition.hpp
│   │   │   ├── BNV_B_Psi_Pion.hpp
│   │   │   └── src/
│   │   │       ├── BNV_B_Chi_Combo.cpp
│   │   │       ├── BNV_B_Chi_Photon.cpp
│   │   │       ├── BNV_B_Chi_Transition.cpp
│   │   │       └── BNV_B_Psi_Pion.cpp
│   │   └── Internal/
│   │       ├── CMakeLists.txt
│   │       ├── BNV_Chi.hpp
│   │       └── src/
│   │           └── BNV_Chi.cpp
│   └── Rates/
│       ├── CMakeLists.txt
│       └── Urca.hpp
├── Physics/
│   ├── CMakeLists.txt
│   ├── BNV.hpp
│   ├── Thermal.hpp
│   ├── Driver/
│   │   ├── CMakeLists.txt
│   │   ├── Coupling.hpp
│   │   ├── IDriver.hpp
│   │   ├── Chem/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── BNVSource.hpp
│   │   │   ├── Rotochemical.hpp
│   │   │   └── WeakRestoration.hpp
│   │   ├── Spin/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── AccretionTorque.hpp
│   │   │   ├── BNVSpinTorque.hpp
│   │   │   └── MagneticDipole.hpp
│   │   └── Thermal/
│   │       ├── CMakeLists.txt
│   │       ├── HeatingFromChem.hpp
│   │       ├── NeutrinoCooling.hpp
│   │       └── PhotonCooling.hpp
│   ├── Evolution/
│   │   ├── CMakeLists.txt
│   │   ├── GeometryCache.hpp
│   │   ├── Graph.hpp
│   │   ├── StarContext.hpp
│   │   ├── System.hpp
│   │   ├── Integrator/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── GSLIntegrator.hpp
│   │   │   └── src/
│   │   │       └── GSLIntegrator.cpp
│   │   ├── Observers/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── CheckpointObserver.hpp
│   │   │   ├── IObserver.hpp
│   │   │   └── LogObserver.hpp
│   │   └── src/
│   │       ├── GeometryCache.cpp
│   │       ├── StarContext.cpp
│   │       └── System.cpp
│   └── State/
│       ├── CMakeLists.txt
│       ├── BNVState.hpp
│       ├── ChemState.hpp
│       ├── SpinState.hpp
│       └── ThermalState.hpp

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
