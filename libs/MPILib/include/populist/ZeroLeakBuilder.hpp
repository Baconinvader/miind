// Copyright (c) 2005 - 2012 Marc de Kamps
//						2012 David-Matthias Sichau
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

#ifndef MPILIB_POPULIST_ZEROLEAKBUILDER_HPP_
#define MPILIB_POPULIST_ZEROLEAKBUILDER_HPP_

#include <string>
#include <boost/shared_ptr.hpp>
#include <MPILib/include/populist/AbstractCirculantSolver.hpp>
#include <MPILib/include/populist/AbstractNonCirculantSolver.hpp>
#include <MPILib/include/populist/zeroLeakEquations/AbstractZeroLeakEquations.hpp>

namespace MPILib {
namespace populist {

	class ZeroLeakBuilder {
	public:

		ZeroLeakBuilder(
			Number&,					//!< reference to the current number of bins
			std::valarray<Potential>&,		//!< reference to state array
			Potential&,					//!< reference to the check sum variable
			SpecialBins&,		
			parameters::PopulationParameter&,		//!< reference to the PopulationParameter
			parameters::PopulistSpecificParameter&,	//!< reference to the PopulistSpecificParameter
			Potential&					//!< reference to the current scale variable
		);

	boost::shared_ptr<AbstractZeroLeakEquations> 
		GenerateZeroLeakEquations
		(
			const std::string&,
			const std::string&,
			const std::string&
		);

	private:

		Number&						_n_bins;
		std::valarray<Potential>&	_array_state;
		Potential&					_checksum;
		SpecialBins&				_bins;
		parameters::PopulationParameter&		_par_pop;
		parameters::PopulistSpecificParameter&	_par_spec;
		Potential&					_delta_v;

	};
} /* namespace populist */
} /* namespace MPILib */

#endif // include guard MPILIB_POPULIST_ZEROLEAKBUILDER_HPP_
