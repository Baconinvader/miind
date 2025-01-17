// Copyright (c) 2005 - 2014 Marc de Kamps
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
#ifndef MPILIB_POPULIST_DELAYEDCONNECTION_HPP_
#define MPILIB_POPULIST_DELAYEDCONNECTION_HPP_

#include <limits>
#include <MPILib/include/TypeDefinitions.hpp>

namespace MPILib {

/**
 * Connection parameters for a network of OrnsteinUhlenbeck populations
 * need two parameters: a number parameter, which is not squared in the
 * calculation of sigma, and an efficacy, which is squared.
 */
struct DelayedConnection {

	/**
	 * effective number, may be fractional
	 */
	double _number_of_connections = 0.0;

	/**
	 * effective synaptic efficacy from one population on another
	 */
	Efficacy _efficacy = 0.0;

	/**
	 * delay of a connection
	 */
	Time _delay = 0.0;

	/**
	 * default constructor
	 */
	DelayedConnection()=default;

	/**
	 * construct, using effective number of connections and effective efficacy
	 * @param number_of_connections effective number of connections
	 * @param efficacy synaptic efficacy
	 * @param delay  delay of connection
	 */
	DelayedConnection(double number_of_connections,
			Efficacy efficacy,
			Time delay = 0
	) :
	_number_of_connections(number_of_connections), _efficacy(efficacy), _delay(
			delay) {
	}
};

/**
 * Necessary for a sensible definition of InnerProduct
 * @param f the factor for the inner product
 * @param connection The connection
 * @return The product of the factor with the efficacy and the number of connections
 */
inline double operator*(double f,
		const DelayedConnection& connection) {
	return f * connection._efficacy * connection._number_of_connections;
}

/**
 * Necessary for tests on Dale's law
 * @param connection The connection
 * @return The efficacy of the connection
 */
inline double toEfficacy(const DelayedConnection& connection) {
	return connection._efficacy;
}

} /* namespace MPILib */
/**
 * if the connection is NAN
 * @param connection The connection
 * @return true if the connection is NAN
 */
inline int IsNan( const MPILib::DelayedConnection& connection) {
  return (std::numeric_limits<MPILib::Efficacy>::signaling_NaN() == connection._efficacy ||
          std::numeric_limits<double>::signaling_NaN() == connection._number_of_connections ||
	  std::numeric_limits<MPILib::Time>::signaling_NaN() == connection._delay);
}

#endif // include guard MPILIB_POPULIST_ORNSTEINUHLENBECKCONNECTION_HPP_
