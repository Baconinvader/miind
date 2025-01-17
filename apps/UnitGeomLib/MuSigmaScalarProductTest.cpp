
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
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>
#include <GeomLib.hpp>
#include <MPILib/include/AlgorithmInterface.hpp>
#include <MPILib/include/DelayedConnection.hpp>

using namespace GeomLib;
using namespace MPILib;
using MPILib::DelayedConnection;

BOOST_AUTO_TEST_CASE(MuSigmaScalarTest)
{
        MuSigmaScalarProduct<MPILib::DelayedConnection>  prod;

	vector<NodeType> vec_type;
	vec_type.push_back(EXCITATORY_GAUSSIAN);
	vec_type.push_back(INHIBITORY_GAUSSIAN);

	vector<MPILib::DelayedConnection> vec_con;
	MPILib::DelayedConnection con_1(1, 0.01,0.0);
	MPILib::DelayedConnection con_2(1,-0.01,0.0);
	vec_con.push_back(con_1);
	vec_con.push_back(con_2);

	Rate rate = 1000.0;
	vector<Rate> vec_rates;

	Time tau = 10e-3;

	vec_rates.push_back(rate);
	vec_rates.push_back(rate);

	MuSigma ms = prod.Evaluate(vec_rates, vec_con, tau);

	BOOST_CHECK(ms._mu == 0.0);
	BOOST_CHECK_CLOSE(ms._sigma, 0.0447214, 0.001);

}

