// / class SeqPoint
// {
//   public:
// 	/** @brief Energy density (EC). */
// 	double ec;

// 	/** @brief Gravitational mass (M). */
// 	double m;

// 	/** @brief Radius (R). */
// 	double r;

// 	/** @brief Central pressure (PC). */
// 	double pc;

// 	/** @brief Baryon number integral (B). */
// 	double b;

// 	/** @brief Moment of inertia (I). */
// 	double I;

// 	/**
// 	 * @brief Default constructor, initializes all values to zero.
// 	 */
// 	SeqPoint() : ec(0), m(0), r(0), pc(0),
// 				 b(0), I(0) {};

// 	/**
// 	 * @brief Parameterized constructor.
// 	 *
// 	 * @param in_ec  Energy density.
// 	 * @param in_m   Mass.
// 	 * @param in_r   Radius.
// 	 * @param in_pc  Central pressure.
// 	 * @param in_b   Baryon number.
// 	 * @param in_I   Moment of inertia.
// 	 */
// 	SeqPoint(const double &in_ec, const double &in_m,
// 			 const double &in_r, const double &in_pc,
// 			 const double &in_b, const double &in_I)
// 		: ec(in_ec), m(in_m), r(in_r), pc(in_pc),
// 		  b(in_b), I(in_I)
// 	{
// 	}

// 	/**
// 	 * @brief Construct from a sequence row vector.
// 	 *
// 	 * Expects a vector of length 6: [ec, m, r, pc, b, I].
// 	 * Logs an error if size mismatch.
// 	 *
// 	 * @param in_seq_row  Input row vector.
// 	 */
// 	SeqPoint(const std::vector<double> &in_seq_row)
// 	{
// 		if (in_seq_row.size() != 6)
// 			Z_LOG_ERROR("The sequence data point is incomplete.");
// 		else
// 		{
// 			ec = in_seq_row[0];
// 			m = in_seq_row[1];
// 			r = in_seq_row[2];
// 			pc = in_seq_row[3];
// 			b = in_seq_row[4];
// 			I = in_seq_row[5];
// 		}
// 	}

// 	/**
// 	 * @brief Format the sequence point as a string.
// 	 *
// 	 * @return Tab-delimited string of values in scientific notation.
// 	 */
// 	std::string Str() const
// 	{
// 		std::stringstream ss;
// 		char tmp[150];
// 		snprintf(tmp, sizeof(tmp), "%.8e\t %.8e\t %.8e\t %.8e\t %.8e\t %.8e",
// 				 ec, m, r, pc, b, I);
// 		ss << tmp;

// 		return ss.str();
// 	}

// 	/**
// 	 * @brief Reset all sequence values to zero.
// 	 */
// 	void Reset()
// 	{
// 		ec = 0;
// 		m = 0;
// 		r = 0;
// 		pc = 0;
// 		b = 0;
// 		I = 0;
// 	}
// 	//..................................................

// 	/**
// 	 * @brief Addition operator.
// 	 *
// 	 * Adds corresponding fields of two SeqPoint objects.
// 	 *
// 	 * @param seq  Sequence point to add.
// 	 * @return     New SeqPoint representing the sum.
// 	 */
// 	SeqPoint operator+(const SeqPoint &seq) const
// 	{
// 		SeqPoint out_seq(ec + seq.ec, m + seq.m, r + seq.r,
// 						 pc + seq.pc, b + seq.b, I + seq.I);
// 		return out_seq;
// 	}

// 	/**
// 	 * @brief Scalar multiplication operator.
// 	 *
// 	 * Multiplies all fields by a scalar.
// 	 *
// 	 * @param num  Scalar multiplier.
// 	 * @return     New SeqPoint scaled by num.
// 	 */
// 	SeqPoint operator*(const double &num) const
// 	{
// 		SeqPoint out_seq(ec * num, m * num, r * num,
// 						 pc * num, b * num, I * num);
// 		return out_seq;
// 	}
// 	//..................................................
// };

// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @file SeqPoint.hpp
 * @brief Sequence point for compact-star M–R sequences and diagnostics.
 * @ingroup Core
 *
 * A lightweight POD-style container for a single model point along a stellar
 * sequence (e.g., fixed EOS, varying central density). Stores commonly used
 * scalars such as energy density, mass, radius, central pressure, baryon
 * number, and moment of inertia.
 *
 * @author Mohammadreza Zakeri
 * Contact: M.Zakeri@eku.edu
 */

#ifndef CompactStar_Core_SeqPoint_H
#define CompactStar_Core_SeqPoint_H

#include <cstdio> // snprintf
#include <sstream>
#include <string>
#include <vector>

// If you have a logging macro like Z_LOG_ERROR declared elsewhere, it will be used.
// Otherwise, this header remains agnostic and does not depend on it.

namespace CompactStar::Core
{

//==============================================================
//                        SeqPoint
//==============================================================
/**
 * @class SeqPoint
 * @brief Holds a single data point in a star sequence.
 * @ingroup Core
 *
 * ### Stored quantities (double):
 * - `ec` : central energy density (code units; often 1/km² or cgs—match your solver)
 * - `m`  : gravitational mass at the surface (M_\odot)
 * - `r`  : circumferential radius at the surface (km)
 * - `pc` : central pressure (same unit system as your solver output)
 * - `b`  : baryon number integral (dimensionless or code units, per your convention)
 * - `I`  : moment of inertia (e.g., g·cm² or km³, depending on internal units)
 *
 * This class is intentionally minimal and trivially copyable so it can be passed
 * by value in vectors/containers and written to text rows easily.
 */
class SeqPoint
{
  public:
	/// Number of scalar fields expected in a serialized row.
	static constexpr std::size_t kSize = 6;

	// ------------------------------------------------------------
	// Data members (public POD for convenience)
	// ------------------------------------------------------------

	double ec = 0.0; ///< Central energy density.
	double m = 0.0;	 ///< Gravitational mass (M_\odot).
	double r = 0.0;	 ///< Radius (km).
	double pc = 0.0; ///< Central pressure.
	double b = 0.0;	 ///< Baryon number integral.
	double I = 0.0;	 ///< Moment of inertia.

	// ------------------------------------------------------------
	// Construction
	// ------------------------------------------------------------

	/**
	 * @brief Default constructor (zero-initialize all fields).
	 */
	SeqPoint() = default;

	/**
	 * @brief Parameterized constructor.
	 * @param in_ec  Energy density (central).
	 * @param in_m   Gravitational mass (M_\odot).
	 * @param in_r   Radius (km).
	 * @param in_pc  Central pressure.
	 * @param in_b   Baryon number integral.
	 * @param in_I   Moment of inertia.
	 */
	SeqPoint(double in_ec, double in_m, double in_r,
			 double in_pc, double in_b, double in_I)
		: ec(in_ec), m(in_m), r(in_r), pc(in_pc), b(in_b), I(in_I) {}

	/**
	 * @brief Construct from a row vector `[ec, m, r, pc, b, I]`.
	 *
	 * If the input size is not exactly `kSize`, the object is zeroed.
	 * (If you want hard failure, replace the behavior with an assert or exception.)
	 *
	 * @param row Vector of length 6.
	 */
	explicit SeqPoint(const std::vector<double> &row)
	{
		if (row.size() == kSize)
		{
			ec = row[0];
			m = row[1];
			r = row[2];
			pc = row[3];
			b = row[4];
			I = row[5];
		}
		else
		{
			clear();
			// Optionally log here (e.g., Z_LOG_ERROR("SeqPoint row size != 6"));
		}
	}

	/**
	 * @brief Assignment from a row vector `[ec, m, r, pc, b, I]`.
	 *
	 * If the input size is not exactly `kSize`, the object is zeroed.
	 * (If you want hard failure, replace the behavior with an assert or exception.)
	 *
	 * @param row Vector of length 6.
	 * @return *this.
	 */
	SeqPoint &operator=(const std::vector<double> &row)
	{
		if (row.size() == kSize)
		{
			ec = row[0];
			m = row[1];
			r = row[2];
			pc = row[3];
			b = row[4];
			I = row[5];
		}
		else
		{
			clear();
		}
		return *this;
	}

