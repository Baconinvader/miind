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

#define private public
#define protected public
#include <MPILib/include/utilities/Log.hpp>
#undef protected
#undef private
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
namespace mpi = boost::mpi;

mpi::communicator world;
#include <boost/test/minimal.hpp>
using namespace boost::unit_test;
namespace MPILib {
namespace utilities {

void test_Constructor() {
	Log lg;
}

void test_Destructor() {
	Log* lg = new Log();
	std::shared_ptr<std::ostringstream> pStream ( new std::ostringstream());

	Log::setStream(pStream);

	lg->writeReport() << "blub";
	BOOST_CHECK(pStream->str().find("blub")!=38);
	delete lg;
	BOOST_CHECK(pStream->str().find("blub")==38);

}

void test_LogLevel() {
	BOOST_CHECK(logERROR<logWARNING);
	BOOST_CHECK(logWARNING<logINFO);
	BOOST_CHECK(logINFO<logDEBUG);
	BOOST_CHECK(logDEBUG<logDEBUG1);
	BOOST_CHECK(logDEBUG1<logDEBUG2);
	BOOST_CHECK(logDEBUG2<logDEBUG3);
	BOOST_CHECK(logDEBUG3<logDEBUG4);

}

void test_writeReport() {
	Log lg;

	lg.writeReport() << "blub" << 42;
	BOOST_CHECK(lg._buffer.str().find("blub")==38);
	BOOST_CHECK(lg._buffer.str().find("42")==42);

}

void test_setGetStream() {
	Log* lg = new Log();
	std::shared_ptr<std::ostringstream> pStream ( new std::ostringstream());

	Log::setStream(pStream);

	BOOST_CHECK(pStream == Log::getStream());
	delete lg;
}



void test_writeOutput() {
	std::shared_ptr<std::ostringstream> pStream ( new std::ostringstream());

	Log::setStream( pStream);
	Log::writeOutput(std::string("blub"));
	BOOST_CHECK(pStream->str().find("blub")==0);

}

void test_getReportingLevel() {
	BOOST_CHECK(Log::getReportingLevel()==logDEBUG4);
}

void test_setReportingLevel() {
	BOOST_CHECK(Log::getReportingLevel()==logDEBUG4);
	std::shared_ptr<std::ostringstream> pStream ( new std::ostringstream());

	Log::setStream( pStream);
	Log::setReportingLevel(logERROR);
	BOOST_CHECK(pStream->str().find("Report")==38);
	BOOST_CHECK(Log::getReportingLevel()==logERROR);

}

void test_Macro() {
	std::shared_ptr<std::ostringstream> pStream ( new std::ostringstream());

	Log::setStream( pStream);

	LOG(logERROR) << "blub" << 42;

	BOOST_CHECK(pStream->str().find("blub")==39);
	BOOST_CHECK(pStream->str().find("42")==43);
	Log::setReportingLevel(logERROR);
	std::shared_ptr<std::ostringstream> pStream1 ( new std::ostringstream());

	Log::setStream( pStream1);
	LOG(logDEBUG) << "blub" << 42;
	BOOST_CHECK(pStream1->str().find("blub")!=39);
	BOOST_CHECK(pStream1->str().find("42")!=43);
	std::shared_ptr<std::ostringstream> pStream2 ( new std::ostringstream());

	Log::setStream( pStream2);
	LOG(logERROR) << "blub" << 42;

	BOOST_CHECK(pStream2->str().find("blub")==39);
	BOOST_CHECK(pStream2->str().find("42")==43);
}

}
}

int test_main(int argc, char* argv[]) // note the name!
		{

	mpi::environment env(argc, argv);
	if (world.size() != 2) {
		BOOST_FAIL( "Run the test with two processes!");
	}

// we use only two processors for this testing
	MPILib::utilities::test_Constructor();
	MPILib::utilities::test_Destructor();
	MPILib::utilities::test_LogLevel();
	MPILib::utilities::test_writeReport();
	MPILib::utilities::test_setGetStream();
	MPILib::utilities::test_writeOutput();
	MPILib::utilities::test_getReportingLevel();
	MPILib::utilities::test_setReportingLevel();
	MPILib::utilities::test_Macro();

	return 0;

}
