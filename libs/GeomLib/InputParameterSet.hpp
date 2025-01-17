// Copyright (c) 2005 - 2014 Marc de Kamps
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
//      If you use this software in work leading to a scientific publication, you should include a reference there to
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net
#ifndef _CODE_LIBS_GEOMLIB_INPUTPARAMETERSET_INCLUDE_GUARD
#define _CODE_LIBS_GEOMLIB_INPUTPARAMETERSET_INCLUDE_GUARD

//#include "BasicDefinitions.hpp"
#include <MPILib/include/BasicDefinitions.hpp>
namespace GeomLib
{
	class InputParameterSet {
	public:

		int					_H_exc;				//! Excitatory efficacy in terms of bins. The real number of bins is rounded to the next integer, for historical reasons. To obtain the floating point value, add _alpha_exc. Do not use _h_exc instead because this is the efficacy in whatever units are given, usually mV or V and does not correspond to a number of bins.
		int					_H_inh;             //! Inhibitory efficacy in terms of bins. The real number of bins is rounded to the next integer, for historical reasons. To obtain the floating point value, add _alpha_exc. Do not use _h_inh instead because this is the efficacy in whatever units are given, usually mV or V and does not correspond to a number of bins.
		MPILib::Potential	_h_exc;				//! Synaptic efficacy in mV or V, do not interpret as a number of bins
		MPILib::Potential	_h_inh;				//! Synaptic efficacy in mV or V, do not interpret as a number of bins
		MPILib::Potential	_alpha_exc;			//! When the efficacy needs to be expressed as a number of bins, add this number to _H_exc for an accurate floating point value
		MPILib::Potential	_alpha_inh;			//! When the efficacy needs to be expressed as a number of bins, add this number to _H_inh for an accurate floating point value
		MPILib::Rate		_rate_exc;			//! Excitatory input rate driving the population, usually in spikes/s
		MPILib::Rate		_rate_inh;			//! Inhibitory input rate driving the population, usually in spikes/s
	};
}

#endif // include guard
