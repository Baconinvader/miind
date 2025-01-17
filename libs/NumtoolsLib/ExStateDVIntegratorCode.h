// Copyright (c) 2005 - 2010 Marc de Kamps
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
#ifndef _CODE_LIBS_NUMTOOLSLIB_EXSTATEDVINTEGRATORCODE_INCLUDE_GUARD
#define _CODE_LIBS_NUMTOOLSLIB_EXSTATEDVINTEGRATORCODE_INCLUDE_GUARD

#include <cassert>
#include "ExStateDVIntegrator.h"
#include "DVIntegratorException.h"
#include "LocalDefinitions.h"

namespace NumtoolsLib 
	{
	
	template <class ParameterObject>
	ExStateDVIntegrator<ParameterObject>::ExStateDVIntegrator
	(
		Number						max_number_iterations,
		double*						p_state,
		Number						number,
		TimeStep					initial_step,
		Time						time,
		const Precision&			precision,
		Function					function, 
		Derivative					derivative,
		const gsl_odeiv_step_type*	p_step_algorithm
	):
	AbstractDVIntegrator<ParameterObject>
	(
		time,
		initial_step,
		precision,
		p_step_algorithm,
		p_state,
		number,
		function,
		derivative,
		max_number_iterations
	)
	{
	}

	template <class ParameterObject>
	ExStateDVIntegrator<ParameterObject>::ExStateDVIntegrator(const ExStateDVIntegrator& integrator):
	AbstractDVIntegrator<ParameterObject>
	(
		integrator
	)
	{
	}

	template <class ParameterObject>
	ExStateDVIntegrator<ParameterObject>::~ExStateDVIntegrator()
	{
	}



	template <class ParameterObject>
	ParameterObject& ExStateDVIntegrator<ParameterObject>::Parameter()
	{
		return this->_parameter_space;
	}

	template <class ParameterObject>
	Time ExStateDVIntegrator<ParameterObject>::CurrentTime() const
	{
		return this->_time_current;
	}

	template <class ParameterObject>
	const double* ExStateDVIntegrator<ParameterObject>::BeginState() const
	{
	  const double* p_return = &(this->_state[0]);
		return p_return;
	}

	template <class ParameterObject>
	const double* ExStateDVIntegrator<ParameterObject>::EndState() const
	{
		const double* p_return = &this->_state.back();
		return p_return;
	}

	template <class ParameterObject>
	string ExStateDVIntegrator<ParameterObject>::Tag() const
	{
		return STRING_DVINTEGRATOR_TAG;
	}

	template <class ParameterObject>
	bool ExStateDVIntegrator<ParameterObject>::Reconfigure
	(
		const DVIntegratorStateParameter<ParameterObject>& parameter
	)
	{
		this->_time_begin   = parameter._time_begin;
		this->_time_current = parameter._time_current;
		this->_step         = parameter._time_step;
		this->_state        = parameter._vector_state;

		this->_parameter_space = parameter._parameter_space;

		this->_max_number_iterations = parameter._number_maximum_iterations;

		this->_number_iterations = 0;

		return true;
	}

} // end of Numtools

#endif // include guard
