// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * MIT License — see LICENSE at repo root.
 */

#ifndef CompactStar_Banner_H
#define CompactStar_Banner_H

#include <mutex>

namespace CompactStar
{

/**
 * @file Banner.hpp
 * @brief Process-wide program banner utilities.
 *
 * Provides a one-shot banner display that runs automatically before
 * `main()` via a TU-scope static in Banner.cpp. You can also call
 * ShowBannerOnce() manually if desired (e.g., in tests).
 */

/**
 * @brief Print the CompactStar banner exactly once per process.
 *
 * Safe to call from any thread; subsequent calls are no-ops.
 * Normally you do not need to call this directly because
 * Banner.cpp defines a TU-scope static that triggers it during
 * static initialization (before `main()`).
 */
void ShowBannerOnce();

} // namespace CompactStar

#endif /* CompactStar_Banner_H */