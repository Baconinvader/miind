# -*- coding: utf-8 -*-
#
#
#  TheVirtualBrain-Scientific Package. This package holds all simulators, and
# analysers necessary to run brain-simulations. You can use it stand alone or
# in conjunction with TheVirtualBrain-Framework Package. See content of the
# documentation-folder for more details. See also http://www.thevirtualbrain.org
#
# (c) 2012-2017, Baycrest Centre for Geriatric Care ("Baycrest") and others
#
# This program is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  See the GNU General Public License for more details.
# You should have received a copy of the GNU General Public License along with this
# program.  If not, see <http://www.gnu.org/licenses/>.
#
#
#   CITATION:
# When using The Virtual Brain for scientific publications, please cite it as follows:
#
#   Paula Sanz Leon, Stuart A. Knock, M. Marmaduke Woodman, Lia Domide,
#   Jochen Mersmann, Anthony R. McIntosh, Viktor Jirsa (2013)
#       The Virtual Brain: a simulator of primate brain network dynamics.
#   Frontiers in Neuroinformatics (7:10. doi: 10.3389/fninf.2013.00010)

"""
MIIND Adapter class to allow MIIND models to be generically imported and run in TVB.

"""

from .base import ModelNumbaDfun, LOG, numpy, basic, arrays
from numba import guvectorize, float64

import imp

class Miind(ModelNumbaDfun):
    r"""
    MIIND (Multiple Instantiations of Interacting Neural Dynamics) [MDK_2013] is
    a software package which simulates interacting populations of neurons using
    population density techniques [OM_2000]. It has been implemented here to leverage
    the TVB's connectivity and coupling functionality while performing the actual
    region (node) simulation in MIIND.

    **Technical Notes**:

    The Miind class constructor takes a filename which should point to a shared library
    file generated by MIIND which both defines and performs the simulation for each region.
    When using MIIND in TVB, a user can instantiate this Miind class to be used as the model
    in the simulation then "plug and play" different shared library files from MIIND.

    Shared library files can be generated by MIIND to support MPI. While tvb-library
    source files do not require an MPI implementation, the script to run the TVB
    simulation should call MPI_Init to ensure compatability.

    The Miind model class requires that MIIND has been installed on the system.
    Installation instructions are available at
    http://miind.sourceforge.net/

    **References**:

    .. [MDK_2013] de Kamps, M., 2013. A generic approach to solving jump diffusion
    equations with applications to neural populations. arXiv preprint arXiv:1309.1654.

    .. [OM_2000] Omurtag, A., Knight, B.W. and Sirovich, L., 2000. On the simulation
    of large populations of neurons. Journal of computational neuroscience, 8(1), pp.51-63.

    """

    _ui_name = "MIIND"
    ui_configurable_parameters = []

    # Used for phase-plane axis ranges and to bound random initial() conditions.
    state_variable_range = basic.Dict(
        label="State Variable ranges [lo, hi]",
        default={"S": numpy.array([0.0, 1.0])},
        doc="Population firing rate",
        order=17)

    variables_of_interest = basic.Enumerate(
        label="Variables watched by Monitors",
        options=["S"],
        default=["S"],
        select_multiple=True,
        doc="""default state variables to be monitored""",
        order=18)

    state_variables = ['S']
    _nvar = 1
    cvar = numpy.array([0], dtype=numpy.int32)

    def __init__(self, module_file, num_nodes, simulation_length, model_params = []):
        # dynamically import the MIIND shared library
        remove_so = module_file.split('.so')
        remove_nix_path = remove_so[0].split('/')
        remove_win_path = remove_nix_path[-1].split('\\')
        miind = imp.load_dynamic(remove_win_path[-1], module_file)
        self.number_of_nodes = num_nodes
        self.simulation_length = simulation_length
        self.miindmodel = miind
	self.model_parameters = model_params
	

    def configure(self):
        """  """
        super(Miind, self).configure()
        self.update_derived_parameters()
    	self.miindmodel.init(self.number_of_nodes, self.simulation_length)
        self.miindmodel.startSimulation()


    def dfun(self, x, c, local_coupling=0.0):

        S = x[0, :]

        # long-range coupling
        c_0 = c[0, :]

        # short-range (local) coupling
        lc_0 = local_coupling * S

        # Would be nice to pass this function as a parameter to make this
        # class more generic
        coupling_S = c_0 + lc_0

        c_ = coupling_S[:,0]
    	x_ = numpy.array(self.miindmodel.evolveSingleStep(c_.tolist()))

        return numpy.reshape(x_, x.shape)
