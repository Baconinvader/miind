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
#ifndef GEOMLIB_CONNECTIONSQUAREDPRODUCT_CODE_HPP_
#define GEOMLIB_CONNECTIONSQUAREDPRODUCT_CODE_HPP_

#include <utility>
#include <functional>
#include <vector>
#include <MPILib/include/DelayedConnection.hpp>

namespace GeomLib {
/**
 * Function object
 */
class ConnectionSquaredProduct: public std::binary_function<MPILib::Rate,
		MPILib::DelayedConnection, double> {
public:

	/**
	 * Calculates the connection squared product for the OrnsteinUhlenbeckConnection
	 * @param connection_first The vector of the rates
	 * @param connection_second The vetor of the weights
	 * @return The squared product
	 */
	double operator()(MPILib::Rate connection_first,
			MPILib::DelayedConnection connection_second) const {

		double f_node_rate = connection_first;
		double f_efficacy_squared = connection_second._efficacy
				* connection_second._efficacy;
		double f_number = connection_second._number_of_connections;

		return f_node_rate * f_efficacy_squared * f_number;
	}
};
} /* namespace GeomLib */

#endif // include guard GEOMLIB_CONNECTIONSQUAREDPRODUCT_CODE_HPP_
