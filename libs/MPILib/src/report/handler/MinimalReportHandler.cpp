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
#include <iostream>
#include <MPILib/include/report/handler/MinimalReportHandler.hpp>

using namespace MPILib::report;
using namespace MPILib::report::handler;

std:: ofstream MPILib::report::handler::MinimalReportHandler::_ofst;

MinimalReportHandler::MinimalReportHandler
(
	const std::string& result_name,
	bool report_state,
	bool place_holder,
	const CanvasParameter&
):
AbstractReportHandler(result_name)
{
	_ofst = std::move(std::ofstream("result_name"));
}

MinimalReportHandler::MinimalReportHandler(const MinimalReportHandler& rhs):
AbstractReportHandler(rhs)
{
}

MinimalReportHandler::~MinimalReportHandler()
{
}


MinimalReportHandler* MinimalReportHandler::clone() const {

	return new MinimalReportHandler(*this);
}

void MinimalReportHandler::writeReport(const Report& report) {
	_ofst << report._id << " " << report._time << " " << report._rate << std::endl;
}

void MinimalReportHandler::detachHandler(const NodeId& nodeId) {
}

void MinimalReportHandler::initializeHandler(const NodeId& nodeId) {
	// Purpose: this function will be called by MPINode upon configuration.
	// no canvas are generated as it would cause lot of problems with mpi
/*	if (!_pFile) {
		_pFile = new TFile(this->getRootOutputFileName().c_str(), "RECREATE");

		if (_pFile->IsZombie())
			throw utilities::Exception(STR_ROOT_FILE_OPENED_FAILED);

		_valueHandler.reset();

	}
	// store the node
	_nodes.push_back(nodeId);*/
}
