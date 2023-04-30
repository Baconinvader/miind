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
//      If you use this software in work leading to a scientific publication, you should include a reference there to
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include "Ode2DSystemGroup.hpp"
#include "TwoDLibException.hpp"

using namespace TwoDLib;


std::vector<Ode2DSystemGroup::Clean> Ode2DSystemGroup::InitializeClean()
{
	std::vector<Ode2DSystemGroup::Clean> vec_ret;
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		Clean clean(*this, _vec_mass, m);
		vec_ret.push_back(clean);
	}
	return vec_ret;
}

std::vector<Ode2DSystemGroup::Reset> Ode2DSystemGroup::InitializeReset()
{
	std::vector<Ode2DSystemGroup::Reset> vec_ret;
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		Reset reset(*this, _vec_mass, m);
		vec_ret.push_back(reset);
	}
	return vec_ret;
}

std::vector<Ode2DSystemGroup::ObjectReset> Ode2DSystemGroup::InitializeObjectReset()
{
	std::vector<Ode2DSystemGroup::ObjectReset> vec_ret;
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		ObjectReset reset(*this, _vec_cells_to_objects, _vec_objects_refract_times, _vec_objects_refract_index, _vec_tau_refractive[m], m);
		vec_ret.push_back(reset);
	}
	return vec_ret;
}

void Ode2DSystemGroup::InitializeResetRefractive(MPILib::Time network_time_step)
{
	_reset_refractive = InitializeResetRefractiveInternal(network_time_step);
}

std::vector<Ode2DSystemGroup::ResetRefractive> Ode2DSystemGroup::InitializeResetRefractiveInternal(MPILib::Time network_time_step)
{
	if (_vec_reset.size() != _mesh_list.size() || _vec_reversal.size() != _mesh_list.size())
		throw TwoDLib::TwoDLibException("Reversal and reset vector sizes must match the number of meshes.");
	std::vector<Ode2DSystemGroup::ResetRefractive> vec_ret;
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		ResetRefractive reset(*this, _vec_mass, network_time_step, _vec_tau_refractive[m], _vec_reset[m], m);
		vec_ret.push_back(reset);
	}
	return vec_ret;
}

std::vector<Ode2DSystemGroup::Reversal> Ode2DSystemGroup::InitializeReversal()
{
	std::vector<Ode2DSystemGroup::Reversal> vec_ret;
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		Reversal reversal(*this, _vec_mass, m);
		vec_ret.push_back(reversal);
	}
	return vec_ret;
}

std::vector<Ode2DSystemGroup::ObjectReversal> Ode2DSystemGroup::InitializeObjectReversal()
{
	std::vector<Ode2DSystemGroup::ObjectReversal> vec_ret;
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		ObjectReversal reversal(*this, _vec_cells_to_objects, _vec_objects_to_index, m);
		vec_ret.push_back(reversal);
	}
	return vec_ret;
}

Ode2DSystemGroup::Ode2DSystemGroup
(
	const std::vector<Mesh>& mesh_list,
	const std::vector<std::vector<Redistribution> >& vec_reversal,
	const std::vector<std::vector<Redistribution> >& vec_reset,
	const std::vector<MPILib::Time>& vec_tau_refractive,
	const std::vector<MPILib::Index> num_objects
) :
	_mesh_list(mesh_list),
	_vec_mesh_offset(MeshOffset(mesh_list)),
	_vec_length(InitializeLengths(mesh_list)),
	_vec_cumulative(InitializeCumulatives(mesh_list)),
	_vec_vs(MeshVs(mesh_list)),
	_vec_tau_refractive(vec_tau_refractive),
	_vec_mass(InitializeMass()),
	_vec_masses(InitializeMasses(0)),
	_vec_area(InitializeArea(mesh_list)),
	_t(0),
	_fs(std::vector<MPILib::Rate>(mesh_list.size(), 0.0)),
	_all_avs(std::vector<std::vector<MPILib::Potential>>(mesh_list.size())),
	_avs(std::vector<MPILib::Potential>(mesh_list.size(), 0.0)),
	_map(InitializeMap()),
	_linear_map(InitializeLinearMap()),
	_linear_unmap(InitializeLinearMap()),
	_map_counter(InitializeMap()),
	_vec_reversal(vec_reversal),
	_vec_reset(vec_reset),
	_reversal(InitializeReversal()),
	_object_reversal(InitializeObjectReversal()),
	_reset(InitializeReset()),
	_reset_refractive(InitializeResetRefractiveInternal(mesh_list[0].TimeStep())),
	_object_reset(InitializeObjectReset()),
	_clean(InitializeClean()),
	_vec_num_objects(num_objects),
	_vec_num_object_offsets(FiniteSizeOffset(num_objects))

