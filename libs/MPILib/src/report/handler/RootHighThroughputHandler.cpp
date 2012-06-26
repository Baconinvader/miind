// Copyright (c) 2005 - 2011 Marc de Kamps
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
//      If you use this software in work leading to a scientific publication, you should cite
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net
#include <TClass.h>
#include <TFile.h>
#include <TGraph.h>
#include <TVectorT.h>
#include <TTree.h>
#include <TNTuple.h>
#include <MPILib/include/utilities/Exception.hpp>
#include <MPILib/include/report/handler/RootHighThroughputHandler.hpp>
#include <iostream>

#include <boost/mpi/communicator.hpp>

namespace MPILib {
namespace report {
namespace handler {

TTree* RootHighThroughputHandler::_pTree = 0;
bool RootHighThroughputHandler::_isRecording = false;
bool RootHighThroughputHandler::_generateNodeGraphs = false;

std::map<int, double> RootHighThroughputHandler::_mData;


TFile* RootHighThroughputHandler::_pFile = 0;
TVectorD* RootHighThroughputHandler::_pArray = 0;

RootHighThroughputHandler::RootHighThroughputHandler(
		const std::string& fileName, bool haveState,
		bool generateGraphs) :
		AbstractReportHandler(fileName) {
	if(haveState){
		throw(utilities::Exception("RootHighThroughputHandler cannot generate State graphs"));
	}
	this->_generateNodeGraphs = generateGraphs;
}

RootHighThroughputHandler::RootHighThroughputHandler(
		const RootHighThroughputHandler& rhs) :
		AbstractReportHandler(rhs) {
}

void RootHighThroughputHandler::detachHandler(const NodeId&) {
	// write after the last array
	if (_pArray) {
		_pTree->Write();

		// if so desired, create node rate graphs
		// note that the file is closed by the first node so that the test for its existence is necessary
		if (_generateNodeGraphs && _pFile)
			generateNodeGraphs(_pFile->GetName());

	}

	// restore all static definitions
	delete _pArray;
	_pArray = 0;
	_isRecording = false;
	_generateNodeGraphs = false;
	_mData.clear();
	_pTree = 0;

	if (_pFile) {
		// clean up
		_pFile->Close();
		delete _pArray;
		_pArray = 0;
		_pFile = 0;
	}
}

RootHighThroughputHandler* RootHighThroughputHandler::clone() const {
	return new RootHighThroughputHandler(*this);
}

void RootHighThroughputHandler::initializeHandler(const NodeId&) {
	if (!_pFile)
		_pFile = new TFile(this->getRootOutputFileName().c_str(), "RECREATE");
	if (_pFile->IsZombie())
		throw utilities::Exception("Couldn't open root file");

}

void RootHighThroughputHandler::writeReport(const Report& report) {

	if (!_isRecording) {
		boost::mpi::communicator world;

		_pTree = new TTree("NodeIds", "Node Ids");


		TVectorD* nodeId = new TVectorD(report._nrNodes);
		_pTree->Branch("GlobalNodeIds", "TVectorT<double>", &nodeId, report._nrNodes, 0);

		for (Index i = 0; i < report._nrNodes; i++) {
			(*nodeId)[i] = world.size() * i + world.rank();
			std::cout<<"a"<<(*nodeId)[i]<<"\t"<<i<<std::endl;
		}
		_pTree->Fill();
		_pTree->Write();

		_pTree = 0;
		delete nodeId;

		_pTree = new TTree("Activations", "Times slices");
		_pTree->Branch("slices", "TVectorT<double>", &_pArray, 32000, 0);
		_pArray = new TVectorD(report._nrNodes + 1);
		_isRecording = true;
	}

	if (_isRecording) {
		//store the node in a map one below the max number of nodes
		if (_mData.size() < (report._nrNodes - 1)) {
			_mData[report._id] = report._rate;
		}
		// write the map to the file
		else {
			_mData[report._id] = report._rate;

			(*_pArray)[0] = report._time;
			//They have to have the same size
			assert(_mData.size() == report._nrNodes);

			Index i = 1;
			for (auto& it : _mData) {
				(*_pArray)[i] = it.second;
				i++;
			}

			//clear the map
			_mData.clear();
			_pTree->Fill();

		}
	}

}
RootHighThroughputHandler::~RootHighThroughputHandler() {
}

void RootHighThroughputHandler::generateNodeGraphs(const char* fileName) {
	const std::string file_name(fileName);
	//close to flush buffers and reopen to append
	_pFile->Close();
	delete _pFile;
	//closing the file has invalidated (destroyed) the tree
	_pTree = 0;
	delete _pArray;
	_pArray = 0;

	_pFile = new TFile(file_name.c_str(), "UPDATE");

	TGraph* p_graph = (TGraph*) _pFile->Get("rate_1");
	if (p_graph) {
		utilities::Exception("The Graph already exists");
	}

	Number number_of_nodes = -1;
	Number number_of_slices = -1;
	std::vector<double> vec_times;

	collectGraphInformation(vec_times, number_of_nodes, number_of_slices);

	storeRateGraphs(vec_times, number_of_nodes, number_of_slices);

	_pFile->Close(); // the file object does not exist anymore, don't delete the pointer
	_pFile = 0; // just set it to 0
	delete _pArray;
	_pArray = 0;

}

void RootHighThroughputHandler::storeRateGraphs(
		const std::vector<double>& vecTime, Number nrNodes, Number nrSlices) {

	TTree* nodeTree = (TTree*) _pFile->Get("NodeIds");
	if (!nodeTree)
		throw utilities::Exception("No valid TTree for the nodeIds");
	// FIXME why the hell are the elements wrong??????
	TVectorD* nodeId = new TVectorD(nrNodes);
	TBranch* nodeBranch = nodeTree->GetBranch("GlobalNodeIds");
	nodeBranch->SetAddress(&nodeId); //address of pointer!
	std::cout<<nodeId->GetNoElements()<<(*nodeId)[0]<<(*nodeId)[1]<<std::endl;

	TBranch* p_branch = _pTree->GetBranch("slices");
	p_branch->SetAddress(&_pArray); //address of pointer!

	for (Index node = 0; node < nrNodes; node++) {
		Index id_node = node + 1;

		std::vector<double> vec_rate;
		for (Index slice = 0; slice < nrSlices; slice++) {
			p_branch->GetEvent(slice);
			vec_rate.push_back((*_pArray)[id_node]);
		}
		if (vec_rate.size() != vecTime.size())
			throw utilities::Exception("Inconsistency between times and rate");

		// Here we have all we need to create a Graph
		TGraph* p_graph = new TGraph(vec_rate.size(), &(vecTime[0]),
				&(vec_rate[0]));
		std::ostringstream stgr;
		///@TODO get the global node id for the node why are they wrong????
		std::cout<<"b"<<(*nodeId)[node]<<"\t"<<node<<std::endl;
		stgr << "rate_" << int((*nodeId)[node]);

		p_graph->SetName(stgr.str().c_str());
		p_graph->Write();
		delete p_graph;
	}
}

void RootHighThroughputHandler::collectGraphInformation(
		std::vector<double>& pVecTime, Number& pNrNodes,
		Number& pNrSlices) {
	if (_pTree)
		throw utilities::Exception("There is a TTree that shouldn't be there");

	_pTree = (TTree*) _pFile->Get("Activations");
	if (!_pTree)
		throw utilities::Exception("No valid TTree");

	pNrSlices = static_cast<Number>(_pTree->GetEntries());

	TBranch* p_branch = _pTree->GetBranch("slices");
	p_branch->SetAddress(&_pArray); //address of pointer!

	for (Index i = 0; i < pNrSlices; i++) {
		p_branch->GetEvent(i);
		pVecTime.push_back((*_pArray)[0]);
	}
	//ignore the time in the first entry
	pNrNodes = _pArray->GetNoElements() - 1;
}

} // end namespace of handler
} // end namespace of report
} // end namespace of MPILib
