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
#ifndef _CODE_LIBS_GEOMLIB_QIF_PARAMETER_INCLUDE_GUARD
#define _CODE_LIBS_GEOMLIB_QIF_PARAMETER_INCLUDE_GUARD

#include "CurrentParameter.hpp"

namespace GeomLib {

	//! This parameter configures the QIFAlgorithm

	//! The default parameters are chosen such that the normal form of the QIFAlgorithm will be configured.
    //! See the QIFAlgorithm documentation for this
	//! The default constructor will set values, so that the topological normal form will be simulated:
	//! \f[
	//! \tau\frac{dV}{dt} = \gamma + V^{2}
	//! \f]
	//! 
	//! A subtlety arises when neurons with negative \f$ \gamma \f$ must be modeled. In the current implementation
	//! this is achieved by defining implementing a model with positive \f$ \gamma \f$, whilst adding a negative
	//! DC contribution to the external neurons, i.e. the resulting neural dynamics is made up of an
	//! intrinsically spiking neuron with \f$ \gamma_{sys} > 0 \f$, and an extra negative DC contribution
	//! to the neural input. This trick is called current compensation. Current compensation is done automatically,
	//! and the user can ignore the \f$ \gamma_{sys} \f$ parameter, unless an influence on the performance is
	//! suspected.
	class QifParameter : public CurrentParameter {
	public:
		QifParameter
		(
			Potential gamma,			//!< Shape parameter (see QIFAlgorithm)
			Potential gamma_sys	= 0.5	//!< The gamma for which SpikingOdeSystem will be run (if you don't know what that means, leave alone)
		):
		CurrentParameter(gamma_sys),
		_gamma(gamma),
		_gamma_sys(gamma_sys)
		{
		}

		//! The value by which QifOdeSystem will be run. For now the default value is recommended.
		double Gammasys() const { return _gamma_sys; }

		public:

			Potential	_gamma;			//! Although a current, it is best to see this as a shape parameter (see QIFAlgorithm)
	
		private:

			Potential	_gamma_sys;		//! The gamma for which OdeSystems will be run (if you don't know what that means, leave alone)

	};

	bool operator==(const QifParameter&, const QifParameter&);
}

#endif // include guard

