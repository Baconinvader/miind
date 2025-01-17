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

#ifndef SLEEPALGORITHM_HPP_
#define SLEEPALGORITHM_HPP_
#include <MPILib/algorithm/AlgorithmGrid.hpp>
#include <boost/thread/thread.hpp>
#include <MPILib/include/BasicDefinitions.hpp>

using namespace MPILib;

template<class WeightValue>
class SleepAlgorithm: public algorithm::AlgorithmInterface<WeightValue> {
public:

	SleepAlgorithm() {

	}
	~SleepAlgorithm() {
	}

	SleepAlgorithm<WeightValue>* clone() const {
		return new SleepAlgorithm(*this);
	}

	void configure(const SimulationRunParameter&) {

	}

	void evolveNodeState(const std::vector<ActivityType>&,
			const std::vector<WeightValue>&, Time) {

		boost::this_thread::sleep(boost::posix_time::seconds(kSleepTime));

	}

	Time getCurrentTime() const {
		return 1.0;
	}

	Rate getCurrentRate() const {
		return 9;

	}

	algorithm::AlgorithmGrid getGrid(NodeId) const {
		std::vector<double> vector_grid(RATE_STATE_DIMENSION, 1);
		std::vector<double> vector_interpretation(RATE_STATE_DIMENSION, 0);
		return algorithm::AlgorithmGrid(vector_grid, vector_interpretation);
	}

private:

	double kSleepTime = 1;
};

#endif /* SLEEPALGORITHM_HPP_ */
