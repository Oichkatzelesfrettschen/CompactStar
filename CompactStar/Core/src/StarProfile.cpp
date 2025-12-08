/*
  StarProfile class
*/

#include <CompactStar/Core/StarProfile.hpp>
#include <chrono>

using namespace CompactStar::Core;
//------------------------------------------------------------
// 13) Export (in-place; no copying)
//------------------------------------------------------------
void StarProfile::Export(const Zaki::String::Directory &out_dir, int precision)
{
	// nothing to export
	if (radial.Dim().empty())
		return;

	if (precision < 0)
		precision = profile_precision;

	// build sequence header
	char seq_header[200];
	std::snprintf(seq_header, sizeof(seq_header),
				  "    %-14s\t %-14s\t %-14s\t %-14s\t %-14s\t %-14s",
				  "ec(g/cm^3)", "M(Sun)", "R(km)", "pc(dyne/cm^2)", "B", "I(km^3)");

	// timestamp
	std::time_t now = std::chrono::system_clock::to_time_t(
		std::chrono::system_clock::now());

	radial.AddHead("# --------------------------------------------------------"
				   "--------------------------------------------------------\n");
	radial.AddHead("# Profile generated on ");
	radial.AddHead(std::string(std::ctime(&now)));
	radial.AddHead("# --------------------------------------------------------"
				   "--------------------------------------------------------\n");
	radial.AddHead("# Sequence point info:\n");
	radial.AddHead("#         " + std::string(seq_header) + "\n");
	radial.AddHead("#         " + seq_point.Str() + "\n");
	radial.AddHead("# --------------------------------------------------------"
				   "--------------------------------------------------------\n");

	// column descriptions
	radial.AddHead("# Columns present in this file:\n");
	if (HasColumn(Column::Radius))
		radial.AddHead("#   r(km)\n");
	if (HasColumn(Column::Mass))
		radial.AddHead("#   m(km)\n"); // stored in km
	if (HasColumn(Column::MetricNuPrime))
		radial.AddHead("#   nu_prime(km^-1)\n");
	if (HasColumn(Column::Pressure))
		radial.AddHead("#   p(km^-2)\n");
	if (HasColumn(Column::EnergyDensity))
		radial.AddHead("#   eps(km^-2)\n");
	if (HasColumn(Column::BaryonDensity))
		radial.AddHead("#   nB(fm^-3)\n");
	if (HasColumn(Column::MetricNu))
		radial.AddHead("#   nu(r)\n");
	if (HasColumn(Column::MetricLambda))
		radial.AddHead("#   lambda(r)\n");

	// species
	if (!species_labels.empty())
	{
		radial.AddHead("# Species densities (following above columns):\n");
		for (std::size_t i = 0; i < species_labels.size(); ++i)
		{
			const int idx = species_idx[i];
			radial.AddHead("#   [" + std::to_string(idx) + "] " +
						   species_labels[i] + "(r)\n");
		}
	}

	radial.AddFoot("# --------------------------------------------------------"
				   "--------------------------------------------------------");

	radial.SetPrecision(precision);
	const std::string out_path =
		out_dir.ThisFileDir().Str() + "/" + out_dir.ThisFile().Str();
	radial.Export(out_path);

	radial.ClearHeadFoot();
}
//------------------------------------------------------------
