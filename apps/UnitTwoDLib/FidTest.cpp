// Copyright (c) 2005 - 2015 Marc de Kamps
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
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <TwoDLib.hpp>
#include <vector>
#include <iostream>

using namespace std;
using namespace TwoDLib;


BOOST_AUTO_TEST_CASE(FidTest)
{
	Mesh mesh("aexp0cfa5d69-a740-4474-b8b6-b39870e2f5ef.mesh");
	Fid fid( "aexp0cfa5d69-a740-4474-b8b6-b39870e2f5ef.fid" );
	std::vector<ProtoFiducial> vec_fid = fid.Extract();

	BOOST_REQUIRE(vec_fid.size() == 9);
	BOOST_REQUIRE(vec_fid[0].second == CONTAIN);
	BOOST_REQUIRE(vec_fid[4].second == LEAK);

	std::vector<FiducialElement> list = fid.Generate(mesh);

	BOOST_REQUIRE(list.size() == 9);
}