{

	InitializeFiniteObjects();
	//TODO add this in
	InitializeFiniteObjectHistories(1);

	for (const auto& m : _mesh_list)
		assert(m.TimeStep() != 0.0);

	this->CheckConsistency();

	_vec_reset_sorted = std::vector<std::map<MPILib::Index, std::map<MPILib::Index, double>>>();

	for (int r = 0; r < _vec_reset.size(); r++) {
		std::map<MPILib::Index, std::map<MPILib::Index, double>> mapping = std::map<MPILib::Index, std::map<MPILib::Index, double>>();
		for (auto res : _vec_reset[r]) {
			if (mapping.count(Map(r, res._from[0], res._from[1])) == 0)
				mapping[Map(r, res._from[0], res._from[1])] = std::map<MPILib::Index, double>();

			mapping[Map(r, res._from[0], res._from[1])][Map(r, res._to[0], res._to[1])] = res._alpha;
		}
		_vec_reset_sorted.push_back(mapping);
	}

}

Ode2DSystemGroup::Ode2DSystemGroup
(
	const std::vector<Mesh>& mesh_list,
	const std::vector<std::vector<Redistribution> >& vec_reversal,
	const std::vector<std::vector<Redistribution> >& vec_reset
) : Ode2DSystemGroup(mesh_list, vec_reversal, vec_reset, std::vector<MPILib::Index>(mesh_list.size())) {}

Ode2DSystemGroup::Ode2DSystemGroup
(
	const std::vector<Mesh>& mesh_list,
	const std::vector<std::vector<Redistribution> >& vec_reversal,
	const std::vector<std::vector<Redistribution> >& vec_reset,
	const std::vector<MPILib::Time>& vec_tau_refractive
) : Ode2DSystemGroup(mesh_list, vec_reversal, vec_reset, vec_tau_refractive, std::vector<MPILib::Index>(mesh_list.size())) {}

Ode2DSystemGroup::Ode2DSystemGroup
(
	const std::vector<Mesh>& mesh_list,
	const std::vector<std::vector<Redistribution> >& vec_reversal,
	const std::vector<std::vector<Redistribution> >& vec_reset,
	const std::vector<MPILib::Index> num_objects

) : Ode2DSystemGroup(mesh_list, vec_reversal, vec_reset, std::vector<MPILib::Time>(mesh_list.size(), 0.0), num_objects) {}

std::vector<MPILib::Number> Ode2DSystemGroup::MeshOffset(const std::vector<Mesh>& l) const
{
	std::vector<MPILib::Number> vec_ret{ 0 }; // first offset is 0
	for (const Mesh& m : l) {
		MPILib::Number n_cell = 0;
		for (MPILib::Index i = 0; i < m.NrStrips(); i++)
			for (MPILib::Index j = 0; j < m.NrCellsInStrip(i); j++)
				n_cell++;
		vec_ret.push_back(n_cell + vec_ret.back());
	}

	return vec_ret;
}

std::vector<MPILib::Index> Ode2DSystemGroup::FiniteSizeOffset(const std::vector<MPILib::Index>& l) const {
	std::vector<MPILib::Index> offsets{ 0 };

	for (const MPILib::Index& i : l) {
		offsets.push_back(i + offsets.back());

	}

	return offsets;
}

std::vector<double> Ode2DSystemGroup::MeshVs(const std::vector<Mesh>& l) const
{
	std::vector<double> vec_ret; // first offset is 0
	for (const Mesh& m : l) {
		for (MPILib::Index i = 0; i < m.NrStrips(); i++) {
			for (MPILib::Index j = 0; j < m.NrCellsInStrip(i); j++) {
				double v = 0.0;
				for (double pv : m.Quad(i, j).getVecV())
					v += pv;
				vec_ret.push_back(v / 4.0);
			}
		}
	}

	return vec_ret;
}

