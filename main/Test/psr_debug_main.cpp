#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "CompactStar/Core/Pulsar.hpp"		// Pulsar (sets mp, name, wrk_dir, FindProfile)
#include "CompactStar/Core/StarProfile.hpp" // For exporting/printing if needed
#include "Zaki/String/Directory.hpp"

// ---------------- CLI ----------------
struct CLI
{
	std::string model_name;				  // e.g. "DD2"
	double mass_Msun = 1.40;			  // target gravitational mass
	Zaki::String::Directory rel_dir = ""; // e.g. "main/Test/results/tov_debug"
	Zaki::String::Directory wrk_dir = ""; // base working directory
	std::string pulsar_name;			  // optional label for output file

	// Explicit default ctor so it's not deleted
	CLI() = default;

	static CLI parse_args(int argc, char **argv)
	{
		CLI cli; // fully-initialized local

		// Sensible defaults (adjust to your tree)
		cli.model_name = "DD2";
		cli.mass_Msun = 1.40;
		cli.rel_dir = Zaki::String::Directory("main/Test/results/tov_debug/");
		cli.wrk_dir = Zaki::String::Directory(""); // empty means paths in rel_dir are already absolute or relative to CWD
		cli.pulsar_name = "pulsar_debug";

		for (int i = 1; i < argc; ++i)
		{
			const std::string a = argv[i];
			auto need = [&](int j)
			{ if (j >= argc) throw std::runtime_error("Missing value after " + a); };

			if (a == "--model")
			{
				need(i + 1);
				cli.model_name = argv[++i];
			}
			else if (a == "--mass")
			{
				need(i + 1);
				cli.mass_Msun = std::strtod(argv[++i], nullptr);
			}
			else if (a == "--dir")
			{
				need(i + 1);
				cli.rel_dir = Zaki::String::Directory(argv[++i]);
			}
			else if (a == "--wrk")
			{
				need(i + 1);
				cli.wrk_dir = Zaki::String::Directory(argv[++i]);
			}
			else if (a == "--name")
			{
				need(i + 1);
				cli.pulsar_name = argv[++i];
			}
			else if (a == "-h" || a == "--help")
			{
				std::cout << "Usage: pulsar_debug_main "
							 "[--model DD2] [--mass 1.40] [--dir path/] [--wrk base/] [--name label]\n";
				std::exit(0);
			}
			else
			{
				throw std::runtime_error("Unknown argument: " + a);
			}
		}
		return cli;
	}
};

// --------------- MAIN ----------------
int main(int argc, char **argv)
{
	Zaki::String::Directory dir(__FILE__);

	std::string model = "tov_debug";

	CompactStar::Pulsar PSRJ1614m2230_908("J1614-2230_1.908_hp",
										  {1.908, 0.016},
										  {3.15, 0.0},
										  {0.96e-20, 0});

	PSRJ1614m2230_908.SetWrkDir(dir.ParentDir() +
								"/results/" +
								model);
	PSRJ1614m2230_908.FindProfile(model);

	// 	try
	// 	{
	// 		const CLI cli = CLI::parse_args(argc, argv);

	// 		// Construct Pulsar and set fields the new API relies on
	// 		CompactStar::Pulsar puls(cli.pulsar_name);
	// 		puls.SetWrkDir(cli.wrk_dir);	  // so StarBuilder sees the right base dir
	// 		puls.SetMass({cli.mass_Msun, 0}); // target mass used inside FindProfile
	// 		// (If you keep a public 'name', set it too; but you already pass model_name below.)

	// 		std::cout << "[debug] model=" << cli.model_name
	// 				  << " mass=" << cli.mass_Msun
	// 				  << " rel_dir=" << (cli.rel_dir.Str())
	// 				  << " wrk_dir=" << (cli.wrk_dir.Str()) << "\n";

	// 		// The IMPORTANT change: pass model name + directory (not mass)
	// 		int seq_idx = puls.FindProfile(cli.model_name, cli.rel_dir);

	// 		std::cout << "[ok] FindProfile returned seq_idx=" << seq_idx << "\n";

	// 		// Optionally, dump a few things to confirm it’s populated
	// 		// (Guard if your Pulsar exposes these; adapt as needed.)
	// 		const auto *prof = puls.GetProfile(); // if you have such an accessor
	// 		if (prof && !prof->empty())
	// 		{
	// 			std::cout << "  R_surf (km): " << prof->R << "\n";
	// 			std::cout << "  M_surf (Msun): " << prof->seq_point.m << "\n";
	// 			std::cout << "  ec (g/cm^3): " << prof->seq_point.ec << "\n";
	// 			std::cout << "  I (km^3): " << prof->seq_point.I << "\n";
	// 		}

	// 		return 0;
	// 	}
	// 	catch (const std::exception &e)
	// 	{
	// 		std::cerr << "[fatal] " << e.what() << "\n";
	// 		return 1;
	// 	}
}