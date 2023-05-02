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

#include <iostream>
#include <cuda_runtime.h>
#include <cstdio>
#include <cmath>
#include "CudaEuler.cuh"
#include "CSRAdapter.cuh"

using namespace CudaTwoDLib;

const fptype TOLERANCE = 1e-9;


#define checkCudaErrors(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char* file, int line, bool abort = true) {
    if (code != cudaSuccess) {
        fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
        if (abort) exit(code);
    }
}

void CSRAdapter::FillMatrixMaps(const std::vector<TwoDLib::CSRMatrix>& vecmat)
{
    for (inttype m = 0; m < vecmat.size(); m++)
    {
        _nval[m] = vecmat[m].Val().size();
        checkCudaErrors(cudaMalloc((fptype**)&_val[m], _nval[m] * sizeof(fptype)));
        // dont't depend on Val() being of fptype
        std::vector<fptype> vecval;
        for (fptype val : vecmat[m].Val())
            vecval.push_back(val);
        checkCudaErrors(cudaMemcpy(_val[m], &vecval[0], sizeof(fptype) * _nval[m], cudaMemcpyHostToDevice));

        _nia[m] = vecmat[m].Ia().size();
        checkCudaErrors(cudaMalloc((inttype**)&_ia[m], _nia[m] * sizeof(inttype)));
        std::vector<inttype> vecia;
        for (inttype ia : vecmat[m].Ia())
            vecia.push_back(ia);
        checkCudaErrors(cudaMemcpy(_ia[m], &vecia[0], sizeof(inttype) * _nia[m], cudaMemcpyHostToDevice));


        _nja[m] = vecmat[m].Ja().size();
        checkCudaErrors(cudaMalloc((inttype**)&_ja[m], _nja[m] * sizeof(inttype)));
        std::vector<inttype> vecja;
        for (inttype ja : vecmat[m].Ja())
            vecja.push_back(ja);
        checkCudaErrors(cudaMemcpy(_ja[m], &vecja[0], sizeof(inttype) * _nja[m], cudaMemcpyHostToDevice));
    }
}

void CSRAdapter::FillForwardMatrixMaps(const std::vector<TwoDLib::CSRMatrix>& vecmat)
{
    for (inttype m = 0; m < vecmat.size(); m++)
    {
        checkCudaErrors(cudaMalloc((fptype**)&_forward_val[m], vecmat[m].ForwardVal().size() * sizeof(fptype)));
        // dont't depend on Val() being of fptype
        std::vector<fptype> vecval;
        for (fptype val : vecmat[m].ForwardVal())
            vecval.push_back(val);
        checkCudaErrors(cudaMemcpy(_forward_val[m], &vecval[0], sizeof(fptype) * vecmat[m].ForwardVal().size(), cudaMemcpyHostToDevice));

        checkCudaErrors(cudaMalloc((inttype**)&_forward_ia[m], vecmat[m].ForwardIa().size() * sizeof(inttype)));
        std::vector<inttype> vecia;
        for (inttype ia : vecmat[m].ForwardIa())
            vecia.push_back(ia);
        checkCudaErrors(cudaMemcpy(_forward_ia[m], &vecia[0], sizeof(inttype) * vecmat[m].ForwardIa().size(), cudaMemcpyHostToDevice));

        checkCudaErrors(cudaMalloc((inttype**)&_forward_ja[m], vecmat[m].ForwardJa().size() * sizeof(inttype)));
        std::vector<inttype> vecja;
        for (inttype ja : vecmat[m].ForwardJa())
            vecja.push_back(ja);
        checkCudaErrors(cudaMemcpy(_forward_ja[m], &vecja[0], sizeof(inttype) * vecmat[m].ForwardJa().size(), cudaMemcpyHostToDevice));
    }
}