std::vector<MPILib::Index> Ode2DSystemGroup::InitializeCumulative(const Mesh& m) const
{
	unsigned int sum = 0;
	vector<unsigned int> vec_ret;
	vec_ret.push_back(0);
	for (unsigned int i = 0; i < m.NrStrips(); i++) {
		sum += m.NrCellsInStrip(i);
		vec_ret.push_back(sum);
	}
	return vec_ret;
}

std::vector<std::vector<MPILib::Index> > Ode2DSystemGroup::InitializeCumulatives(const std::vector<Mesh>& mesh_list)
{
	std::vector<std::vector<MPILib::Index> > vec_ret;
	for (const Mesh& m : mesh_list)
		vec_ret.push_back(this->InitializeCumulative(m));
	return vec_ret;
}

vector<MPILib::Potential> Ode2DSystemGroup::InitializeMass() const
{
	MPILib::Number n_cells = 0;
	for (const std::vector<MPILib::Index>& v : _vec_cumulative)
		n_cells += v.back();

	return vector<MPILib::Potential>(n_cells, 0.0);
}




vector<vector<MPILib::Potential>> Ode2DSystemGroup::InitializeMasses(const unsigned int count) const
{

	vector<vector<MPILib::Potential>> histories;

	//TODO add this
	//if (count > 1) {
	for (int h = 0; h < count; h++) {
		histories.push_back(InitializeMass());
	}
	//}

	return histories;
}

void Ode2DSystemGroup::InitializeFiniteObjects() {
	_vec_cells_to_objects = vector<vector<MPILib::Index>>(_vec_mass.size());
	for (int i = 0; i < _vec_cells_to_objects.size(); i++) {
		_vec_cells_to_objects[i] = vector<MPILib::Index>();
	}

	unsigned int total_objects = 0;
	for (auto n : _vec_num_objects)
		total_objects += n;



	_vec_objects_to_index = vector<MPILib::Index>(total_objects);
	_vec_objects_refract_times = vector<double>(total_objects);
	_vec_objects_refract_index = vector<MPILib::Index>(total_objects);
	for (int i = 0; i < total_objects; i++) {
		_vec_objects_to_index[i] = 0;
		_vec_objects_refract_times[i] = -1.0;
		_vec_objects_refract_index[i] = 0;
	}

}




std::vector<MPILib::Index> Ode2DSystemGroup::InitializeWorkingIndex()
{
	std::vector<MPILib::Index> vec_ret;
	MPILib::Index counter = 0;
	for (const Mesh& mesh : _mesh_list) {
		for (MPILib::Index i = 0; i < mesh.NrStrips(); i++)
			for (MPILib::Index j = 0; j < mesh.NrCellsInStrip(i); j++)
				vec_ret.push_back(counter++);
	}
	return vec_ret;
}

std::vector<MPILib::Index> Ode2DSystemGroup::InitializeLength(const Mesh& m) const
{
	std::vector<MPILib::Index> vec_ret;
	for (unsigned int i = 0; i < m.NrStrips(); i++)
		vec_ret.push_back(m.NrCellsInStrip(i));
	return vec_ret;
}

std::vector< std::vector<MPILib::Index> > Ode2DSystemGroup::InitializeLengths(const std::vector<Mesh>& list)
{
	std::vector< std::vector<MPILib::Index> > vec_ret;
	for (const Mesh& m : list)
		vec_ret.push_back(this->InitializeLength(m));

	return vec_ret;
}


vector<MPILib::Potential> Ode2DSystemGroup::InitializeArea(const std::vector<Mesh>& vec) const
{
	vector<MPILib::Potential> vec_ret;
	for (const Mesh& m : _mesh_list) {
		for (unsigned int i = 0; i < m.NrStrips(); i++)
			for (unsigned int j = 0; j < m.NrCellsInStrip(i); j++)
				vec_ret.push_back(m.Quad(i, j).SignedArea());
	}
	return vec_ret;
}

