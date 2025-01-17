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

#ifndef MPILIB_UTILITIES_NODEDISTRIBUTIONINTERFACE_HPP_
#define MPILIB_UTILITIES_NODEDISTRIBUTIONINTERFACE_HPP_

#include <MPILib/include/TypeDefinitions.hpp>


namespace MPILib{
namespace utilities{


/**
 * NodeDistributionInterface abstract interface for implementing concrete distributions
 */
class NodeDistributionInterface{
public:
	/**
	 * constructor
	 */
	NodeDistributionInterface()=default;

	/**
	 * destructor virtual to allow inheritance
	 */
	virtual ~NodeDistributionInterface()=default;

	/** check is a node is local to the processor
	 * @param nodeId The Id of the Node
	 * @return true if the Node is local
	 */
	virtual bool isLocalNode(NodeId nodeId) const= 0;

	/** get the processor number which is responsible for the node
	 * @param nodeId The Id of the Node
	 * @return the processor responsible
	 */
	virtual int getResponsibleProcessor(NodeId nodeId) const= 0;

	/**
	 * If the processor is master (We assume the processor with _processorId=0 is the master)
	 * @return true if the node is the master.
	 */
	virtual bool isMaster() const = 0;

};
}//end namespace
}//end namespace

#endif /* MPILIB_UTILITIES_NODEDISTRIBUTIONINTERFACE_HPP_ */
