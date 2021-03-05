#ifndef _INCLUDE_GUARD_SIMULATION_PARSER_GPU
#define _INCLUDE_GUARD_SIMULATION_PARSER_GPU

#include <string>
#include <MPILib/include/MiindTvbModelAbstract.hpp>
#include <MPILib/include/RateAlgorithm.hpp>
#include <TwoDLib/GridAlgorithm.hpp>
#include <MPILib/include/CustomConnectionParameters.hpp>
#include <CudaTwoDLib/CudaTwoDLib.hpp>
#include <MiindLib/VectorizedNetwork.hpp>
#include <TwoDLib/SimulationParserCPU.h>

typedef CudaTwoDLib::fptype fptype;

template <class WeightType>
class SimulationParserGPU : public SimulationParserCPU<WeightType> {
public:
	SimulationParserGPU(int num_nodes, const std::string xml_filename);
	SimulationParserGPU(const std::string xml_filename);

	void endSimulation();

	void addGridConnection(pugi::xml_node& xml_conn);

	void addMeshConnection(pugi::xml_node& xml_conn);

	/*void addGridConnectionCCP(pugi::xml_node& xml_conn);

	void addMeshConnectionCCP(pugi::xml_node& xml_conn);

	void addIncomingGridConnectionCCP(pugi::xml_node& xml_conn);

	void addIncomingMeshConnectionCCP(pugi::xml_node& xml_conn);

	bool addGridAlgorithmGroupNode(pugi::xml_document& doc, std::string alg_name);

	bool addMeshAlgorithmGroupNode(pugi::xml_document& doc, std::string alg_name);

	bool addRateFunctorNode(pugi::xml_document& doc, std::string alg_name);

	void parseXmlFile();

	void startSimulation();

	void init();

	double getCurrentSimTime();

	std::vector<double> evolveSingleStep(std::vector<double> activity);

	bool simulationComplete();*/

private:

	MiindLib::VectorizedNetwork vec_network;
	std::map<std::string, std::string> _node_algorithm_types;

	/*std::vector<TwoDLib::Mesh> _meshes;
	std::vector<std::vector<TwoDLib::Redistribution>> _reversal_mappings;
	std::vector<std::vector<TwoDLib::Redistribution>> _reset_mappings;
	std::vector<TwoDLib::TransitionMatrix> _transition_mats;
	std::map<std::string, std::map<std::string, TwoDLib::TransitionMatrix>> _mesh_transition_matrics;
	std::map<std::string, std::string> _node_algorithm_mapping;
	std::vector<rate_functor> _rate_functors;

	std::string _xml_filename;

	std::map<std::string, MPILib::NodeId> _node_ids;
	
	unsigned int _external_node_count;

	unsigned long _count;
	std::vector<MPILib::NodeId> _display_nodes;
	std::vector<MPILib::NodeId> _rate_nodes;
	std::vector<MPILib::Time> _rate_node_intervals;
	std::vector<MPILib::NodeId> _density_nodes;
	std::vector<MPILib::Time> _density_node_start_times;
	std::vector<MPILib::Time> _density_node_end_times;
	std::vector<MPILib::Time> _density_node_intervals;*/


};

#endif