void Ode2DSystemGroup::Initialize(MPILib::Index m, MPILib::Index i, MPILib::Index j) {
	unsigned int start_index = this->Map(m, i, j);

	_vec_mass[start_index] = 1.0;

	//histories
	if (_vec_masses.size() > 1) {
		for (int i = 0; i < _vec_masses.size(); i++) {
			_vec_masses[i][start_index] = 1.0;
		}
	}


	for (int i = 0; i < _vec_num_objects[m]; i++) {
		_vec_objects_to_index[i + _vec_num_object_offsets[m]] = start_index;
		_vec_objects_refract_times[i + _vec_num_object_offsets[m]] = -1.0;
		_vec_objects_refract_index[i + _vec_num_object_offsets[m]] = 0;
		_vec_cells_to_objects[start_index].push_back(i + _vec_num_object_offsets[m]);
	}

	for (int j = 0; j < _vec_masses.size(); j++) {
		for (int i = 0; i < _vec_num_objects[m]; i++) {
			_vec_vec_objects_to_index[j][i + _vec_num_object_offsets[m]] = start_index;
			_vec_vec_objects_refract_times[j][i + _vec_num_object_offsets[m]] = -1.0;
			_vec_vec_objects_refract_index[j][i + _vec_num_object_offsets[m]] = 0;
		}
	}

}

void Ode2DSystemGroup::InitializeFiniteObjectHistories(const unsigned int count) {
	for (unsigned int i = 0; i < count; i++) {
		_vec_vec_objects_to_index.push_back(_vec_objects_to_index);
		_vec_vec_objects_refract_times.push_back(_vec_objects_refract_times);
		_vec_vec_objects_refract_index.push_back(_vec_objects_refract_index);
	}
}

std::vector< std::vector< std::vector<MPILib::Index> > > Ode2DSystemGroup::InitializeMap() const
{
	std::vector< std::vector<std::vector<MPILib::Index> > > vec_map;

	MPILib::Index count = 0;
	for (const Mesh& mesh : _mesh_list) {
		std::vector<std::vector<MPILib::Index> > vec_mesh;
		for (MPILib::Index i = 0; i < mesh.NrStrips(); i++) {
			std::vector<MPILib::Index> vec_strip;
			for (MPILib::Index j = 0; j < mesh.NrCellsInStrip(i); j++) {
				vec_strip.push_back(count++);
			}
			vec_mesh.push_back(vec_strip);
		}
		vec_map.push_back(vec_mesh);
	}
	return vec_map;
}

std::vector<MPILib::Index> Ode2DSystemGroup::InitializeLinearMap()
{
	std::vector<MPILib::Index> vec_ret;
	MPILib::Index counter = 0;
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		for (MPILib::Index i = 0; i < _mesh_list[m].NrStrips(); i++) {
			for (MPILib::Index j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
				vec_ret.push_back(counter++);
			}
		}
	}
	return vec_ret;
}

void Ode2DSystemGroup::Dump(const std::vector<std::ostream*>& vecost, int mode) const
{
	assert(vecost.size() == _mesh_list.size());
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		vecost[m]->precision(10);
		if (mode == 0) {
			for (unsigned int i = 0; i < _mesh_list[m].NrStrips(); i++)
				for (unsigned int j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++)
					// a division by _vec_area[this->Map(i,j)] is wrong
					// the fabs is required since we don't care about the sign of the area and
					// must write out a positive density
					(*vecost[m]) << i << "\t" << j << "\t" << " " << std::abs(_vec_mass[this->Map(m, i, j)] / _mesh_list[m].Quad(i, j).SignedArea()) << "\t";
		}
		else {
			for (unsigned int i = 0; i < _mesh_list[m].NrStrips(); i++)
				for (unsigned int j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++)
					(*vecost[m]) << i << "\t" << j << "\t" << " " << _vec_mass[this->Map(m, i, j)] << "\t";
		}
	}
}

