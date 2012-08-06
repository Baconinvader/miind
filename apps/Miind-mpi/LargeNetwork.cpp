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
//      If you use this software in work leading to a scientific publication, you should include a reference there to
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net


#include <MPILib/include/largeNetwork/GenerateHexagonalNetwork.hpp>
#include <MPILib/include/largeNetwork/Hexagon.hpp>
#include <MPILib/include/largeNetwork/LargeNetwork.hpp>


#include <MPILib/include/TypeDefinitions.hpp>
#include <MPILib/include/populist/PopulationAlgorithm.hpp>
#include <MPILib/include/populist/OrnsteinUhlenbeckConnection.hpp>
#include <MPILib/include/utilities/CircularDistribution.hpp>
#include <MPILib/include/report/handler/RootReportHandler.hpp>
#include <MPILib/include/SimulationRunParameter.hpp>
int main(int argc, char* argv[]) {

	MPILib::MPINetwork<MPILib::populist::OrnsteinUhlenbeckConnection,
	MPILib::utilities::CircularDistribution> net;
	MPILib::NodeId id_centrum;
	MPILib::NodeId id_bg;

	std::vector<IdGrid> vec_grid;
	std::vector<std::pair<MPILib::NodeId, MPILib::NodeId> > vec_link;
	std::vector<MPILib::NodeId> vec_inh;
	int i_offset;

	// generates a network of hexgonal rings
	GenerateHexagonalNetwork(
			0,		// number of rings, increase if you want a larger network
			&net, &id_centrum, &id_bg, &vec_grid, &vec_link, &vec_inh,
			&i_offset);

	MPILib::Time t_begin = 0.0;
	MPILib::Time t_end = 0.13;
	MPILib::Time t_report = 1e-4;
	MPILib::Time t_step = 1e-5;

	MPILib::report::handler::RootReportHandler hex_handler("hexagon", false);

	MPILib::SimulationRunParameter par_run(hex_handler, 1000000, t_begin, t_end,
			t_report, t_step, "hex");

	net.configureSimulation(par_run);
	try {
		net.evolve();
	} catch (MPILib::utilities::Exception& exc) {
		std::cout << exc.what() << std::endl;
	}
	return 0;
}
