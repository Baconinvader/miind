// Copyright (c) 2005 - 2009 Marc de Kamps
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


#ifndef MPILIB_ALGORITHMS_RATEFUNCTOR_HPP_
#define MPILIB_ALGORITHMS_RATEFUNCTOR_HPP_

#include <MPILib/include/AlgorithmInterface.hpp>
#include <MPILib/include/TypeDefinitions.hpp>

namespace MPILib {

	typedef MPILib::Rate (*RateFunction)(MPILib::Time);

	inline Rate Nul(Time){ return 0; }

	//! An Algorithm that encapsulates a rate as a function of time
	//!
	//! It is sometime necessary to provide a network with external inputs. These inputs are created as
	//! nodes themselves. Their state is trivial and their output firing rate, given by the
	//! getCurrentRate method follows a given function of time.

	template <class WeightValue>
	class RateFunctor : public AlgorithmInterface<WeightValue>{
	public:


		//! Constructor must be initialized with pointer a rate function of time.
		RateFunctor(RateFunction);

		//! mandatory virtual destructor
		virtual ~RateFunctor(){}

		/**
		 * Cloning operation, to provide each DynamicNode with its own
		 * Algorithm instance. Clients use the naked pointer at their own risk.
		 */
		virtual RateFunctor* clone() const;


		/**
		 * Configure the Algorithm
		 * @param simParam
		 */
		virtual void configure(const SimulationRunParameter& simParam);


		/**
		 * Evolve the node state, here a call is made to the encapsulated function that determines the input
		 * @param nodeVector Vector of the node States
		 * @param weightVector Vector of the weights of the nodes
		 * @param time Time point of the algorithm
		 */
		virtual void evolveNodeState(const std::vector<Rate>& nodeVector,
				const std::vector<WeightValue>& weightVector, Time time);


		/**
		 * The current timepoint
		 * @return The current time point
		 */
		virtual MPILib::Time getCurrentTime() const;

		/**
		 * The calculated rate of the node
		 * @return The rate of the node
		 */
		virtual MPILib::Rate getCurrentRate() const;


		virtual AlgorithmGrid getGrid(NodeId, bool b_state) const;


	private:

		RateFunction _function;
		MPILib::Time _current_time;
		MPILib::Rate _current_rate;

	}; // end of RateFunctor

	typedef RateFunctor<double> D_RateFunctor;

} // end of MPILib

#endif // include guard