void Ode2DSystemGroup::DumpSingleMesh(std::ostream* vecost, unsigned int m, int mode) const
{
	vecost->precision(10);
	if (mode == 0) {
		for (unsigned int i = 0; i < _mesh_list[m].NrStrips(); i++)
			for (unsigned int j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++)
				// a division by _vec_area[this->Map(i,j)] is wrong
				// the fabs is required since we don't care about the sign of the area and
				// must write out a positive density
				(*vecost) << i << "\t" << j << "\t" << " " << std::abs(_vec_mass[this->Map(m, i, j)] / _mesh_list[m].Quad(i, j).SignedArea()) << "\t";
	}
	else {
		for (unsigned int i = 0; i < _mesh_list[m].NrStrips(); i++)
			for (unsigned int j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++)
				(*vecost) << i << "\t" << j << "\t" << " " << _vec_mass[this->Map(m, i, j)] << "\t";
	}
}

void Ode2DSystemGroup::Evolve()
{
	EvolveWithoutMeshUpdate();
	this->UpdateMap();
}

void Ode2DSystemGroup::Evolve(std::vector<MPILib::Index>& meshes)
{
	EvolveWithoutMeshUpdate();
	this->UpdateMap(meshes);
}

void Ode2DSystemGroup::EvolveWithoutMeshUpdate() {
	_t += 1;
	for (MPILib::Rate& f : _fs)
		f = 0.;
}

void Ode2DSystemGroup::ShiftHistories(unsigned int count) {
	std::cout << "longest kernel " << count << std::endl;

	if (_vec_masses.size() > 0) {
		//shift histories
		for (int history = _vec_masses.size() - 1; history > 0; history--) {
			_vec_masses.at(history) = _vec_masses.at(history - 1);
		}
		//add most recent history
		_vec_masses.at(0) = _vec_mass;
	}

	if (_vec_masses.size() < count) {
		_vec_masses.insert(_vec_masses.begin(), _vec_mass);
	}
}

void Ode2DSystemGroup::ShiftFiniteObjectHistories() {
	// shift histories
	for (unsigned int i = 1; i < _vec_masses.size(); i++) {
		_vec_vec_objects_to_index.at(i) = _vec_vec_objects_to_index.at(i - 1);
		_vec_vec_objects_refract_times.at(i) = _vec_vec_objects_refract_times.at(i - 1);
		//_vec_vec_objects_refract_index.at(i) = _vec_vec_objects_refract_index.at(i - 1);
	}

	// set new history
	_vec_vec_objects_to_index[0] = _vec_objects_to_index;
	_vec_vec_objects_refract_times[0] = _vec_objects_refract_times;
	//_vec_vec_objects_refract_index[0] = _vec_objects_refract_index;

}

std::vector<MPILib::Index> Ode2DSystemGroup::BuildMapCumulatives() {
	std::vector<MPILib::Index> cumulatives;

	for (unsigned int m = 0; m < _mesh_list.size(); m++) {
		for (MPILib::Index i = 0; i < _mesh_list[m].NrStrips(); i++) {
			for (MPILib::Index j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
				cumulatives.push_back(_vec_cumulative[m][i]);
			}
		}
	}

	return cumulatives;
}

std::vector<MPILib::Index> Ode2DSystemGroup::BuildMapLengths() {
	std::vector<MPILib::Index> lengths;

	for (unsigned int m = 0; m < _mesh_list.size(); m++) {
		for (MPILib::Index j = 0; j < _mesh_list[m].NrCellsInStrip(0); j++) {
			lengths.push_back(0);
		}
		for (MPILib::Index i = 1; i < _mesh_list[m].NrStrips(); i++) {
			for (MPILib::Index j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
				lengths.push_back(_vec_length[m][i]);
			}
		}
	}

	return lengths;
}


void Ode2DSystemGroup::UpdateMap()
{
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) { // we need the index for mapping, so no range-based loop
		for (MPILib::Index i_stat = 0; i_stat < _mesh_list[m].NrCellsInStrip(0); i_stat++) {
			_linear_map[_map_counter[m][0][i_stat]] = i_stat + _vec_mesh_offset[m]; // the stationary strip needs to be handled separately
			_linear_unmap[i_stat + _vec_mesh_offset[m]] = _map_counter[m][0][i_stat];
		}
#pragma omp parallel for
		for (int i = 1; i < _mesh_list[m].NrStrips(); i++) {
			// yes! i = 1. strip 0 is not supposed to have dynamics
			for (int j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
				MPILib::Index ind = _vec_cumulative[m][i] + modulo(j - _t, _vec_length[m][i]) + _vec_mesh_offset[m];
				_map[m][i][j] = ind;
				_linear_map[_map_counter[m][i][j]] = ind;
				_linear_unmap[ind] = _map_counter[m][i][j];
			}
		}
	}
}

