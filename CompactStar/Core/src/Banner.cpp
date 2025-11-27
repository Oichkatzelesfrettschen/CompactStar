// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * MIT License — see LICENSE at repo root.
 */

#include "CompactStar/Core/Banner.hpp"
#include "CompactStar/Core/CompactStarConfig.h"

#include <Zaki/String/Banner.hpp>
#include <mutex>

namespace CompactStar
{

// ----- internal: single once_flag in this TU ---------------------------------
static std::once_flag g_banner_once;

/**
 * @brief Actual banner rendering body (non-exported).
 *
 * Contains the original banner composition using Zaki::String::Banner.
 * Kept non-virtual and with no external side effects beyond stdout.
 */
static void ShowBannerImpl()
{
	using namespace Zaki::String;

	Banner banner;

	ProgramName p_name("CompactStar", 1);
	banner.AddContent(&p_name);

	Author auth("Mohammadreza", "Zakeri", 4);
	banner.AddContent(&auth);

	Version ver(CompactStar_VERSION_STR, CompactStar_RELEASE_DATE, 2);
	banner.AddContent(&ver);

	Website web("GitHub", "github.com/ZAKI1905/CompactStar", 5);
	banner.AddContent(&web);

	banner.GetTextBox()->SetTextColor({FGColor::LCyan, BGColor::BlackBg});
	banner.GetTextBox()->SetFrameColor({FGColor::LYellow, BGColor::BlackBg});
	banner.GetTextBox()->SetPadColor({FGColor::LCyan, BGColor::BlackBg});
	banner.GetTextBox()->SetAlignment(TextBox::center);
	banner.GetTextBox()->SetPadding(5);

	// Clear screen for a clean banner display.
	banner.GetTextBox()->EnableClearScreen();

	banner.Show();
}

void ShowBannerOnce()
{
	std::call_once(g_banner_once, []
				   { ShowBannerImpl(); });
}

/**
 * @brief TU-scope static that triggers the banner before `main()`.
 *
 * One instance per process when this object file is linked once from
 * the core library. Guarded by `std::call_once` to avoid multi-print
 * under concurrency.
 */
struct BannerAuto
{
	BannerAuto() { ShowBannerOnce(); }
};

// One TU-local static — constructed during static initialization.
static BannerAuto g_banner_auto;

} // namespace CompactStar