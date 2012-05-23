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
//      If you use this software in work leading to a scientific publication, you should include a reference there to
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net
#ifndef _CODE_LIBS_NETLIB_SPARSEIMPLEMENTATIONTESTCODE_INCLUDE_GUARD
#define _CODE_LIBS_NETLIB_SPARSEIMPLEMENTATIONTESTCODE_INCLUDE_GUARD

#include "SparseImplementationTest.h"
#include "SparseLibException.h"
#include "LayerWeightIteratorCode.h"
#include "TestDefinitions.h"

namespace SparseImplementationLib
{
 
	    template <class NodeType>
		inline LayeredSparseImplementation<NodeType>
		     SparseImplementationTest::CreateTestLayeredSparseImplementation() const
		{
			typedef LayeredSparseImplementation<NodeType> Implementation;

			NetLibTest test;
			// Create Architecture
			LayeredArchitecture architecture = test.CreateArchitecture(false);

			// Create implementation
			Implementation implementation(&architecture);

			// Label the weights

			bool b_test = InsertTestWeights<Implementation>(implementation);

			if (b_test)
				throw SparseLibException("Test weight insertion test should have failed");
			// It should fail because this is an architecture without thresholds

			// try again
			b_test = InsertTestWeightsWithoutThreshold<Implementation>(implementation);
	
			if (! b_test)
				throw SparseLibException(STRING_TESTIMPLEMENTATION_NOT_CREATED);

			return implementation;
		}

	    template <class ReversibleNode>
		inline ReversibleLayeredSparseImplementation<ReversibleNode>
		     SparseImplementationTest::CreateTestReversibleLayeredSparseImplementation() const
		{
			typedef ReversibleLayeredSparseImplementation<ReversibleNode> Implementation;

			// Create Architecture
			NetLibTest test;

			// Must be true ! we also insert threshold weights
			LayeredArchitecture architecture = test.CreateArchitecture(true);

			// Create implementation
			Implementation implementation(&architecture);


			// Label the weights
			bool b_test = InsertTestWeights<Implementation>(implementation);

			if (b_test)
				return implementation;

			//TODO: string in header 
			throw NetLib::NetlibTestException("Weight test failed");
		}

} // end of SparseImplementationLib

#endif // include guard
