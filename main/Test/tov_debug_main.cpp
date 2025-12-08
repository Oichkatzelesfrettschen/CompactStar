// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * Debug / smoke-test main for the TOV solver.
 *
 * Goal:
 *  - make sure TOVSolver.hpp / TOVSolver.cpp actually link
 *  - exercise: ImportEOS -> Solve(...) -> ExportSequence(...)
 *  - touch both NS and (optionally) mixed-star paths
 *
 * This file is intentionally verbose and defensive: it prints
 * out what it’s doing so you can see where it fails (path,
 * EOS import, GSL spline, surface reached, export, ...).
 */

#include <gsl/gsl_errno.h>
#include <iostream>

// #include <Zaki/Math/
#include <Zaki/String/Directory.hpp>
#include <Zaki/Util/Logger.hpp>

#include <CompactStar/Core/TOVSolver.hpp>

//--------------------------------------------------------------
// GSL error handler (so GSL does not abort the program)
static void
my_gsl_error_handler(const char *reason,
					 const char *file,
					 int line,
					 int gsl_errno)
{
	std::fprintf(stderr,
				 "GSL ERROR: %s:%d: %s (%s)\n",
				 file, line, reason, gsl_strerror(gsl_errno));
	// keep going — this is a debug main
}
//--------------------------------------------------------------

//--------------------------------------------------------------
// Example condition: export every NS profile
static bool AlwaysExportNS(const CompactStar::Core::NStar &)
{
	return true;
}

// Example condition: export every mixed-star profile
static bool AlwaysExportMixed(const CompactStar::Core::MixedStar &)
{
	return true;
}
//--------------------------------------------------------------

int main()
{
	// 1) GSL error handler
	gsl_set_error_handler(&my_gsl_error_handler);

	// 2) Logging: make it a bit chatty for debugging
	Zaki::Util::LogManager::SetLogLevels(Zaki::Util::LogLevel::Info);

	// 3) Working directory based on this file
	//    dir      -> .../CompactStar/Core/main/
	//    dir.ParentDir() -> .../CompactStar/Core/
	Zaki::String::Directory dir(__FILE__);
	std::cout << "[debug] this file dir = " << dir << "\n";

	// You will need to adjust these two to your layout.
	// I’m following the same pattern you used in the BNV code.
	Zaki::String::Directory eos_root =
		dir.ParentDir().ParentDir() + "/EOS/CompOSE/";
	// Zaki::String::Directory eos_root = "EOS/CompOSE/";
	std::string eos_name = "DS(CMF)-1_with_crust"; // change to your actual table

	std::cout << "[debug] assuming EOS root = " << eos_root << "\n";
	std::cout << "[debug] assuming EOS name = " << eos_name << "\n";

	// 4) Create a TOV solver
	CompactStar::Core::TOVSolver tov;
	tov.AddNCondition(&AlwaysExportNS);
	tov.AddMixCondition(&AlwaysExportMixed);

	// Optional: make the profile TSVs more precise
	tov.SetProfilePrecision(12);
	// Optional: smaller max radius if you know you’re NS-only
	tov.SetMaxRadius(15); // 15 km

	// 5) Import EOS
	//    CASE A (visible only):
	try
	{
		tov.ImportEOS(eos_root + eos_name + "/" + eos_name + ".eos");
		std::cout << "[debug] visible EOS imported.\n";
	}
	catch (...)
	{
		std::cerr << "[error] visible EOS import failed. "
				  << "Check path: " << (eos_root + eos_name) << "\n";
		return 1;
	}

	//    CASE B (visible + dark): uncomment if you want to test mixed
	/*
	try {
		tov.ImportEOS(eos_root + "DS(CMF)-1",
					  eos_root + "DarkEOS-1");
		std::cout << "[debug] visible+dark EOS imported.\n";
	} catch (...) {
		std::cerr << "[error] mixed EOS import failed.\n";
	}
	*/

	tov.SetWrkDir(dir.ParentDir() + "/results");

	// 6) Print EOS (so we know the table is sane)
	tov.PrintEOSSummary();
	tov.PrintEOSTable(5);

	// 7) Build a central-*pressure* (or energy-density) axis.
	//    Your TOV code’s Solve(...) signature is:
	//    void Solve(const Zaki::Math::Axis&, const Directory&, const Directory&);
	//
	//    Let’s just scan 40 points linearly between two plausible pressures.
	//    YOU may need to change these numbers to match your EOS range.
	Zaki::Math::Axis ec_axis({{1.0e+14, 1.913e15}, 20, "Log"});
	//                ^min     ^max     ^N   ^type
	// ε(energy density) : [ 1.658808e+08, 1.106871e+16 ];

	// 8) Solve the sequence
	//    Output directory: results/tov/
	//    Output file name: Sequence.tsv
	Zaki::String::Directory out_dir = "tov_debug/";
	Zaki::String::Directory out_file = "tov_debug";

	std::cout << "[debug] solving TOV for axis of size "
			  << ec_axis.res << " ...\n";

	try
	{
		tov.Solve(ec_axis, out_dir, out_file);
	}
	catch (const std::exception &ex)
	{
		std::cerr << "[error] TOV::Solve threw: " << ex.what() << "\n";
		return 1;
	}
	catch (...)
	{
		std::cerr << "[error] TOV::Solve threw an unknown exception.\n";
		return 1;
	}

	std::cout << "[debug] TOV solve finished.\n";

	// 9) Export the sequence again explicitly (the solver already does it,
	//    but we call it to check linkage)
	// tov.ExportSequence(out_dir);

	// std::cout << "[debug] sequence exported to: " << out_dir + out_file << "\n";

	// 10) Try the “test sequence” path — this exercises:
	//     - GetEDens(...)
	//     - varying radial resolution
	//     - export profile
	//
	// Pick one central energy density inside your EOS range.
	// This is ONLY for debugging.
	// double test_ec = 1.0e16;
	// try
	// {
	// 	tov.GenTestSequence(test_ec,
	// 						out_dir,
	// 						"GenTestSequence.tsv");
	// 	std::cout << "[debug] GenTestSequence(...) OK.\n";
	// }
	// catch (...)
	// {
	// 	std::cerr << "[warn] GenTestSequence(...) failed — ok for now.\n";
	// }

	std::cout << "[debug] done.\n";
	return 0;
}