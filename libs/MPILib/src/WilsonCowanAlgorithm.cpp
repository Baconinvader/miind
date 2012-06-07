/*
 * WilsonCowanAlgorithm.cpp
 *
 *  Created on: 07.06.2012
 *      Author: david
 */

#include <MPILib/include/WilsonCowanAlgorithm.hpp>
#include <MPILib/include/utilities/ParallelException.hpp>
#include <MPILib/include/BasicTypes.hpp>


#include "../../NumtoolsLib/NumtoolsLib.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_errno.h>

#include <functional>
#include <numeric>

namespace
{

	int sigmoid(double t, const double y[], double f[], void *params)
	{
		MPILib::WilsonCowanParameter* p_parameter = (MPILib::WilsonCowanParameter *)params;

		f[0] = (-y[0] + p_parameter->_rate_maximum/(1 + exp(-p_parameter->_f_noise*p_parameter->_f_input))) /p_parameter->_time_membrane;

		return GSL_SUCCESS;
	}


	int sigmoidprime(double t, const double y[], double *dfdy, double dfdt[], void *params)
	{
		MPILib::WilsonCowanParameter* p_parameter = (MPILib::WilsonCowanParameter *)params;
		gsl_matrix_view dfdy_mat  = gsl_matrix_view_array (dfdy, 1, 1);

		gsl_matrix * m = &dfdy_mat.matrix;


		gsl_matrix_set (m, 0, 0, -1/p_parameter->_time_membrane);

		dfdt[0] = 0.0;

		return GSL_SUCCESS;

	}
};

namespace MPILib {

WilsonCowanAlgorithm::WilsonCowanAlgorithm() :
		AlgorithmInterface<double>(),
		_integrator
			(
				0,
				InitialState(),
				0,
				0,
				NumtoolsLib::Precision(WC_ABSOLUTE_PRECISION,WC_RELATIVE_PRECISION),
				sigmoid,
				sigmoidprime
			){
	// TODO Auto-generated constructor stub

}

WilsonCowanAlgorithm::~WilsonCowanAlgorithm() {
	// TODO Auto-generated destructor stub
}

WilsonCowanAlgorithm* WilsonCowanAlgorithm::Clone() const {
	return new WilsonCowanAlgorithm(*this);
}

void WilsonCowanAlgorithm::Configure(const SimulationRunParameter& simParam) {

}

void WilsonCowanAlgorithm::EvolveNodeState(
		const std::vector<Rate>& nodeVector,
		const std::vector<double>& weightVector, Time time) {

	double f_inner_product = innerProduct(nodeVector, weightVector);

	_integrator.Parameter()._f_input = f_inner_product;

	try {
		while (_integrator.Evolve(time) < time)
			;
	} catch (NumtoolsLib::DVIntegratorException& except) {
		//FIXME
//		if (except.Code() == NumtoolsLib::NUMBER_ITERATIONS_EXCEEDED)
//			throw miind_parallel_fail("number of iterations exceeded");
//		else
//			throw except;
	}
}

Time WilsonCowanAlgorithm::getCurrentTime() const {
	return 0.0;

}

NodeState WilsonCowanAlgorithm::getCurrentRate() const {
	vector<double> state(1);
	state[0] = *_integrator.BeginState();
	return state;
}

double WilsonCowanAlgorithm::innerProduct(
		const std::vector<Rate>& nodeVector,
		const std::vector<double>& weightVector) {

	assert(nodeVector.size()==weightVector.size());

	if (nodeVector.begin() == nodeVector.end())
		return 0;

	return std::inner_product(nodeVector.begin(), nodeVector.end(),
			weightVector.begin(), 0.0);

}

vector<double> WilsonCowanAlgorithm::InitialState() const
{
	vector<double> array_return(WILSON_COWAN_STATE_DIMENSION);
	array_return[0] = 0;
	return array_return;
}

} /* namespace MPILib */
