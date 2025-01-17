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
#ifndef _CODE_LIBS_NUMTOOLSLIB_POISSONDISTRIBUTION_INCLUDE_GUARD
#define _CODE_LIBS_NUMTOOLSLIB_POISSONDISTRIBUTION_INCLUDE_GUARD

#include "Distribution.h"

// Date:   12-04-1999
// Author: Marc de Kamps
// Short Description:  Produces values that are distributed accoding to a Gaussian with zero mean
// and unit sd
 


namespace NumtoolsLib
{

	//! Poissondistribution
	//! generates a Poisson distribution with a rate

	class PoissonDistribution : public Distribution 
	{
	public:
		
		//!  Initialize with a rate, and relate to the RandomGenerator

		PoissonDistribution	
			(
				RandomGenerator&, 
				double  f_rate
			);   

		//! Generate the next sample value.
		virtual	double	NextSampleValue	();

	private:

		// copying of distributions is potentially dangerous (seed mixing, reproduction problems)
		// and seems unnecessary

		PoissonDistribution	(const PoissonDistribution&);
		PoissonDistribution& operator=(const PoissonDistribution&);	
	
		// auxiliary variables for Poisson algorithm

		double	_f_rate;
		double	_sq;
		double	_alxm;
		double	_g;
		double	_oldm;
	};

} // end of Numtools



#endif // include guard