	/**
	 * @brief Initializer-list constructor.
	 *
	 * If the input size is not exactly `kSize`, the object is zeroed.
	 * (If you want hard failure, replace the behavior with an assert or exception.)
	 *
	 * @param list Initializer list of length 6.
	 */
	SeqPoint(std::initializer_list<double> list)
	{
		if (list.size() == kSize)
		{
			auto it = list.begin();
			ec = *it++;
			m = *it++;
			r = *it++;
			pc = *it++;
			b = *it++;
			I = *it++;
		}
		else
		{
			clear();
		}
	}

	/**
	 * @brief Assignment from an initializer list.
	 *
	 * If the input size is not exactly `kSize`, the object is zeroed.
	 * (If you want hard failure, replace the behavior with an assert or exception.)
	 *
	 * @param list Initializer list of length 6.
	 * @return *this.
	 */
	SeqPoint &operator=(std::initializer_list<double> list)
	{
		if (list.size() == kSize)
		{
			auto it = list.begin();
			ec = *it++;
			m = *it++;
			r = *it++;
			pc = *it++;
			b = *it++;
			I = *it++;
		}
		else
		{
			clear();
		}
		return *this;
	}

	/**
	 * @brief Factory: construct from a row vector `[ec, m, r, pc, b, I]`.
	 * @param row Vector of length 6.
	 * @return A new SeqPoint (zeroed if size mismatch).
	 */
	[[nodiscard]] static SeqPoint FromRow(const std::vector<double> &row)
	{
		return SeqPoint(row);
	}

	// ------------------------------------------------------------
	// Utilities
	// ------------------------------------------------------------

	/**
	 * @brief Reset all fields to zero.
	 */
	void clear() { ec = m = r = pc = b = I = 0.0; }

	/**
	 * @brief Format fields as a single tab-delimited string.
	 * @return Tab-delimited scientific notation string.
	 *
	 * Order: `ec, m, r, pc, b, I`.
	 */
	[[nodiscard]] std::string Str() const
	{
		std::stringstream ss;
		char tmp[160];
		std::snprintf(tmp, sizeof(tmp),
					  "%.8e\t%.8e\t%.8e\t%.8e\t%.8e\t%.8e",
					  ec, m, r, pc, b, I);
		ss << tmp;
		return ss.str();
	}

	/**
	 * @brief Convert to a packed vector `[ec, m, r, pc, b, I]`.
	 * @return std::vector<double> with kSize entries.
	 */
	[[nodiscard]] std::vector<double> ToRow() const
	{
		return {ec, m, r, pc, b, I};
	}

	// ------------------------------------------------------------
	// Arithmetic (component-wise)
	// ------------------------------------------------------------

	/**
	 * @brief Component-wise addition.
	 * @param rhs Other sequence point.
	 * @return Sum of fields, element-wise.
	 */
	[[nodiscard]] SeqPoint operator+(const SeqPoint &rhs) const
	{
		return SeqPoint(ec + rhs.ec, m + rhs.m, r + rhs.r,
						pc + rhs.pc, b + rhs.b, I + rhs.I);
	}

	/**
	 * @brief Component-wise in-place addition.
	 * @param rhs Other sequence point.
	 * @return *this.
	 */
	SeqPoint &operator+=(const SeqPoint &rhs)
	{
		ec += rhs.ec;
		m += rhs.m;
		r += rhs.r;
		pc += rhs.pc;
		b += rhs.b;
		I += rhs.I;
		return *this;
	}

	/**
	 * @brief Scalar multiplication.
	 * @param s Scalar multiplier.
	 * @return Scaled sequence point.
	 */
	[[nodiscard]] SeqPoint operator*(double s) const
	{
		return SeqPoint(ec * s, m * s, r * s, pc * s, b * s, I * s);
	}

	/**
	 * @brief In-place scalar multiplication.
	 * @param s Scalar multiplier.
	 * @return *this.
	 */
	SeqPoint &operator*=(double s)
	{
		ec *= s;
		m *= s;
		r *= s;
		pc *= s;
		b *= s;
		I *= s;
		return *this;
	}
};

} // namespace CompactStar::Core

#endif /* CompactStar_Core_SeqPoint_H */