void Ode2DSystemGroup::UpdateMap(std::vector<MPILib::Index>& meshes)
{
	for (MPILib::Index n = 0; n < meshes.size(); n++) { // we need the index for mapping, so no range-based loop
		MPILib::Index m = meshes[n];
		for (MPILib::Index i_stat = 0; i_stat < _mesh_list[m].NrCellsInStrip(0); i_stat++) {
			_linear_map[_map_counter[m][0][i_stat]] = i_stat + _vec_mesh_offset[m]; // the stationary strip needs to be handled separately
			_linear_unmap[i_stat + _vec_mesh_offset[m]] = _map_counter[m][0][i_stat];
		}
#pragma omp parallel for
		for (int i = 1; i < _mesh_list[m].NrStrips(); i++) {
			// yes! i = 1. strip 0 is not supposed to have dynamics
			for (int j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
				MPILib::Index ind = _vec_cumulative[m][i] + modulo(j - _t, _vec_length[m][i]) + _vec_mesh_offset[m];
				_map[m][i][j] = ind;
				_linear_map[_map_counter[m][i][j]] = ind;
				_linear_unmap[ind] = _map_counter[m][i][j];
			}
		}
	}
}

void Ode2DSystemGroup::RemapReversal() {

	if (_vec_num_objects[0] > 0) {
		RemapObjectReversal();
		return;
	}

	for (MPILib::Index m = 0; m < _mesh_list.size(); m++)
		std::for_each(_vec_reversal[m].begin(), _vec_reversal[m].end(), _reversal[m]);
}

void Ode2DSystemGroup::RemapObjectReversal() {
	// requires valid _vec_cells_to_objects and _vec_objects_to_index
	// _vec_cells_to_objects is valid at the end of this
	// _vec_objects_to_index is valid at the end of this
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++)
		std::for_each(_vec_reversal[m].begin(), _vec_reversal[m].end(), _object_reversal[m]);

}

void Ode2DSystemGroup::RedistributeObjects(MPILib::Time timestep) {
	// requires valid _vec_cells_to_objects and _vec_objects_to_index
	// _vec_cells_to_objects is valid at the end of this
	// _vec_objects_to_index is valid at the end of this
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		std::for_each(_vec_reset[m].begin(), _vec_reset[m].end(), _object_reset[m]);

		for (int i = 0; i < _vec_objects_refract_times.size(); i++) {
			if (_vec_objects_refract_times[i] >= 0) {
				_vec_objects_refract_times[i] -= timestep;
				if (_vec_objects_refract_times[i] <= 0) { // ok reset
					_vec_objects_refract_times[i] = -1.0;
					double r1 = ((double)rand() / (double)RAND_MAX);
					double check = 0.0;
					std::map<MPILib::Index, double> trans = _vec_reset_sorted[m].at(_vec_objects_refract_index[i]);
					for (auto const& t : trans) {
						check += t.second;
						if (r1 <= check) {
							_fs[m]++;
							auto ind = Map(t.first);
							_vec_objects_to_index[i] = ind;
							_vec_cells_to_objects[ind].push_back(i);
							break;
						}
					}
				}
			}

		}
	}
}

void Ode2DSystemGroup::RedistributeProbability(MPILib::Number steps)
{
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {

		MPILib::Time t_step = _mesh_list[m].TimeStep();

		if (_vec_num_objects[m] > 0) {
			RedistributeObjects(t_step);
			_fs[m] /= _vec_num_objects[m] * t_step * steps;
		}
		else {
			if (_vec_tau_refractive[m] == 0.) {
				std::for_each(_vec_reset[m].begin(), _vec_reset[m].end(), _reset[m]);
			}
			else {
				for (auto& r : _vec_reset[m])
					_reset_refractive[m](r);
			}

			std::for_each(_vec_reset[m].begin(), _vec_reset[m].end(), _clean[m]);

			_fs[m] /= t_step * steps;
		}


	}
}

