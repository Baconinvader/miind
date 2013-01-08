// Copyright (c) 2005 - 2011 Marc de Kamps
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
#include <cassert>
#include <NumtoolsLib/NumtoolsLib.h>
#include <MPILib/include/populist/circulantSolvers/AbstractCirculantSolver.hpp>
#include <MPILib/include/populist/rateComputation/AbstractRateComputation.hpp>
#include <MPILib/include/populist/circulantSolvers/CirculantSolver.hpp>
#include <MPILib/include/populist/zeroLeakEquations/LIFZeroLeakEquations.hpp>
#include <MPILib/include/populist/nonCirculantSolvers/NonCirculantSolver.hpp>
using namespace MPILib;
using namespace populist;
using namespace zeroLeakEquations;

using NumtoolsLib::IsApproximatelyEqualTo;
LIFZeroLeakEquations::LIFZeroLeakEquations
(
	Number&								n_bins,		
	valarray<Potential>&				array_state,
	Potential&							check_sum,
	SpecialBins&						bins,		
	parameters::PopulationParameter&				par_pop,		//!< reference to the PopulationParameter (TODO: is this necessary?)
	parameters::PopulistSpecificParameter&			par_spec,		//!< reference to the PopulistSpecificParameter
	Potential&							delta_v,		//!< reference to the current scale variable
	const circulantSolvers::AbstractCirculantSolver&		circ,
	const nonCirculantSolvers::AbstractNonCirculantSolver&	noncirc
):
AbstractZeroLeakEquations
(
	n_bins,
	array_state,
	check_sum,
	bins,
	par_pop,
	par_spec,
	delta_v
),
_time_current(0),
_p_n_bins(&n_bins),
_p_array_state(&array_state),
_p_check_sum(&check_sum),
_convertor
(
	bins,
	par_pop,
	par_spec,
	delta_v,
	n_bins
),
_p_solver_circulant(circ.clone()),
_p_solver_non_circulant(noncirc.Clone())
{
}

void LIFZeroLeakEquations::Configure
(
	void*					p_par					// irrelevant for LIFZeroLeakequations
)
{
	_convertor.Configure(this->ArrayState());

	const parameters::InputParameterSet& input_set =  _convertor.SolverParameter()[0];

	_p_solver_circulant->Configure
	(
		_p_array_state,
		input_set
	);


	_p_solver_non_circulant->Configure
	(
		*_p_array_state,
		input_set
	);

	_p_rate_calc = std::unique_ptr<rateComputation::AbstractRateComputation>(this->ParSpec().getRateComputation().Clone());

	_p_rate_calc->Configure
	(
		*_p_array_state,
		_convertor.getSolverParameter(),
		_convertor.getParPop(), 
		_convertor.getIndexReversalBin()
	);
}

Rate LIFZeroLeakEquations::CalculateRate() const
{
	return _p_rate_calc->CalculateRate(*_p_n_bins);
}

void LIFZeroLeakEquations::RecalculateSolverParameters()
{
	_convertor.RecalculateSolverParameters();
}

void LIFZeroLeakEquations::SortConnectionvector
(
        const std::vector<Rate>& nodeVector,
        const std::vector<OrnsteinUhlenbeckConnection>& weightVector,
	const std::vector<NodeType>& typeVector
)
{
        _convertor.SortConnectionvector(nodeVector,weightVector,typeVector);
}

void LIFZeroLeakEquations::AdaptParameters
(
)
{
	_convertor.AdaptParameters();
}
