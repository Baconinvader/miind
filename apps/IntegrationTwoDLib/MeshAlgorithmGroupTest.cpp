// Copyright (c) 2005 - 2014 Marc de Kamps
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

#include <fstream>
#include <boost/timer/timer.hpp>
#include <TwoDLib.hpp>
#include <MPILib/include/utilities/Exception.hpp>



bool TestEquality(const TwoDLib::Mesh& mesh, const TwoDLib::Ode2DSystemGroup& group, MPILib::Number nmesh ){
	std::vector<double> vals(nmesh,0.0);

	for (MPILib::Index i = 0; i < mesh.NrStrips(); i++)
		for (MPILib::Index j = 0; j < mesh.NrCellsInStrip(i); j++){
			for (MPILib::Number m = 0; m < nmesh; m++)
			{
				vals[m] = group.Mass()[group.Map(m,i,j)];
			}
			double val = vals[0];
			for (MPILib::Number m = 0; m < nmesh; m++)
				if (vals[m] != val){
					std::cout << "Difference in: " << i << " " << j << " for mesh: " << m <<std::endl;
					exit(0);
				}
		}
	return true;
}


int main(int argc, char** argv)
{
  try {
  std::cout << "Start. " << std::endl;
  TwoDLib::Mesh mesh1("condee2a5ff4-0087-4d69-bae3-c0a223d03693.model");
  TwoDLib::Mesh mesh2("condee2a5ff4-0087-4d69-bae3-c0a223d03693.model");
  TwoDLib::Mesh mesh3("condee2a5ff4-0087-4d69-bae3-c0a223d03693.model");

  std::ifstream ifstrev("condee2a5ff4-0087-4d69-bae3-c0a223d03693.rev");
  std::vector<TwoDLib::Redistribution> vec_rev1 = TwoDLib::ReMapping(ifstrev);
  std::vector<TwoDLib::Redistribution> vec_rev2 = vec_rev1;
  std::vector<TwoDLib::Redistribution> vec_rev3 = vec_rev1;
  
  std::ifstream ifstres("condee2a5ff4-0087-4d69-bae3-c0a223d03693.res");
  std::vector<TwoDLib::Redistribution> vec_res1 = TwoDLib::ReMapping(ifstres);
  std::vector<TwoDLib::Redistribution> vec_res2 = vec_res1;
  std::vector<TwoDLib::Redistribution> vec_res3 = vec_res1;

  std::vector<TwoDLib::Mesh> mesh_vec{ mesh1, mesh2, mesh3 };
  std::vector<std::vector<TwoDLib::Redistribution> > vec_rev{ vec_rev1, vec_rev2, vec_rev3 };
  std::vector<std::vector<TwoDLib::Redistribution> > vec_res{ vec_res1, vec_res2, vec_res3 };

  std::cout << "Building group. " << std::endl;  
  TwoDLib::Ode2DSystemGroup group(mesh_vec,vec_rev,vec_res);
  group.Initialize(0,0,0);
  group.Initialize(1,0,0);
  group.Initialize(2,0,0);

  std::cout << "Building matrices." << std::endl;
  TwoDLib::TransitionMatrix mat1("condee2a5ff4-0087-4d69-bae3-c0a223d03693_0_0.05_0_0_.mat");
  TwoDLib::TransitionMatrix mat2("condee2a5ff4-0087-4d69-bae3-c0a223d03693_0_0.05_0_0_.mat");
  TwoDLib::TransitionMatrix mat3("condee2a5ff4-0087-4d69-bae3-c0a223d03693_0_0.05_0_0_.mat");

  std::cout << "Filling CSR." << std::endl;
  TwoDLib::CSRMatrix csrmat1(mat1,group,0);
  TwoDLib::CSRMatrix csrmat2(mat2,group,1);
  TwoDLib::CSRMatrix csrmat3(mat3,group,2);


  std::cout << "Creating matrix vector." << std::endl;

  std::vector<TwoDLib::CSRMatrix> vecmat{csrmat1, csrmat2, csrmat3};
 

  // create a vector for the derivative
  std::vector<double> dydt(group.Mass().size());

  // Some parameters to mess around with are here
  MPILib::Time t_sim=1.0;
  MPILib::Number n_steps = static_cast<MPILib::Number>(floor(t_sim/mesh1.TimeStep()));
  TwoDLib::MasterParameter par(10);
  double h = 1./par._N_steps*mesh1.TimeStep();

  std::cout << "Defining rates" << std::endl;
  vector<double> vecrates{800., 900., 1000.};
 
  boost::timer::auto_cpu_timer t;
  std::cout << "Starting loop." << std::endl;
  for (MPILib::Index i = 0; i < n_steps; i++){
    group.Evolve();

    for (MPILib::Index i_part = 0; i_part < par._N_steps; i_part++ ){
      TwoDLib::ClearDerivative(dydt);
      TwoDLib::CalculateDerivative(group,dydt,vecmat,vecrates);
      TwoDLib::AddDerivative(group.Mass(),dydt,h);
    }

    group.RedistributeProbability();
    group.RemapReversal();

    if (i%100 == 0) std::cout << mesh1.TimeStep()*i << " " << group.F()[0] << " " << group.F()[1] <<  " " << group.F()[2] <<  std::endl;
  }
  t.stop();
  std::cout << "Overall time spend\n";
  t.report();

  std::ofstream ofst1("dens1.dat");
  std::ofstream ofst2("dens2.dat");
  std::ofstream ofst3("dens3.dat");

  std::vector<std::ostream*> vec_stream{ &ofst1, &ofst2, &ofst3 };
  group.Dump(vec_stream);
}
catch(...){
    std::cout << "Oops" << std::endl;
}
  return 0;
}
