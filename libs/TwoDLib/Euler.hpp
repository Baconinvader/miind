// Copyright (c) 2005 - 2018 Marc de Kamps
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
#ifndef _EULER_HPP
#define _EULER_HPP
#include <vector>
#include <MPILib/include/MPINetworkCode.hpp>
#include "CSRMatrix.hpp"
#include "Ode2DSystemGroup.hpp"

namespace TwoDLib {

	void ClearDerivative(std::vector<double>& dydt);

	void AddDerivative
	(
			std::vector<MPILib::Mass>& mass,
			const std::vector<MPILib::Mass>& dydt,
			MPILib::Time h
	);


void CalculateDerivative
	(
			TwoDLib::Ode2DSystemGroup&              sys,
			vector<MPILib::Mass>&		            dydt,
			const std::vector<TwoDLib::CSRMatrix>&  vecmat,
			const std::vector<MPILib::Rate>& 		vecrates
	);

}
#endif