void CSRAdapter::InitializeStaticGridEfficacies(const std::vector<inttype>& vecindex, const std::vector<fptype>& efficacy, const std::vector<fptype>& cell_width, const std::vector<inttype>& grid_efficacy_offset) {
    _nr_grid_connections = efficacy.size();
    for (inttype m = 0; m < efficacy.size(); m++)
    {
        checkCudaErrors(cudaMalloc((fptype**)&_goes[m], _nr_rows[vecindex[m]] * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((fptype**)&_stays[m], _nr_rows[vecindex[m]] * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((inttype**)&_offset1s[m], _nr_rows[vecindex[m]] * sizeof(inttype)));
        checkCudaErrors(cudaMalloc((inttype**)&_offset2s[m], _nr_rows[vecindex[m]] * sizeof(inttype)));

        inttype numBlocks = (_nr_rows[vecindex[m]] + _blockSize - 1) / _blockSize;

        CudaCalculateGridEfficacies << <numBlocks, _blockSize >> > (_nr_rows[vecindex[m]],
            efficacy[m], cell_width[m], grid_efficacy_offset[m],
            _stays[m], _goes[m], _offset1s[m], _offset2s[m]);
    }
}

void CSRAdapter::InitializeStaticGridCellEfficacies(const std::vector<inttype>& vecindex, const std::vector<std::vector<fptype>>& vals, const std::vector<fptype>& cell_width, const std::vector<inttype>& grid_efficacy_offset) {
    _nr_grid_connections = vals.size();
    for (inttype m = 0; m < vals.size(); m++)
    {
        checkCudaErrors(cudaMalloc((fptype**)&_goes[m], _nr_rows[vecindex[m]] * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((fptype**)&_stays[m], _nr_rows[vecindex[m]] * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((inttype**)&_offset1s[m], _nr_rows[vecindex[m]] * sizeof(inttype)));
        checkCudaErrors(cudaMalloc((inttype**)&_offset2s[m], _nr_rows[vecindex[m]] * sizeof(inttype)));
        checkCudaErrors(cudaMalloc((fptype**)&_cell_vals[m], vals[m].size() * sizeof(fptype)));
        checkCudaErrors(cudaMemcpy(_cell_vals[m], &vals[m][0], vals[m].size() * sizeof(fptype), cudaMemcpyHostToDevice));

        inttype numBlocks = (_nr_rows[vecindex[m]] + _blockSize - 1) / _blockSize;

        CudaCalculateGridCellEfficacies << <numBlocks, _blockSize >> > (_nr_rows[vecindex[m]],
            _cell_vals[m], cell_width[m], grid_efficacy_offset[m],
            _stays[m], _goes[m], _offset1s[m], _offset2s[m], _offsets[vecindex[m]]);
    }
}

void CSRAdapter::InitializeStaticGridCellCsrNd(const std::vector<inttype>& vecindex, const std::vector<TwoDLib::CSRMatrix>& mats) {

    _nr_grid_connections = mats.size();

    for (inttype m = 0; m < _nr_grid_connections; m++) {

        // yikes, convert the array of doubles to fptypes
        std::vector<fptype> d2f(mats[m].Val().size());
        for (int f = 0; f < mats[m].Val().size(); f++)
            d2f[f] = (fptype)mats[m].Val()[f];

        std::vector<fptype> fd2f(mats[m].ForwardVal().size());
        for (int f = 0; f < mats[m].ForwardVal().size(); f++)
            fd2f[f] = (fptype)mats[m].ForwardVal()[f];

        checkCudaErrors(cudaMalloc((fptype**)&_grid_val[m], d2f.size() * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((inttype**)&_grid_ia[m], mats[m].Ia().size() * sizeof(inttype)));
        checkCudaErrors(cudaMalloc((inttype**)&_grid_ja[m], mats[m].Ja().size() * sizeof(inttype)));
        checkCudaErrors(cudaMalloc((fptype**)&_grid_forward_val[m], fd2f.size() * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((inttype**)&_grid_forward_ia[m], mats[m].ForwardIa().size() * sizeof(inttype)));
        checkCudaErrors(cudaMalloc((inttype**)&_grid_forward_ja[m], mats[m].ForwardJa().size() * sizeof(inttype)));


        checkCudaErrors(cudaMemcpy(_grid_val[m], &d2f[0], d2f.size() * sizeof(fptype), cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(_grid_ia[m], &mats[m].Ia()[0], mats[m].Ia().size() * sizeof(inttype), cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(_grid_ja[m], &mats[m].Ja()[0], mats[m].Ja().size() * sizeof(inttype), cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(_grid_forward_val[m], &fd2f[0], fd2f.size() * sizeof(fptype), cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(_grid_forward_ia[m], &mats[m].ForwardIa()[0], mats[m].ForwardIa().size() * sizeof(inttype), cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(_grid_forward_ja[m], &mats[m].ForwardJa()[0], mats[m].ForwardJa().size() * sizeof(inttype), cudaMemcpyHostToDevice));
    }

}

void CSRAdapter::InitializeStaticGridConductanceEfficacies(const std::vector<inttype>& vecindex,
    const std::vector<fptype>& efficacy, const std::vector<fptype>& cell_widths, const std::vector<inttype>& cell_offsets, const std::vector<fptype>& rest_vs) {
    _nr_grid_connections = efficacy.size();

    checkCudaErrors(cudaMalloc((fptype**)&_cell_vs, _group.getGroup().Vs().size() * sizeof(fptype)));

    std::vector<fptype> vecval;
    for (double val : _group.getGroup().Vs())
        vecval.push_back((fptype)val);

    checkCudaErrors(cudaMemcpy(_cell_vs, &vecval[0], _group.getGroup().Vs().size() * sizeof(fptype), cudaMemcpyHostToDevice));

    for (inttype m = 0; m < efficacy.size(); m++)
    {
        checkCudaErrors(cudaMalloc((fptype**)&_goes[m], _nr_rows[vecindex[m]] * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((fptype**)&_stays[m], _nr_rows[vecindex[m]] * sizeof(fptype)));
        checkCudaErrors(cudaMalloc((inttype**)&_offset1s[m], _nr_rows[vecindex[m]] * sizeof(inttype)));
        checkCudaErrors(cudaMalloc((inttype**)&_offset2s[m], _nr_rows[vecindex[m]] * sizeof(inttype)));

        inttype numBlocks = (_nr_rows[vecindex[m]] + _blockSize - 1) / _blockSize;

        CudaCalculateGridEfficaciesWithConductance << <numBlocks, _blockSize >> > (_nr_rows[vecindex[m]],
            efficacy[m], cell_widths[m], cell_offsets[m], _cell_vs, rest_vs[m],
            _stays[m], _goes[m], _offset1s[m], _offset2s[m], _offsets[vecindex[m]]);
    }
}


void CSRAdapter::DeleteMatrixMaps()
{
    for (inttype m = 0; m < _nr_m; m++)
    {
        cudaFree(_val[m]);
        cudaFree(_ia[m]);
        cudaFree(_ja[m]);
    }
}

void CSRAdapter::DeleteForwardMatrixMaps()
{
    for (inttype m = 0; m < _nr_m; m++)
    {
        cudaFree(_forward_val[m]);
        cudaFree(_forward_ia[m]);
        cudaFree(_forward_ja[m]);
    }
}

inttype CSRAdapter::NumberIterations(const CudaOde2DSystemAdapter& group, fptype euler_timestep) const
{
    fptype tstep = group._group.MeshObjects()[0].TimeStep();
    for (const auto& mesh : group._group.MeshObjects())
        if (fabs(tstep - mesh.TimeStep()) > TOLERANCE) {
            std::cerr << "Not all meshes in this group have the same time step. " << tstep << " " << mesh.TimeStep() << " " << tstep - mesh.TimeStep() << std::endl;
            exit(0);
        }
    inttype  n_steps = static_cast<inttype>(std::round(tstep / euler_timestep));

    return n_steps;
}

void CSRAdapter::InspectMass(inttype i)
{
    std::vector<fptype> hostvec(_group._n);
    checkCudaErrors(cudaMemcpy(&hostvec[0], _group._mass, sizeof(fptype) * _group._n, cudaMemcpyDeviceToHost));
}

CSRAdapter::CSRAdapter(CudaOde2DSystemAdapter& group, const std::vector<TwoDLib::CSRMatrix>& vecmat,
    inttype nr_grid_connections, fptype euler_timestep,
    const std::vector<inttype>& vecmat_indexes, const std::vector<inttype>& grid_transforms) :
    _group(group),
    _euler_timestep(euler_timestep),
    _nr_iterations(NumberIterations(group, euler_timestep)),
    _nr_m(vecmat.size()),
    _nr_streams(vecmat.size()),
    _vecmats(vecmat_indexes),
    _grid_transforms(grid_transforms),
    _nval(std::vector<inttype>(vecmat.size())),
    _val(std::vector<fptype*>(vecmat.size())),
    _forward_val(std::vector<fptype*>(vecmat.size())),
    _nia(std::vector<inttype>(vecmat.size())),
    _ia(std::vector<inttype*>(vecmat.size())),
    _forward_ia(std::vector<inttype*>(vecmat.size())),
    _nja(std::vector<inttype>(vecmat.size())),
    _ja(std::vector<inttype*>(vecmat.size())),
    _forward_ja(std::vector<inttype*>(vecmat.size())),
    _offsets(this->Offsets(vecmat)),
    _nr_rows(this->NrRows(vecmat)),
    _goes(nr_grid_connections),
    _stays(nr_grid_connections),
    _offset1s(nr_grid_connections),
    _offset2s(nr_grid_connections),
    _cell_vals(nr_grid_connections),
    _grid_val(nr_grid_connections),
    _grid_ia(nr_grid_connections),
    _grid_ja(nr_grid_connections),
    _grid_forward_val(nr_grid_connections),
    _grid_forward_ia(nr_grid_connections),
    _grid_forward_ja(nr_grid_connections),
    _blockSize(256),
    _numBlocks((_group._n + _blockSize - 1) / _blockSize)
{
    this->FillMatrixMaps(vecmat);
    this->FillForwardMatrixMaps(vecmat);
    this->FillDerivative();
    this->CreateStreams();
    this->FillRandom();
    // Speed Testing for comparison - Izhikevich neurons on the GPU
#ifdef IZHIKEVICH_TEST
    this->FillIzhVectors();
#endif
}

// Speed Testing for comparison - Izhikevich neurons on the GPU
void CSRAdapter::FillIzhVectors() {
    checkCudaErrors(cudaMalloc((fptype**)&_izh_vs, _group.NumObjects() * sizeof(fptype)));
    checkCudaErrors(cudaMalloc((fptype**)&_izh_ws, _group.NumObjects() * sizeof(fptype)));
    checkCudaErrors(cudaMalloc((fptype**)&_refract_times, _group.NumObjects() * sizeof(fptype)));

    std::vector<fptype> vs(_group.NumObjects(), -70.0);
    std::vector<fptype> ws(_group.NumObjects(), 0.0);
    std::vector<fptype> refract_times(_group.NumObjects(), -1.0);

    checkCudaErrors(cudaMemcpy(_izh_vs, &vs[0], _group.NumObjects() * sizeof(fptype), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(_izh_ws, &ws[0], _group.NumObjects() * sizeof(fptype), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(_refract_times, &refract_times[0], _group.NumObjects() * sizeof(fptype), cudaMemcpyHostToDevice));
}

// Speed Testing for comparison - Izhikevich neurons on the GPU
void CSRAdapter::IzhTest(inttype* spikes) {
    inttype numBlocks = (_group.NumObjects() + _blockSize - 1) / _blockSize;
    generatePoissonSpikes << <numBlocks, _blockSize >> > (_group.NumObjects(), 0, 5000 * 2, 0.0001, _random_poisson, _randomState);

    CudaSolveIzhikevichNeurons << <numBlocks, _blockSize >> > (_group.NumObjects(), _random_poisson, spikes, _izh_vs, _izh_ws, _refract_times, 0.0, 0.0001, _randomState);

}

CSRAdapter::CSRAdapter(CudaOde2DSystemAdapter& group, const std::vector<TwoDLib::CSRMatrix>& vecmat, fptype euler_timestep) :
    CSRAdapter(group, vecmat, vecmat.size(), euler_timestep,
        std::vector<inttype>(), std::vector<inttype>()) {
    for (unsigned int i = 0; i < vecmat.size(); i++)
        _vecmats.push_back(i);
}

CSRAdapter::~CSRAdapter()
{
#ifdef IZHIKEVICH_TEST
    free(_izh_vs);
    free(_izh_ws);
    free(_refract_times);
#endif

    cudaFree(_cell_vs);

    for (inttype m = 0; m < _nr_grid_connections; m++) {
        cudaFree(_grid_val[m]);
        cudaFree(_grid_ia[m]);
        cudaFree(_grid_ja[m]);
        cudaFree(_grid_forward_val[m]);
        cudaFree(_grid_forward_ia[m]);
        cudaFree(_grid_forward_ja[m]);
        cudaFree(_cell_vals[m]);
        cudaFree(_goes[m]);
        cudaFree(_stays[m]);
        cudaFree(_offset1s[m]);
        cudaFree(_offset2s[m]);
    }

    this->DeleteMatrixMaps();
    this->DeleteForwardMatrixMaps();
    this->DeleteDerivative();
    this->DeleteStreams();
    this->DeleteRandom();
}

void CSRAdapter::CreateStreams()
{
    _streams = (cudaStream_t*)malloc(_nr_streams * sizeof(cudaStream_t));
    for (int i = 0; i < _nr_streams; i++)
        cudaStreamCreate(&_streams[i]);
}

void CSRAdapter::DeleteStreams()
{
    cudaFree(_streams);
}

void CSRAdapter::FillDerivative()
{

    checkCudaErrors(cudaMalloc((fptype**)&_dydt, _group._n * sizeof(fptype)));
}

void CSRAdapter::FillRandom()
{
    inttype numBlocks = (_group.NumObjects() + _blockSize - 1) / _blockSize;
    checkCudaErrors(cudaMalloc((inttype**)&_random_poisson, _group.NumObjects() * sizeof(inttype)));
    checkCudaErrors(cudaMalloc((void**)&_randomState, _blockSize * numBlocks * sizeof(curandState)));
}

void CSRAdapter::DeleteRandom()
{
    cudaFree(_random_poisson);
    cudaFree(_randomState);
}

void CSRAdapter::DeleteDerivative()
{
    cudaFree(_dydt);
}

void CSRAdapter::ClearDerivative()
{

    inttype n = _group._n;
    CudaClearDerivative << <_numBlocks, _blockSize >> > (n, _dydt);

}

std::vector<inttype> CSRAdapter::NrRows(const std::vector<TwoDLib::CSRMatrix>& vecmat) const
{
    std::vector<inttype> vecret;
    for (inttype m = 0; m < vecmat.size(); m++)
        vecret.push_back(vecmat[m].NrRows());
    return vecret;
}

std::vector<inttype> CSRAdapter::Offsets(const std::vector<TwoDLib::CSRMatrix>& vecmat) const
{
    std::vector<inttype> vecret;
    for (inttype m : _group.getGroup().Offsets())
        vecret.push_back(m);
    return vecret;
}

void CSRAdapter::CalculateDerivative(const std::vector<fptype>& vecrates)
{
    for (inttype m : _vecmats)
    {
        // be careful to use this block size
        inttype numBlocks = (_nr_rows[m] + _blockSize - 1) / _blockSize;
        CudaCalculateDerivative << <numBlocks, _blockSize >> > (_nr_rows[m], vecrates[m], _dydt, _group._mass, _val[m], _ia[m], _ja[m], _group._map, _offsets[m]);
    }

}

void CSRAdapter::CalculateGridDerivative(const std::vector<inttype>& vecindex, const std::vector<fptype>& vecrates, const std::vector<fptype>& vecstays, const std::vector<fptype>& vecgoes, const std::vector<int>& vecoff1s, const std::vector<int>& vecoff2s)
{
    for (inttype m = 0; m < vecindex.size(); m++)
    {
        // be careful to use this block size
        inttype numBlocks = (_nr_rows[vecindex[m]] + _blockSize - 1) / _blockSize;
        CudaCalculateGridDerivative << <numBlocks, _blockSize, 0, _streams[vecindex[m]] >> > (_nr_rows[vecindex[m]], vecrates[m], vecstays[m], vecgoes[m], vecoff1s[m], vecoff2s[m], _dydt, _group._mass, _offsets[m]);
    }

    cudaDeviceSynchronize();
}

void CSRAdapter::CalculateMeshGridDerivative(const std::vector<inttype>& vecindex,
    const std::vector<fptype>& vecrates, const std::vector<fptype>& vecstays,
    const std::vector<fptype>& vecgoes, const std::vector<int>& vecoff1s,
    const std::vector<int>& vecoff2s)
{

    for (inttype m = 0; m < vecstays.size(); m++)
    {
        // be careful to use this block size
        inttype numBlocks = (_nr_rows[vecindex[m]] + _blockSize - 1) / _blockSize;
        CudaCalculateGridDerivative << <numBlocks, _blockSize, 0, _streams[vecindex[m]] >> > (_nr_rows[vecindex[m]], vecrates[m], vecstays[m], vecgoes[m], vecoff1s[m], vecoff2s[m], _dydt, _group._mass, _offsets[vecindex[m]]);
    }

    for (int n = vecstays.size(); n < vecrates.size(); n++)
    {
        inttype mat_index = _grid_transforms.size() + (n - vecstays.size());
        // be careful to use this block size
        inttype numBlocks = (_nr_rows[mat_index] + _blockSize - 1) / _blockSize;
        CudaCalculateDerivative << <numBlocks, _blockSize, 0, _streams[vecindex[n]] >> > (_nr_rows[mat_index], vecrates[n], _dydt, _group._mass, _val[mat_index], _ia[mat_index], _ja[mat_index], _group._map, _offsets[mat_index]);
    }

    cudaDeviceSynchronize();

}


void CSRAdapter::CalculateMeshGridDerivativeWithEfficacy(const std::vector<inttype>& vecindex, const std::vector<inttype>& in_vecindex,
    const std::vector<fptype>& vecrates)
{

    for (inttype m = 0; m < _nr_grid_connections; m++)
    {
        unsigned int mesh_m = vecindex[m];
        unsigned int in_mesh_m = in_vecindex[m];

        if (_group.getGroupObjects()[mesh_m] > 0) {
            continue;
        }

        // be careful to use this block size
        inttype numBlocks = (_nr_rows[vecindex[m]] + _blockSize - 1) / _blockSize;

        dim3 numBlocks2D;
        dim3 blockSize2D = dim3(_blockSize, 1);
        if (in_mesh_m < _group._kernels.size() && _group._kernels[in_mesh_m].size() > 0) {
            // kernel
            numBlocks2D = dim3(numBlocks, _group._kernels[in_mesh_m].size());
        }
        else {
            // no kernel
            numBlocks2D = dim3(numBlocks, 1);
        }

        CudaCalculateGridDerivativeCsrKernel << <numBlocks, _blockSize, 0, _streams[vecindex[m]] >> > (_nr_rows[vecindex[m]], vecrates[m], _dydt, _group._mass, _group._mass_histories, _group._host_mass_histories.size(),
            _grid_val[m], _grid_ia[m], _grid_ja[m], _offsets[mesh_m], _group._vec_vec_kernels[in_mesh_m], _group._kernels[in_mesh_m].size());


    }


    for (int n = _nr_grid_connections; n < vecrates.size(); n++)
    {
        inttype mat_index = _grid_transforms.size() + (n - _nr_grid_connections);

        unsigned int mesh_m = vecindex[n];
        unsigned int in_mesh_m = in_vecindex[n];

        if (_group.getGroupObjects()[mesh_m] > 0) {
            continue;
        }

        // be careful to use this block size
        inttype numBlocks = (_nr_rows[mat_index] + _blockSize - 1) / _blockSize;

        dim3 numBlocks2D;
        dim3 blockSize2D = dim3(_blockSize, 1);
        if (in_mesh_m < _group._kernels.size() && _group._kernels[in_mesh_m].size() > 0) {
            // kernel
            numBlocks2D = dim3(numBlocks, _group._kernels[in_mesh_m].size());
        }
        else {
            // no kernel
            numBlocks2D = dim3(numBlocks, 1);
        }

        CudaCalculateDerivativeKernel << < numBlocks2D, blockSize2D, 0, _streams[vecindex[n]] >> > (_nr_rows[mat_index], vecrates[n], _dydt, _group._mass, _group._mass_histories, _group._host_mass_histories.size(),
            _val[mat_index], _ia[mat_index], _ja[mat_index], _group._map, _offsets[mesh_m],
            _group._vec_vec_kernels[in_mesh_m], _group._kernels[in_mesh_m].size());

    }

    cudaDeviceSynchronize();

}



void CSRAdapter::SingleTransformStep()
{

    for (inttype m : _grid_transforms)
    {
        if (_group._vec_num_objects[m] > 0)
            continue;

        // be careful to use this block size
        inttype numBlocks = (_nr_rows[m] + _blockSize - 1) / _blockSize;
        CudaSingleTransformStep << <numBlocks, _blockSize, 0, _streams[m] >> > (_nr_rows[m], _dydt, _group._mass, _val[m], _ia[m], _ja[m], _group._map, _offsets[m]);
    }

}

void CSRAdapter::SingleTransformStepFiniteSize()
{

    for (inttype m : _grid_transforms)
    {
        if (_group._vec_num_objects[m] == 0)
            continue;
        // be careful to use this block size
        inttype numBlocks = (_group._vec_num_objects[m] + _blockSize - 1) / _blockSize;
        CudaGridEvolveFiniteObjects << <numBlocks, _blockSize, 0, _streams[m] >> >
            (_group._vec_num_objects[m], _group._vec_num_object_offsets[m], _group._vec_objects_to_index, _group._vec_objects_refract_times,
                _forward_val[m], _forward_ia[m], _forward_ja[m],
                _offsets[m], _randomState);
    }
}


void CSRAdapter::AddDerivative()
{
    EulerStep << <_numBlocks, _blockSize >> > (_group._n, _dydt, _group._mass, _euler_timestep);
}

void CSRAdapter::AddDerivativeFull()
{
    EulerStep << <_numBlocks, _blockSize >> > (_group._n, _dydt, _group._mass, 1.0);
}

void CSRAdapter::setRandomSeeds(double seed) {


    inttype numBlocks = (_group.NumObjects() + _blockSize - 1) / _blockSize;

    initCurand << <numBlocks, _blockSize >> > (_randomState, seed);




}

void CSRAdapter::CalculateMeshGridDerivativeWithEfficacyFinite(const std::vector<inttype>& vecindex,
    const std::vector<fptype>& vecrates, const std::vector<fptype>& efficacy, const std::vector<fptype>& cell_widths, const std::vector<inttype>& cell_offsets, double timestep)
{
    for (inttype m = 0; m < _nr_grid_connections; m++)
    {
        unsigned int mesh_m = vecindex[m];

        if (_group.getGroupObjects()[mesh_m] == 0)
            continue;

        // be careful to use this block size
        inttype numBlocks = (_group._vec_num_objects[mesh_m] + _blockSize - 1) / _blockSize;

        generatePoissonSpikes << <numBlocks, _blockSize >> > (_group._vec_num_objects[mesh_m], _group._vec_num_object_offsets[mesh_m], vecrates[m], timestep, _random_poisson, _randomState);

        CudaCalculateGridDerivativeCsrFinite << <numBlocks, _blockSize >> > (_group._vec_num_objects[mesh_m], _group._vec_num_object_offsets[mesh_m], _random_poisson, _group._vec_objects_to_index,
            _group._vec_objects_refract_times, _group._vec_objects_refract_index,
            _grid_forward_val[m], _grid_forward_ia[m], _grid_forward_ja[m], _offsets[mesh_m], _randomState);

        for (int n = _nr_grid_connections; n < vecrates.size(); n++)
        {
            inttype mat_index = _grid_transforms.size() + (n - _nr_grid_connections);
            unsigned int mesh_n = vecindex[n];

            if (_group.getGroupObjects()[mesh_n] == 0) {
                continue;
            }

            inttype numBlocks = (_group._vec_num_objects[mesh_n] + _blockSize - 1) / _blockSize;
            // be careful to use this block size
            generatePoissonSpikes << <numBlocks, _blockSize >> > (_group._vec_num_objects[mesh_n], _group._vec_num_object_offsets[mesh_n], vecrates[n], timestep, _random_poisson, _randomState);

            CudaUpdateFiniteObjects << <numBlocks, _blockSize >> > (_group._vec_num_objects[mesh_n], _group._vec_num_object_offsets[mesh_n], _random_poisson, _group._vec_objects_to_index,
                _group._vec_objects_refract_times, _group._vec_objects_refract_index, _forward_val[mat_index], _forward_ia[mat_index], _forward_ja[mat_index],
                _group._map, _group._unmap, _offsets[mesh_n], _randomState);

        }
    }
    cudaDeviceSynchronize();

}