void Ode2DSystemGroup::RedistributeProbability()
{
	RedistributeProbability(1);
}

const std::vector<MPILib::Potential>& Ode2DSystemGroup::AvgV() const
{
	// Rate calculation for non-threshold crossing models such as Fitzhugh-Nagumo
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		MPILib::Potential av = 0.;
		for (MPILib::Index i = 0; i < _mesh_list[m].NrStrips(); i++) {
			for (MPILib::Index j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
				MPILib::Potential V = _mesh_list[m].Quad(i, j).Centroid()[0];
				av += V * _vec_mass[this->Map(m, i, j)];
			}
		}
		const_cast<MPILib::Potential&>(_avs[m]) = av;
	}
	return _avs;
}

const std::vector<std::vector<MPILib::Potential>>& Ode2DSystemGroup::Avgs() const
{
	// Rate calculation for non-threshold crossing models such as Fitzhugh-Nagumo
	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		Avgs(m);
	}
	return _all_avs;
}

const std::vector<MPILib::Potential>& Ode2DSystemGroup::Avgs(unsigned int m) const
{
	if (this->FiniteSizeNumObjects()[m] == 0) {
		if (_mesh_list[m].hasDefinedStrips()) { // this is a mesh or an old style grid

			MPILib::Potential av = 0.;
			MPILib::Potential aw = 0.;
			for (MPILib::Index i = 0; i < _mesh_list[m].NrStrips(); i++) {
				for (MPILib::Index j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
					MPILib::Potential V = _mesh_list[m].Quad(i, j).Centroid()[0];
					MPILib::Potential W = _mesh_list[m].Quad(i, j).Centroid()[1];
					av += V * _vec_mass[this->Map(m, i, j)];
					aw += W * _vec_mass[this->Map(m, i, j)];
				}
			}
			if (_all_avs[m].size() == 0) {
				const_cast<std::vector<MPILib::Potential>&>(_all_avs[m]) = std::vector<MPILib::Potential>(2);
			}

			const_cast<MPILib::Potential&>(_all_avs[m][0]) = av;
			const_cast<MPILib::Potential&>(_all_avs[m][1]) = aw;
		}
		else { // this is a new-fangled grid
			std::vector<MPILib::Potential> av(_mesh_list[m].getGridNumDimensions());

			double mass_sum = 0.0;
			for (MPILib::Index i = 0; i < _vec_mesh_offset[m + 1] - _vec_mesh_offset[m]; i++) {
				std::vector<MPILib::Potential> V = _mesh_list[m].Centroid(i);
				for (unsigned int d = 0; d < _mesh_list[m].getGridNumDimensions(); d++) {
					av[d] += V[d] * _vec_mass[_vec_mesh_offset[m] + i];
				}
				mass_sum += _vec_mass[_vec_mesh_offset[m] + i];
			}

			av[_mesh_list[m].getGridThresholdResetDirection()] += _mesh_list[m].getReset() * (1.0 - mass_sum);

			if (_all_avs[m].size() == 0) {
				const_cast<std::vector<MPILib::Potential>&>(_all_avs[m]) = std::vector<MPILib::Potential>(_mesh_list[m].getGridNumDimensions());
			}
			for (unsigned int d = 0; d < _mesh_list[m].getGridNumDimensions(); d++) {
				const_cast<MPILib::Potential&>(_all_avs[m][d]) = av[d];
			}
		}
	}
	else { // finite size
		if (_mesh_list[m].hasDefinedStrips()) { // this is a mesh or an old style grid
			MPILib::Potential av = 0.;
			MPILib::Potential aw = 0.;

			for (MPILib::Index i = 0; i < _mesh_list[m].NrStrips(); i++) {
				for (MPILib::Index j = 0; j < _mesh_list[m].NrCellsInStrip(i); j++) {
					MPILib::Potential V = _mesh_list[m].Quad(i, j).Centroid()[0];
					MPILib::Potential W = _mesh_list[m].Quad(i, j).Centroid()[1];
					av += V * this->_vec_cells_to_objects[this->Map(m, i, j)].size();
					aw += W * this->_vec_cells_to_objects[this->Map(m, i, j)].size();
				}
			}
			if (_all_avs[m].size() == 0) {
				const_cast<std::vector<MPILib::Potential>&>(_all_avs[m]) = std::vector<MPILib::Potential>(2);
			}

			const_cast<MPILib::Potential&>(_all_avs[m][0]) = av / this->FiniteSizeNumObjects()[m];
			const_cast<MPILib::Potential&>(_all_avs[m][1]) = aw / this->FiniteSizeNumObjects()[m];
		}
		else { // this is a new-fangled grid
			std::vector<MPILib::Potential> av(_mesh_list[m].getGridNumDimensions());

			for (MPILib::Index i = 0; i < _vec_num_objects[m]; i++) {
				if (_vec_objects_refract_times[i + this->_vec_num_object_offsets[m]] > 0) {
					av[_mesh_list[m].getGridThresholdResetDirection()] += _mesh_list[m].getReset();
					continue;
				}

				std::vector<MPILib::Potential> V = _mesh_list[m].Centroid(this->_vec_objects_to_index[i + this->_vec_num_object_offsets[m]]);
				for (unsigned int d = 0; d < _mesh_list[m].getGridNumDimensions(); d++) {
					av[d] += V[d];
				}
			}

			if (_all_avs[m].size() == 0) {
				const_cast<std::vector<MPILib::Potential>&>(_all_avs[m]) = std::vector<MPILib::Potential>(_mesh_list[m].getGridNumDimensions());
			}
			for (unsigned int d = 0; d < _mesh_list[m].getGridNumDimensions(); d++) {
				const_cast<MPILib::Potential&>(_all_avs[m][d]) = av[d] / this->FiniteSizeNumObjects()[m];
			}
		}
	}

	return _all_avs[m];
}

