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
#ifndef _CODE_LIBS_POPULISTLIB_ABSTRACTODESYSTEM_INCLUDE_GUARD
#define _CODE_LIBS_POPULISTLIB_ABSTRACTODESYSTEM_INCLUDE_GUARD

#include <vector>
#include <boost/shared_ptr.hpp>
#include "AbstractNeuralDynamics.hpp"
#include "OdeParameter.hpp"

using std::vector;

namespace GeomLib {

	//! A geometric grid to represent population densities.

	//! Internally, GeomAlgorithm uses two arrays: one to represent probability mass and one one to represent
	//! the bin boundaries in membrane potential. We call this a system. In this base class: AbstractOdeSystem,
    //! both the buffer storing the probability mass and the array storing the bin boundaries are kept.
	//! Solvers for the density operate on the mass buffer: AbstractOdeSystem::_buffer_mass.
    //! The grid boundaries are stored in AbstractOdeSystem::_buffer_interpretation. The relationship between mass
	//! bins and grid boundaries is time dependent, AbstractOdeSytem keeps track of that.
    //! A one-to-one mapping identifies each mass bin with a
	//! a potential bin, thus providing a basis for a density representation. The mapping is time dependent, and
	//! depends on the topology of the neural system under consideration. Derived classes provide concrete
	//! implementations of these different topologies. LeakingOdeSystem implements the topology of
	//! leaky-integrate-and-fire neurons. SpikingOdeSystem implements spiking neural models, such as
	//! quadratic-integrate-and-fire neurons (leaky-integrate-and-fire neurons can be modeled as well by SpikingOdeSystem,
	//! using the current compensation device).

	class AbstractOdeSystem {
	public:

		//! Constructor using neural dynamics object (see AbstractNeuralDynamics and derived classes).
		AbstractOdeSystem
		(
			const AbstractNeuralDynamics& 		//! NeuralDynamics object
		);

		//! copy constructor
		AbstractOdeSystem(const AbstractOdeSystem& sys);

		//! pure virtual destructor for base class
		virtual ~AbstractOdeSystem() = 0;
		
		//! Every sub class defines its own evolution. This Evolve function does not relate to neural dynamics:
		//! that is controlled by AbstractNeuralDynamics objects. This function is a hook to call update
		//! the mapping between mass and potential bins.
		virtual void 
			Evolve
			(
				MPILib::Time
			) = 0;

		MPILib::Time TStep() const {return _t_step; }

		virtual AbstractOdeSystem* Clone() const  = 0;

		//! Access to the  OdeParameter of the system. It is often time-critical, therefore implemented as reference return.
		const OdeParameter& Par() const { return _par; }

		MPILib::Time CurrentTime() const { return _t_current; }

		//! Rate due to neural dynamics driving
		virtual MPILib::Rate CurrentRate() const = 0;

		virtual Potential DCContribution() const { return 0;}

		//! Number of bins used in the grid representation
	        MPILib::Number NumberOfBins() const {return _number_of_bins; }

		//! Access to the array of bin limits; be aware of the convention that the upper limit of the highest bin is
		//! not present in this array, and is equal to VMax, which can be obtained by Par()._nr_bins
		vector<MPILib::Potential>& InterpretationBuffer()             { return _buffer_interpretation; }

		//! Const access to the array of bin limits; be aware of the convention that the upper limit of the highest bin is
		//! not present in this array, and is equal to VMax, which can be obtained by Par()._nr_bins
		const vector<MPILib::Potential>& InterpretationBuffer() const { return _buffer_interpretation; }

		//! Direct access to the array that represents the density mass. Note that this may be confusing, and in general
		//! you do not want to use this information directly. For a density profile, use PrepareReport, which performs the appropriate preprocessing.
		//! a density profile use PrepareReport
		vector<MPILib::Potential>& MassBuffer()                       { return _buffer_mass; }

		//! Const direct access to the array that represents the density mass. Note that this may be confusing, and in general
		//! you do not want to use this information directly. For a density profile, use PrepareReport, which performs the appropriate preprocessing.
		//! a density profile use PrepareReport

		const vector<MPILib::Potential>& MassBuffer() const           { return _buffer_mass; }

		//! Index of the reset bin relative to the interpretation array, i.e. constant during simulation
	  MPILib::Index IndexResetBin() const {return _i_reset;}

		//! Find which bin in the interpretation array contains this potential.
	  MPILib::Index FindBin(Potential) const;

		//! Maintains the current mapping from a probability mass bin to its current poetntial bin in the interpretation array
	  MPILib::Index MapPotentialToProbabilityBin(MPILib::Index i) const { assert(i < _map_cache.size()); return _map_cache[i]; }

		//! Represents the current density profile. Both double pointers must point to contiguous memory at least NumberOfBins() large.
		//! After calling the first array will contain
		void PrepareReport
		(
			double*,
			double*
		) const;

	protected:

		boost::shared_ptr<AbstractNeuralDynamics> _p_dyn;
		const string		_name_namerical;
		MPILib::Time		_t_period;
		MPILib::Time 		_t_step;

		const OdeParameter& _par;
		vector<MPILib::Potential>	_buffer_interpretation;
		vector<MPILib::Density>		_buffer_mass;

	  MPILib::Index		       	_i_reset;
	  MPILib::Index				_i_reversal;
		MPILib::Time       	_t_current;

	  vector<MPILib::Index>		_map_cache;

	private:

	  MPILib::Index InitializeResetBin    () const;
	  MPILib::Index InitializeReversalBin () const;

		vector<MPILib::Density> InitializeDensity	() const;

		void NormaliseDensity  				(vector<MPILib::Density>*) const;
		void InitializeGaussian				(vector<MPILib::Density>*) const;
		void InitializeSingleBin       		(vector<MPILib::Density>*) const;

		Number _number_of_bins;
	};
}

#endif // include guard

