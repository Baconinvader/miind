/*
 * ParallelException.cpp
 *
 *  Created on: 30.05.2012
 *      Author: david
 */

#include <MPILib/config.hpp>
#include <MPILib/include/utilities/ParallelException.hpp>
#include <MPILib/include/utilities/MPIProxy.hpp>
#include <sstream>

using namespace MPILib::utilities;


ParallelException::ParallelException(const char* message) :
		Exception(message) {
	MPIProxy mpiProxy;
	std::stringstream sstream;
	sstream <<std::endl<< "Parallel Exception on processor: " << mpiProxy.getRank() << " from: "
			<< mpiProxy.getSize() << " with error message: " << msg_<<std::endl;
	msg_ = sstream.str();
}

ParallelException::ParallelException(const std::string& message) :
		Exception(message) {
	MPIProxy mpiProxy;
	std::stringstream sstream;
	sstream <<std::endl<< "Parallel Exception on processor: " << mpiProxy.getRank() << " from: "
			<< mpiProxy.getSize() << " with error message: " << msg_<<std::endl;
	msg_ = sstream.str();
}

ParallelException::~ParallelException() throw () {
}

const char* ParallelException::what() const throw () {
	return msg_.c_str();
}
