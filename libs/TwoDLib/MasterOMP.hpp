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
#ifndef _CODE_LIBS_TWODLIB_MASTEROMP_INCLUDE_GUARD
#define _CODE_LIBS_TWODLIB_MASTEROMP_INCLUDE_GUARD

#include <string>
#include "CSRMatrix.hpp"
#include "TransitionMatrix.hpp"
#include "Ode2DSystemGroup.hpp"
#include "MasterParameter.hpp"

namespace TwoDLib {

	//! OpenMP version of a forward Euler integration of the Master equation

	class MasterOMP {
	public:

		MasterOMP
		(
			Ode2DSystemGroup&,
			const std::vector<std::vector<TransitionMatrix> >&,  //! vector of all TransitionMatrix instances that determine the input to this population
			const MasterParameter&
		);

		//! Number of meshes
		MPILib::Number  NrMeshes() const { return _vec_vec_csr.size(); }

		//! Number of weight matrices associated with Mesh i
		MPILib::Number  NrMatrices(MPILib::Index i) const { return _vec_vec_csr[i].size(); }

		//! Efficacy associated with matrix matrix_index from Mesh mesh_index
		double Efficacy
		(
				MPILib::Index mesh_index,	//!< Index labeling mesh
				MPILib::Index matrix_index
		) const {return _vec_vec_csr[mesh_index][matrix_index].Efficacy(); }


		void Apply
			(
				double                                         time_step,      //!< mesh time step
				const std::vector<std::vector<MPILib::Rate> >& rate_matrix,	//!< rate matrix
				const vector<MPILib::Index>&	               weight_order    //!< reference to the mapping from NodeId to weight order
			);

		void ApplyFinitePoisson
		(
			double                                         time_step,      //!< mesh time step
			const std::vector<std::vector<MPILib::Rate> >& rate_matrix,	//!< rate matrix
			const vector<MPILib::Index>& weight_order    //!< reference to the mapping from NodeId to weight order
		);

	private:

		MasterOMP(const MasterOMP&);
		MasterOMP& operator=(const MasterOMP&);

		std::vector< std::vector<CSRMatrix> >
			InitializeCSR
			(
				const std::vector<std::vector<TransitionMatrix> >&  vec_vec_mat,
				const Ode2DSystemGroup&                             sys
			);



		//! Auxilliary struct to calculate the derivative for forward Euler
		struct Derivative {

			Derivative(vector<double>& dydt, const Ode2DSystemGroup& sys, double& rate ):_dydt(dydt),_sys(sys),_rate(rate){}

			void operator()(const TransitionMatrix::TransferLine& line, MPILib::Index mesh_index) {
				for(const TransitionMatrix::Redistribution& r: line._vec_to_line){
					double from = _rate*r._fraction*_sys._vec_mass[_sys.Map(mesh_index,line._from[0],line._from[1])];
					_dydt[_sys.Map(mesh_index,r._to[0],r._to[1])] += from;
					}
				_dydt[_sys.Map(mesh_index,line._from[0],line._from[1])] -= _rate*_sys._vec_mass[_sys.Map(mesh_index,line._from[0],line._from[1])];
			}
			vector<double>&          _dydt;
			const Ode2DSystemGroup&  _sys;
			const double&            _rate;
		};

		//! Auxilliary struct for forward Euler
		struct Add {
			Add(double h):_h(h){}

			double operator()(double mass, double dydt){
				mass += _h*dydt;
				return mass;
			}

			double _h;
		};

		//! Auxilliary struct for initialization
		struct Init {
			Init(double rate):_rate(rate){}

			const double _rate;

			double operator()(double mass){
				return 0.;
			}
		};


		Ode2DSystemGroup& _sys;

		const std::vector<std::vector<TransitionMatrix> >& _vec_vec_mat; // After initialization, the original object is allowed to go out of scope; it will not be referred anymore
		const std::vector<std::vector<CSRMatrix> >         _vec_vec_csr;

		const MasterParameter	_par;
		vector<double>			_dydt;
		double					_rate;

		Derivative				_derivative;
	    Add 					_add;
		double					_h;
		Init                    _init;
	};
}

#endif // include guard
