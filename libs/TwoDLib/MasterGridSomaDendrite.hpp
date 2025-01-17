// Copyright (c) 2005 - 2015 Marc de Kamps
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#ifndef _CODE_LIBS_TWODLIB_MASTERGRIDSOMADENDRITE_INCLUDE_GUARD
#define _CODE_LIBS_TWODLIB_MASTERGRIDSOMADENDRITE_INCLUDE_GUARD

#include <string>
#include <map>
#include <boost/numeric/odeint.hpp>
#include "CSRMatrix.hpp"
#include "TransitionMatrix.hpp"
#include "Ode2DSystemGroup.hpp"
#include "MasterParameter.hpp"

namespace TwoDLib {

	//! OpenMP version of a forward Euler integration of the Master equation

	class MasterGridSomaDendrite {
	public:

		MasterGridSomaDendrite
		(
			Ode2DSystemGroup&,
			double
		);

		void MVGrid(
			vector<double>&       dydt,
			const vector<double>& vec_mass,
			double                rate,
			unsigned int          efficiacy_index) const;

		void InitializeEfficacyVectors(unsigned int size);
		void CalculateDynamicEfficiacies(vector<std::string>& conn_types, vector<double>& efficacy_map, vector<double>& rest_v, vector<double>& conductances);

		void Apply(double t_step, const vector<double>& rates);

		void operator()(const vector<double>&, vector<double>&, const double t = 0);

	private:

		MasterGridSomaDendrite& operator=(const MasterGridSomaDendrite&);

		Ode2DSystemGroup& _sys;

		double _cell_width;

		vector<double>			_dydt;
		vector<std::map<int, vector<double>>>		_stays;
		vector<std::map<int, vector<double>>>		_goes;

		const vector<double>* _p_vec_rates;
	};
}

#endif // include guard