bool Ode2DSystemGroup::CheckConsistency() const {

	std::ostringstream ost_err;
	ost_err << "Mesh inconsistent with mapping: ";
	// it is allowed to have no reversal and reset mappings
	if (_vec_reversal.size() == 0 && _vec_reset.size() == 0)
		return true;
	else // but if you have them, they must match the mesh list
	{
		if (_vec_reset.size() != _mesh_list.size()) {
			ost_err << "Reset mapping vector size does not match mesh list size";
			return false;
		}
		if (_vec_reversal.size() != _mesh_list.size()) {
			ost_err << "Reversal mapping vector size does not match mesh list size";
			return false;
		}
	}

	for (MPILib::Index m = 0; m < _mesh_list.size(); m++) {
		for (const Redistribution& r : _vec_reversal[m]) {
			if (r._from[0] >= _mesh_list[m].NrStrips()) {
				ost_err << "reversal. NrStrips: " << _mesh_list[m].NrStrips() << ", from: " << r._from[0];
				throw TwoDLib::TwoDLibException(ost_err.str());
			}
			if (r._from[1] >= _mesh_list[m].NrCellsInStrip(r._from[0])) {
				ost_err << "reversal. Nr cells in strip from: " << _mesh_list[m].NrCellsInStrip(r._from[0]) << ",from: " << r._from[0] << "\n";
				ost_err << "In total there are: " << _mesh_list[m].NrStrips() << " strips." << std::endl;
				throw TwoDLib::TwoDLibException(ost_err.str());
			}
		}

		for (const Redistribution& r : _vec_reset[m]) {
			if (r._from[0] >= _mesh_list[m].NrStrips()) {
				ost_err << "reset. NrStrips: " << _mesh_list[m].NrStrips() << ", from: " << r._from[0] << std::endl;
				throw TwoDLib::TwoDLibException(ost_err.str());
			}
			if (r._from[1] >= _mesh_list[m].NrCellsInStrip(r._from[0])) {
				ost_err << "reset. Nr cells in strip r._from[0]: " << _mesh_list[m].NrCellsInStrip(r._from[0]) << ", from: " << r._from[1];
				throw TwoDLib::TwoDLibException(ost_err.str());
			}
		}
	}
	return true;
}

