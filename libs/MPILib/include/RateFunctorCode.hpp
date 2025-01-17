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
//      If you use this software in work leading to a scientific publication, you should cite
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net

#ifndef _CODE_LIBS_MPILIB_RATEFUNCTORCODE_INCLUDE_GUARD
#define _CODE_LIBS_MPILIB_RATEFUNCTORCODE_INCLUDE_GUARD

#include "RateFunctor.hpp"

namespace MPILib {

	template <class WeightValue>
	RateFunctor<WeightValue>::RateFunctor(RateFunction function):
	AlgorithmInterface<WeightValue>(),
	_function(function),
	_current_time(0)
	{
	}

	template <class WeightValue>
	void RateFunctor<WeightValue>::configure( const SimulationRunParameter&){
	}

	template <class WeightValue>
	void RateFunctor<WeightValue>::evolveNodeState(const std::vector<MPILib::Rate>&,
			const std::vector<WeightValue>&, Time time)
	{
		_current_time = time; 
		_current_rate = _function(_current_time);
	}

	template <class WeightValue>
	AlgorithmGrid RateFunctor<WeightValue>::getGrid(NodeId, bool) const
	{
		std::vector<double> vector_grid(1,_function(_current_time));
		return AlgorithmGrid(vector_grid);
	}
	
	template <class WeightValue>
	MPILib::RateFunctor<WeightValue>* RateFunctor<WeightValue>::clone() const
	{
		return new RateFunctor<WeightValue>(*this);
	}

	template <class WeightValue>
	MPILib::Time RateFunctor<WeightValue>::getCurrentTime() const
	{
		return _current_time;
	}

	template <class WeightValue>
	MPILib::Rate RateFunctor<WeightValue>::getCurrentRate() const
	{
		return _current_rate;
	}

}
#endif // include guard
