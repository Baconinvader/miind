/*
 * WilsonCowanAlgorithm.hpp
 *
 *  Created on: 07.06.2012
 *      Author: david
 */

#ifndef MPILIB_ALGORITHMS_WILSONCOWANALGORITHM_HPP_
#define MPILIB_ALGORITHMS_WILSONCOWANALGORITHM_HPP_

#include <NumtoolsLib/NumtoolsLib.h>
#include <DynamicLib/WilsonCowanParameter.h>

#include <MPILib/include/algorithm/AlgorithmInterface.hpp>

namespace MPILib {
namespace algorithm{


class WilsonCowanAlgorithm: public AlgorithmInterface<double> {
public:
	WilsonCowanAlgorithm();

	WilsonCowanAlgorithm(const DynamicLib::WilsonCowanParameter&);

	virtual ~WilsonCowanAlgorithm();

	/**
	 * Cloning operation, to provide each DynamicNode with its own
	 * Algorithm instance. Clients use the naked pointer at their own risk.
	 */
	virtual WilsonCowanAlgorithm* clone() const;

	/**
	 * Configure the Algorithm
	 * @param simParam
	 */
	virtual void configure(const DynamicLib::SimulationRunParameter& simParam);

	/**
	 * Evolve the node state
	 * @param nodeVector Vector of the node States
	 * @param weightVector Vector of the weights of the nodes
	 * @param time Time point of the algorithm
	 */
	virtual void evolveNodeState(const std::vector<Rate>& nodeVector,
			const std::vector<double>& weightVector, Time time);

	/**
	 * The current timepoint
	 * @return The current time point
	 */
	virtual Time getCurrentTime() const;

	/**
	 * The calculated rate of the node
	 * @return The rate of the node
	 */
	virtual Rate getCurrentRate() const;

	virtual DynamicLib::AlgorithmGrid getGrid() const;

private:

	double innerProduct(const std::vector<Rate>& nodeVector,
			const std::vector<double>& weightVector);

	vector<double> getInitialState() const;

	DynamicLib::WilsonCowanParameter _parameter;

	NumtoolsLib::DVIntegrator<DynamicLib::WilsonCowanParameter> _integrator;

};

} /* namespace algorithm */
} /* namespace MPILib */
#endif /* MPILIB_ALGORITHMS_WILSONCOWANALGORITHM_HPP_ */