/*
  Analysis Abstract class
*/

#include "CompactStar/Core/Analysis.hpp"

using namespace CompactStar::Core;
//==============================================================
//                        Analysis class
//==============================================================

/// @brief Constructor for the Analysis class.
/// Initializes the base Prog class with the label "Analysis".
Analysis::Analysis()
	: Prog("Analysis")
{
}

//--------------------------------------------------------------

/// @brief Virtual destructor for the Analysis class.
Analysis::~Analysis() {}

//--------------------------------------------------------------

/// @brief Sets the label for the analysis.
/// @param in_label A string label used to tag or identify this analysis.
void Analysis::SetLabel(const std::string &in_label)
{
	label = in_label;
}

//--------------------------------------------------------------

//==============================================================