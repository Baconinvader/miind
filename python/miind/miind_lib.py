#!/usr/bin/env python
import os
import re
import shutil
import argparse
import numpy as np
import xml.etree.ElementTree as ET
import miind.directories3 as directories
from collections import Counter
import miind.reporting as reporting
import miind.connections as connections
import miind.variables as variables
import miind.algorithms as algorithms

# These algorithms can feature in a MeshAlgorithmGroup simulation, and no others
MESH_ALGORITHM_GROUP_LIST = ['GridAlgorithmGroup', 'MeshAlgorithmGroup', 'DelayAlgorithm', 'RateFunctor' ]

def parse_rate_functors(algorithms):
     s=''
     for algorithm in algorithms:
          if algorithm.attrib['type'] == 'RateFunctor':
             expression = algorithm.find('expression')
             name=re.sub(r'\s+', '_', algorithm.attrib['name']) # a user may use white space, we need to replace it
             s += 'static MPILib::Rate ' + name + '( MPILib::Time t ){\n'
             s += '\treturn ' + expression.text + ';\n'
             s += '}\n\n'
     return s


def generate_preamble(fn, variables, nodes, algorithms, connections, parameters, cuda):
    '''Generates the function declarations, required for RateFunctors etc in the C++ file. fn is the file name where the C++
    is to be written. variable, nodes and algorithms are XML elements.'''

    if cuda == True:
         template_argument = 'fptype'
    else:
         template_argument = 'MPILib::Rate'

    with open(fn,'w') as f:
        f.write('//Machine-generated by miind.py. Edit at your own risk.\n\n')
        f.write('#define PY_SSIZE_T_CLEAN\n')
        f.write('#include <Python.h>\n')
        f.write('#include <boost/timer/timer.hpp>\n')
        f.write('#include <GeomLib.hpp>\n')
        f.write('#include <TwoDLib.hpp>\n')
        if cuda == True: f.write('#include <CudaTwoDLib.hpp>\n')
        f.write('#include <MPILib/include/RateAlgorithmCode.hpp>\n')
        f.write('#include <MPILib/include/SimulationRunParameter.hpp>\n')
        f.write('#include <MPILib/include/DelayAlgorithmCode.hpp>\n')
        f.write('#include <MPILib/include/RateFunctorCode.hpp>\n')
        f.write('#include <MPILib/include/MiindTvbModelAbstract.hpp>\n')
        f.write('#include <MiindLib/VectorizedNetwork.hpp>\n\n')
        if cuda == True: f.write('typedef CudaTwoDLib::fptype fptype;\n')
        f.write('\n')

def constructor_override(outfile,tree,typ):

    t_begin = tree.find('SimulationRunParameter/t_begin')
    t_end = tree.find('SimulationRunParameter/t_end')

    variable_list = tree.findall('Variable')
    variables.parse_variables(variable_list,outfile)
    t_step = tree.find('SimulationRunParameter/t_step')

    outfile.write('\tMiindModel(int num_nodes):\n')
    outfile.write('\t\tMiindTvbModelAbstract(num_nodes,  ' + t_end.text + '), vec_network('+ t_step.text +'),_count(0){\n')
    outfile.write('#ifdef ENABLE_MPI\n')
    outfile.write('\t// initialise the mpi environment this cannot be forwarded to a class\n')
    outfile.write('\tboost::mpi::environment env();\n')
    outfile.write('#endif\n')
    outfile.write('}\n\n')

    outfile.write('\tMiindModel():\n')
    outfile.write('\t\tMiindTvbModelAbstract(1,  ' + t_end.text + '), vec_network('+ t_step.text +'),_count(0){\n')
    outfile.write('#ifdef ENABLE_MPI\n')
    outfile.write('\t// initialise the mpi environment this cannot be forwarded to a class\n')
    outfile.write('\tboost::mpi::environment env();\n')
    outfile.write('#endif\n')
    outfile.write('}\n\n')

    # for some reason we need an explicit copy constructor
    outfile.write('\tMiindModel(const MiindModel& rhs):\n')
    outfile.write('\tMiindTvbModelAbstract(1, rhs._simulation_length), vec_network('+ t_step.text +'),_count(rhs._count) {}\n\n')

    if len(variable_list) > 0:
        outfile.write('\tMiindModel(int num_nodes, \n')
        variables.parse_variables_as_parameters(variable_list,outfile)
        outfile.write('):\n')
        outfile.write('\t\tMiindTvbModelAbstract(num_nodes,  ' + t_end.text +'), vec_network('+ t_step.text +'),_count(0)\n')
        variables.parse_variables_as_constructor_defaults(variable_list, outfile)
        outfile.write('{\n')
        outfile.write('#ifdef ENABLE_MPI\n')
        outfile.write('\t// initialise the mpi environment this cannot be forwarded to a class\n')
        outfile.write('\tboost::mpi::environment env();\n')
        outfile.write('#endif\n')
        outfile.write('}\n\n')

    if len(variable_list) > 0:
        outfile.write('\tMiindModel( \n')
        variables.parse_variables_as_parameters(variable_list,outfile)
        outfile.write('):\n')
        outfile.write('\t\tMiindTvbModelAbstract(1,  ' + t_end.text +'), vec_network('+ t_step.text +'),_count(0)\n')
        variables.parse_variables_as_constructor_defaults(variable_list, outfile)
        outfile.write('{\n')
        outfile.write('#ifdef ENABLE_MPI\n')
        outfile.write('\t// initialise the mpi environment this cannot be forwarded to a class\n')
        outfile.write('\tboost::mpi::environment env();\n')
        outfile.write('#endif\n')
        outfile.write('}\n\n')

def abstract_type(type):
    if type ==  "DelayedConnection":
        s = "MPILib::" + type
    else:
        s = "double"
    return 'MPILib::MiindTvbModelAbstract<' + s + ', MPILib::utilities::CircularDistribution>'

def define_abstract_type(type):
    if type ==  "DelayedConnection":
        s = "MPILib::" + type
    else:
        s = "double"
    return 'define_python_MiindTvbModelAbstract<' + s + ', MPILib::utilities::CircularDistribution>();\n'


def function_overrides(outfile,tree,typ):
    outfile.write('\n\tvoid endSimulation(){\n')
    outfile.write('\t\t'+ abstract_type(typ) +'::endSimulation();\n')
    outfile.write('\t}\n')

    outfile.write('\tvoid startSimulation(){\n')
    outfile.write('\t\tvec_network.setupLoop(true);\n')
    outfile.write('\t\tpb = new utilities::ProgressBar((int)(_simulation_length/_time_step));\n')
    outfile.write('\t}\n')

    outfile.write('\t\tstd::vector<double> evolveSingleStep(std::vector<double> activity){\n')

    outfile.write('\t\tstd::vector<double> out_activities;\n')
    outfile.write('\t\tfor(auto& it : vec_network.singleStep(activity,_count)) {\n')
    outfile.write('\t\t\tout_activities.push_back(it);\n')
    outfile.write('\t\t}\n\n')

    outfile.write('\t\t_count++;\n')
    outfile.write('\t\t(*pb)++;\n\n')

    outfile.write('\t\treturn out_activities;\n')
    outfile.write('\t}\n\n')

def generate_opening(fn, tree, typ, algorithms, variables):
    with open(fn,'a') as outfile:
        outfile.write('class MiindModel : public ' + abstract_type(typ) + ' {\n')
        outfile.write('public:\n\n')
        outfile.write('\t\tMiindLib::VectorizedNetwork vec_network;\n')
        outfile.write('\t\tpugi::xml_document doc;\n')
        outfile.write('\t\tstd::vector<MPILib::NodeId> _display_nodes;\n')
        outfile.write('\t\tstd::vector<MPILib::NodeId> _rate_nodes;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> _rate_node_intervals;\n')
        outfile.write('\t\tstd::vector<MPILib::NodeId> _density_nodes;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> _density_node_start_times;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> _density_node_end_times;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> _density_node_intervals;\n')
        outfile.write('\tunsigned int _count;\n')
        outfile.write('\n')

        # the rate functor functions need to be declared before the main program
        function_declarations = parse_rate_functors(algorithms)
        outfile.write(function_declarations)
        constructor_override(outfile, tree,typ)
        function_overrides(outfile,tree,typ)
        outfile.write('\n')
        outfile.write('\tvoid init()\n')
        outfile.write('\t{\n')
        t_step = tree.find('SimulationRunParameter/t_step')
        outfile.write('\t\t_time_step = ' + t_step.text + ';\n')
        outfile.write('\t\tstd::vector<MPILib::NodeId> density_nodes;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> density_node_start_times;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> density_node_end_times;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> density_node_intervals;\n')
        outfile.write('\t\tstd::vector<MPILib::NodeId> rate_nodes;\n')
        outfile.write('\t\tstd::vector<MPILib::Time> rate_node_intervals;\n')
        outfile.write('\t\tstd::vector<MPILib::NodeId> display_nodes;\n')
        outfile.write('\t\tfor(int i=0; i<_num_nodes; i++) {\n')

def generate_closing(fn,parameters,tree,type,prog_name,members):
    end_time = parameters.find('t_end').text
    time_step = parameters.find('t_step').text

    steps = ''
    master_steps = parameters.findall('master_steps')
    if(len(master_steps) > 0):
        steps = master_steps[0].text

    node_list = tree.findall('Nodes/Node')
    nodemap = node_name_to_node_id(node_list)

    '''Generates the closing statements in the C++ file.'''
    with open(fn,'a') as f:
        f.write(reporting.define_display_nodes(tree,nodemap,unscoped_vectors=True,looped_definition=True))
        f.write(reporting.define_rate_nodes(tree,nodemap, unscoped_vectors=True,looped_definition=True))
        f.write(reporting.define_density_nodes(tree,nodemap, unscoped_vectors=True,looped_definition=True))
        f.write('\n')
        f.write('\t\t\n')
        f.write('\t\t}\n')
        f.write('\t\t_rate_nodes = rate_nodes;\n')
        f.write('\t\t_rate_node_intervals = rate_node_intervals;\n')
        f.write('\t\t_display_nodes = display_nodes;\n')
        f.write('\t\t_density_nodes = density_nodes;\n')
        f.write('\t\t_density_node_start_times = density_node_start_times;\n')
        f.write('\t\t_density_node_end_times = density_node_end_times;\n')
        f.write('\t\t_density_node_intervals = density_node_intervals;\n')
        f.write('\t\tvec_network.setDisplayNodes(_display_nodes);\n')
        f.write('\t\tvec_network.setRateNodes(_rate_nodes, _rate_node_intervals);\n')
        f.write('\t\tvec_network.setDensityNodes(_density_nodes,_density_node_start_times,_density_node_end_times,_density_node_intervals);\n')
        f.write('\n')
        f.write('\t\tvec_network.initOde2DSystem('+ steps +');\n')
        f.write('\t}\n')
        f.write('\n')
        for t in algorithms.RATEFUNCTIONS:
            f.write(t)
        f.write('\n\n')
        f.write(members)
        f.write('};\n\n')

def python_wrapper(outfile, prog_name, variable_list, num_outputs):
    addition = """
static MiindModel *model;

static PyObject *miind_init(PyObject *self, PyObject *args)
{{
    if (model) {{
        delete model;
        model = NULL;
    }}

{variable_constructors}

    Py_INCREF(Py_None);
    return Py_None;
}}

static PyObject *miind_getTimeStep(PyObject *self, PyObject *args)
{{
    return Py_BuildValue("d", model->getTimeStep());
}}

static PyObject *miind_getSimulationLength(PyObject *self, PyObject *args)
{{
    return Py_BuildValue("d", model->getSimulationLength());
}}

static PyObject *miind_startSimulation(PyObject *self, PyObject *args)
{{
    model->startSimulation();
    Py_INCREF(Py_None);
    return Py_None;
}}

static PyObject *miind_evolveSingleStep(PyObject *self, PyObject *args)
{{
    PyObject *float_list;
    int pr_length;

    if (!PyArg_ParseTuple(args, "O", &float_list))
        return NULL;
    pr_length = PyObject_Length(float_list);
    if (pr_length < 0)
        return NULL;

    std::vector<double> activities(pr_length);

    for (int index = 0; index < pr_length; index++) {{
        PyObject *item;
        item = PyList_GetItem(float_list, index);
        if (!PyFloat_Check(item))
            activities[index] = 0.0;
        activities[index] = PyFloat_AsDouble(item);
    }}

    std::vector<double> out_activities = model->evolveSingleStep(activities);

    PyObject* tuple = PyTuple_New(model->getNumNodes() * {num_external_outputs});

    for (int index = 0; index < model->getNumNodes() * {num_external_outputs}; index++) {{
        PyTuple_SetItem(tuple, index, Py_BuildValue("d", out_activities[index]));
    }}

    return tuple;
}}

static PyObject *miind_endSimulation(PyObject *self, PyObject *args)
{{
    model->endSimulation();
    Py_INCREF(Py_None);
    return Py_None;
}}

static PyMethodDef MiindModelMethods[] = {{
    {{"init",  miind_init, METH_VARARGS, "Init Miind Model."}},
    {{"getTimeStep",  miind_getTimeStep, METH_VARARGS, "Get time step."}},
    {{"getSimulationLength",  miind_getSimulationLength, METH_VARARGS, "Get sim time."}},
    {{"startSimulation",  miind_startSimulation, METH_VARARGS, "Start simulation."}},
    {{"evolveSingleStep",  miind_evolveSingleStep, METH_VARARGS, "Evolve one time step."}},
    {{"endSimulation",  miind_endSimulation, METH_VARARGS, "Clean up."}},
    {{NULL, NULL, 0, NULL}}        /* Sentinel */
}};

static struct PyModuleDef miindmodule = {{
    PyModuleDef_HEAD_INIT,
    "lib{name}",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    MiindModelMethods
}};

PyMODINIT_FUNC
PyInit_lib{name}(void)
{{
    return PyModule_Create(&miindmodule);
}}
"""
    context = { "name" : prog_name, "variable_constructors" : variables.parse_variable_python_def(variable_list), "num_external_outputs" : num_outputs }

    with open(outfile,'a') as f:
        f.write(addition.format(**context))

def process_tree(root):

    variables=root.findall(".//Variable")
    nodes=root.findall('.//Node')
    algorithms=root.findall('.//Algorithm')
    connections=root.findall('.//Connection')
    external_incoming_connections = root.findall('.//IncomingConnection')
    external_outgoing_connections = root.findall('.//OutgoingConnection')
    parameters=root.findall('.//SimulationRunParameter')
    io=root.findall('.//SimulationIO')
    return variables, nodes, algorithms, connections, external_incoming_connections, external_outgoing_connections, parameters[0], io

def parse(fn):
    '''Takes a filename. Puts the file with filename fn through the XML parser. Returns nothing.'''
    try:
        tree = ET.parse(fn)
        root = tree.getroot()

    except FileNotFoundError:
        print('No file ' + fn)
    return root

def generate_model_files(fn, nodes,algorithms):
    members = ''
    with open(fn,'a') as f:
        node_id = 0
        for node in nodes:
            algname = node.attrib['algorithm']
            algorithm = None
            for alg in algorithms:
                if alg.attrib['name'] == algname: # here we assume the name is unique
                    algorithm = alg

            if not algorithm:
                raise Exception('No algorithm named \'' + algname + '\' for node \'' + node.attrib['name'] + '\'')

            if algorithm.attrib['type'] == 'MeshAlgorithmGroup':
              ref = '0.0'
              if 'tau_refractive' in algorithm.attrib.keys():
                   ref = algorithm.attrib['tau_refractive']

              f.write('\t\t\tpugi::xml_parse_result result' + str(node_id) + ' = doc.load_file(\"' + algorithm.attrib['modelfile'] +'\");\n')
              f.write('\t\t\tpugi::xml_node  root' + str(node_id) + ' = doc.first_child();\n\n')
              members += '\t\t\tstd::vector<TwoDLib::Mesh> mesh' + str(node_id) +';\n'
              f.write('\t\t\tmesh' + str(node_id) +'.push_back(TwoDLib::RetrieveMeshFromXML(root' + str(node_id) + '));\n')
              members += '\t\t\tstd::vector<std::vector<TwoDLib::Redistribution>> vec_rev' + str(node_id) + ';\n'
              f.write('\t\t\tvec_rev' + str(node_id) + '.push_back(TwoDLib::RetrieveMappingFromXML("Reversal",root' + str(node_id) + '));\n')
              members += '\t\t\tstd::vector<std::vector<TwoDLib::Redistribution>> vec_res' + str(node_id) + ';\n'
              f.write('\t\t\tvec_res' + str(node_id) + '.push_back(TwoDLib::RetrieveMappingFromXML("Reset",root' + str(node_id) + '));\n\n')
              f.write('\t\t\tvec_network.addMeshNode(mesh'+ str(node_id) +'[i], vec_rev'+ str(node_id) +'[i], vec_res'+ str(node_id) +'[i], ' + ref + ');\n')
              f.write('\n')

              node_id = node_id + 1

            elif algorithm.attrib['type'] == 'GridAlgorithmGroup':
              ref = 0.0
              if 'tau_refractive' in algorithm.attrib.keys():
                 ref = algorithm.attrib['tau_refractive']

              f.write('\t\t\tpugi::xml_parse_result result' + str(node_id) + ' = doc.load_file(\"' + algorithm.attrib['modelfile'] +'\");\n')
              f.write('\t\t\tpugi::xml_node  root' + str(node_id) + ' = doc.first_child();\n\n')
              members += '\t\t\tstd::vector<TwoDLib::Mesh> mesh' + str(node_id) +';\n'
              f.write('\t\t\tmesh' + str(node_id) +'.push_back(TwoDLib::RetrieveMeshFromXML(root' + str(node_id) + '));\n')
              members += '\t\t\tstd::vector<std::vector<TwoDLib::Redistribution>> vec_rev' + str(node_id) + ';\n'
              f.write('\t\t\tvec_rev' + str(node_id) + '.push_back(TwoDLib::RetrieveMappingFromXML("Reversal",root' + str(node_id) + '));\n')
              members += '\t\t\tstd::vector<std::vector<TwoDLib::Redistribution>> vec_res' + str(node_id) + ';\n'
              f.write('\t\t\tvec_res' + str(node_id) + '.push_back(TwoDLib::RetrieveMappingFromXML("Reset",root' + str(node_id) + '));\n\n')
              members += '\t\t\tstd::vector<TwoDLib::TransitionMatrix> transform' + str(node_id) + ';\n'
              f.write('\t\t\ttransform' + str(node_id) + '.push_back(TwoDLib::TransitionMatrix(\"' + algorithm.attrib['transformfile'] + '\"));\n')
              f.write('\t\t\tvec_network.addGridNode(mesh'+ str(node_id) +'[i], transform'+ str(node_id) +'[i], ' + str(algorithm.attrib['start_v']) + ', ' + str(algorithm.attrib['start_w']) +', vec_rev'+ str(node_id) +'[i], vec_res'+ str(node_id) +'[i], '+ ref +');\n')
              f.write('\n')

              node_id = node_id + 1

            elif algorithm.attrib['type'] == 'RateFunctor':
                rn = 'MiindModel::' + algorithm.attrib['name']
                f.write('\t\t\tvec_network.addRateNode(' + rn + ');\n')
                node_id = node_id + 1

    return members

def extract_efficacy(fn):
     '''Extract efficacy from a matrix file. Takes a filename, returns efficacy as a single float. In the
     file efficacies are represented by two numbers. We will assume for now that one of them in zero. We will
     return the non-zero number as efficacy.'''

     with open(fn) as f:
          line=f.readline()
          nrs = [ float(x) for x in line.split()]
          if nrs[0] == 0.:
               return nrs[1]
          else:
               if nrs[1] != 0:
                    raise ValueError('Expected at least one non-zero value')
               return nrs[0]

def construct_CSR_map_external(nodes,algorithms,connections,connection_type):
    '''Creates a list that corresponds one-to-one with the connection structure. Returns a tuple: [0] node name of receiving node,[1] matrix file name for this connection  '''
    csrlist=[]
    combi = []
    for connection in connections:
         for node in nodes:
              if connection.attrib['Node'] == node.attrib['name']:
                   # we have the right node, now see if it's a MeshAlgorithmGroup
                   nodealgorithm=node.attrib['algorithm']
                   for algorithm in algorithms:
                        if nodealgorithm == algorithm.attrib['name']:
                             if algorithm.attrib['type'] == 'MeshAlgorithmGroup':

                                  mfs=algorithm.findall('MatrixFile')
                                  mfn= [ mf.text for mf in mfs]
                                  if connection_type == "DelayedConnection":
                                       efficacy=connection.text.split()[1]
                                  elif connection_type == "CustomConnectionParameters":
                                       efficacy=connection.attrib['efficacy']
                                  effs= [extract_efficacy(fn) for fn in mfn]

                                  count = Counter(combi)
                                  combi.append((connection.attrib['Node']))
                                  nr_connection = count[(connection.attrib['Node'])]

                                  if efficacy.isnumeric():
                                      efficacy = float(efficacy)
                                      candidates=[]
                                      for i, eff in enumerate(effs):
                                          if np.isclose(eff,efficacy):
                                              candidates.append(i)
                                      if len(candidates) == 0: raise ValueError('No efficacy found that corresponds to the connection efficacy ' + str(efficacy))
                                      if len(candidates) > 1: raise ValueError('Same efficacy found twice')
                                      csrlist.append([node.attrib['name'],mfn[candidates[0]], effs[candidates[0]],nr_connection,connection])
                                  else:
                                      csrlist.append([node.attrib['name'],None, efficacy,nr_connection,connection])
                                  
    return csrlist

def construct_CSR_map(nodes,algorithms,connections, connection_type):
     '''Creates a list that corresponds one-to-one with the connection structure. Returns a tuple: [0] node name of receiving node,[1] matrix file name for this connection  '''
     csrlist=[]
     combi = []
     for connection in connections:
          for node in nodes:
               if connection.attrib['Out'] == node.attrib['name']:
                    # we have the right node, now see if it's a MeshAlgorithmGroup
                    nodealgorithm=node.attrib['algorithm']
                    for algorithm in algorithms:
                         if nodealgorithm == algorithm.attrib['name']:
                              if algorithm.attrib['type'] == 'MeshAlgorithmGroup':

                                   mfs=algorithm.findall('MatrixFile')
                                   mfn= [ mf.text for mf in mfs]
                                   efficacy = None
                                   if connection_type == "DelayedConnection":
                                       efficacy=connection.text.split()[1]
                                   elif connection_type == "CustomConnectionParameters":
                                       efficacy=connection.attrib['efficacy']
                                   effs= [extract_efficacy(fn) for fn in mfn]

                                   count = Counter(combi)
                                   combi.append((connection.attrib['Out'],connection.attrib['In']))
                                   nr_connection = count[(connection.attrib['Out'],connection.attrib['In'])]

                                   if efficacy.isnumeric():
                                       efficacy = float(efficacy)
                                       candidates=[]
                                       for i, eff in enumerate(effs):
                                           if np.isclose(eff,efficacy):
                                               candidates.append(i)
                                       if len(candidates) == 0: raise ValueError('No efficacy found that corresponds to the connection efficacy ' + str(efficacy))
                                       if len(candidates) > 1: raise ValueError('Same efficacy found twice')

                                       csrlist.append([node.attrib['name'],mfn[candidates[0]], effs[candidates[0]],connection.attrib['In'],nr_connection,connection])
                                   else:
                                       csrlist.append([node.attrib['name'],None, efficacy,connection.attrib['In'],nr_connection,connection])
                                

     return csrlist

def GenerateAllMatrixFiles(nodes,algorithms,connections, connection_type):
    csrlist=[]
    combi = []
    for connection in connections:
          for node in nodes:
               if connection.attrib['Out'] == node.attrib['name']:
                    # we have the right node, now see if it's a MeshAlgorithmGroup
                    nodealgorithm=node.attrib['algorithm']
                    for algorithm in algorithms:
                         if nodealgorithm == algorithm.attrib['name']:
                              if algorithm.attrib['type'] == 'MeshAlgorithmGroup':

                                   mfs=algorithm.findall('MatrixFile')
                                   mfn= [ mf.text for mf in mfs]
                                   effs= [extract_efficacy(fn) for fn in mfn]
                                   count = Counter(combi)
                                   combi.append((connection.attrib['Out'],connection.attrib['In']))
                                   nr_connection = count[(connection.attrib['Out'],connection.attrib['In'])]
                                   csrlist.append([node.attrib['name'],mfn, effs,connection.attrib['In'],nr_connection,connection])
    return csrlist

def GenerateAllMatrixFilesExternal(nodes,algorithms,connections, connection_type):
    csrlist=[]
    combi = []
    for connection in connections:
          for node in nodes:
               if connection.attrib['Node'] == node.attrib['name']:
                    # we have the right node, now see if it's a MeshAlgorithmGroup
                    nodealgorithm=node.attrib['algorithm']
                    for algorithm in algorithms:
                         if nodealgorithm == algorithm.attrib['name']:
                              if algorithm.attrib['type'] == 'MeshAlgorithmGroup':

                                   mfs=algorithm.findall('MatrixFile')
                                   mfn= [ mf.text for mf in mfs]
                                   effs= [extract_efficacy(fn) for fn in mfn]
                                   count = Counter(combi)
                                   combi.append((connection.attrib['Node']))
                                   nr_connection = count[(connection.attrib['Node'])]
                                   csrlist.append([node.attrib['name'],mfn, effs,nr_connection,connection])
    return csrlist

def construct_grid_connection_map_external(nodes,algorithms,connections):
    '''Creates a list that corresponds one-to-one with the connection structure. Returns a tuple: [0] node name of receiving node,[1] matrix file name for this connection  '''
    gridlist=[]

    for connection in connections:
         for node in nodes:
              if connection.attrib['Node'] == node.attrib['name']:
                   # we have the right node, now see if it's a MeshAlgorithmGroup
                   nodealgorithm=node.attrib['algorithm']
                   for algorithm in algorithms:
                        if nodealgorithm == algorithm.attrib['name']:
                             if algorithm.attrib['type'] == 'GridAlgorithmGroup':
                                  gridlist.append([node.attrib['name'],connection])

    return gridlist

def construct_grid_connection_map(nodes,algorithms,connections):
     '''Creates a list that corresponds one-to-one with the connection structure. Returns a tuple: [0] node name of receiving node,[1] matrix file name for this connection  '''
     gridlist=[]

     for connection in connections:
          for node in nodes:
               if connection.attrib['Out'] == node.attrib['name']:
                    # we have the right node, now see if it's a MeshAlgorithmGroup
                    nodealgorithm=node.attrib['algorithm']
                    for algorithm in algorithms:
                         if nodealgorithm == algorithm.attrib['name']:
                              if algorithm.attrib['type'] == 'GridAlgorithmGroup':
                                   gridlist.append([node.attrib['name'],connection])

     return gridlist

def node_name_to_node_id(nodes):
     '''Create a map from name to NodeId from node elements. Return this map.'''
     d ={}
     for i,node in enumerate(nodes):
          d[node.attrib['name']] = i
     return d

def construct_monitor_external(nodes,algorithms, connections):
    '''Creates a list that corresponds one-to-one with the connection structure. Returns a tuple: [0] node name of receiving node,[1] matrix file name for this connection  '''
    gridlist=[]

    for connection in connections:
         for node in nodes:
              if connection.attrib['Node'] == node.attrib['name']:
                   # we have the right node, now see if it's a MeshAlgorithmGroup
                   nodealgorithm=node.attrib['algorithm']
                   for algorithm in algorithms:
                        if nodealgorithm == algorithm.attrib['name']:
                                  gridlist.append([node.attrib['name'],connection])

    return gridlist

def generate_connections(fn,conns, external_incoming_connections, external_outgoing_connections, nodes, algorithms, weighttype):
    members = ''
    grid_cons = construct_grid_connection_map(nodes, algorithms, conns)
    grid_cons_ext = construct_grid_connection_map_external(nodes, algorithms, external_incoming_connections)
    mesh_cons = construct_CSR_map(nodes, algorithms, conns, weighttype.text)
    mesh_cons_ext = construct_CSR_map_external(nodes, algorithms, external_incoming_connections, weighttype.text)
    mesh_ext = construct_monitor_external(nodes, algorithms, external_outgoing_connections)
    nodemap = node_name_to_node_id(nodes)

    total_external_conns = len(mesh_cons_ext) + len(grid_cons_ext)

    all_mats = GenerateAllMatrixFiles(nodes, algorithms, conns, weighttype.text)
    all_mats_ext = GenerateAllMatrixFilesExternal(nodes, algorithms, external_incoming_connections, weighttype.text)

    external_node_id = 0
    with open(fn,'a') as f:
        for cn in all_mats:
            cpp_name = 'mat_' + str(nodemap[cn[0]]) + '_' + str(nodemap[cn[3]]) + '_' + str(cn[4])
            members += 'std::map<float,std::vector<TwoDLib::TransitionMatrix>> '+ cpp_name + ';\n'
            for mfn in range(len(cn[1])):
                f.write('\t' + cpp_name + '[' + str(cn[2][mfn]) + '] = std::vector<TwoDLib::TransitionMatrix>(_num_nodes);\n')
                f.write('\t' + cpp_name + '[' + str(cn[2][mfn]) + '][i] = TwoDLib::TransitionMatrix(\"' + cn[1][mfn] + '\");\n')
                
        for cn in all_mats_ext:
            cpp_name = 'mat_' + str(nodemap[cn[0]]) + '_ext_' + str(cn[3])
            members += 'std::map<float,std::vector<TwoDLib::TransitionMatrix>> '+ cpp_name + ';\n'
            for mfn in range(len(cn[1])):
                f.write('\t' + cpp_name + '[' + str(cn[2][mfn]) + '] = std::vector<TwoDLib::TransitionMatrix>(_num_nodes);\n')
                f.write('\t' + cpp_name + '[' + str(cn[2][mfn]) + '][i] = TwoDLib::TransitionMatrix(\"' + cn[1][mfn] + '\");\n')

        if weighttype.text ==  "DelayedConnection":
            for cn in mesh_cons:
                cpp_name = 'mat_' + str(nodemap[cn[0]]) + '_' + str(nodemap[cn[3]]) + '_' + str(cn[4])
                mat_name = cpp_name + '.at(' + str(cn[2]) + ')'
                f.write(connections.parse_mesh_connection(cn[5], nodemap, mat_name +'[i]', 'vec_network',looped_definition=True))

            for cn in mesh_cons_ext:
                cpp_name = 'mat_' + str(nodemap[cn[0]]) + '_ext_' + str(cn[3])
                mat_name = cpp_name + '.at(' + str(cn[2]) + ')'
                f.write(connections.parse_external_incoming_mesh_connection(cn[4], nodemap, mat_name +'[i]', str(total_external_conns)+'*i+'+str(external_node_id), 'vec_network',looped_definition=True))
                external_node_id = external_node_id + 1

        if weighttype.text ==  "CustomConnectionParameters":
            for cn in mesh_cons:
                cpp_name = 'mat_' + str(nodemap[cn[0]]) + '_' + str(nodemap[cn[3]]) + '_' + str(cn[4])
                mat_name = cpp_name + '.at(' + str(cn[2]) + ')'
                f.write(connections.parse_mesh_vectorized_connection(cn[5], nodemap, mat_name +'[i]', 'vec_network',looped_definition=True))

            for cn in mesh_cons_ext:
                cpp_name = 'mat_' + str(nodemap[cn[0]]) + '_ext_' + str(cn[3])
                mat_name = cpp_name + '.at(' + str(cn[2]) + ')'
                f.write(connections.parse_external_incoming_mesh_vectorized_connection(cn[4], nodemap, mat_name +'[i]', str(total_external_conns)+'*i+'+str(external_node_id), 'vec_network',looped_definition=True))
                external_node_id = external_node_id + 1

        for cn in grid_cons:
            f.write(connections.parse_grid_vectorized_connection(cn[1],nodemap, 'vec_network',looped_definition=True))

        for cn in grid_cons_ext:
            f.write(connections.parse_external_incoming_grid_vectorized_connection(cn[1],nodemap, str(total_external_conns)+'*i+'+str(external_node_id), 'vec_network',looped_definition=True))
            external_node_id = external_node_id + 1

        for cn in mesh_ext:
            f.write(connections.parse_external_outgoing_connection(cn[1],nodemap, 'vec_network',looped_definition=True))

        f.write('\n')

    return members


def create_cpp_file(xmlfile, dirpath, progname, modname, cuda):
    '''Write the C++ file specified by xmlfile into dirpath as progname.'''
    root=parse(xmlfile)
    variables, nodes, algorithms, connections, external_incoming_connections, external_outgoing_connections, parameter, io=process_tree(root)
    if sanity_check(algorithms) == False: raise NameError('An algorithm incompatible with MeshAlgorithmGroup was used')
    if cuda == True:
         fn=os.path.join(dirpath, progname)+'.cu'
    else:
         fn=os.path.join(dirpath, progname)+'.cpp'
    
    generate_preamble(fn, variables, nodes, algorithms,connections,parameter, cuda)
    weighttype = root.find('WeightType')
    generate_opening(fn, root, weighttype.text, algorithms, variables)
    members = generate_model_files(fn,nodes,algorithms)
    members += generate_connections(fn,connections,external_incoming_connections, external_outgoing_connections,nodes,algorithms,weighttype)
    generate_closing(fn,parameter,root,weighttype.text,progname,members)
    variable_list = root.findall('Variable')
    python_wrapper(fn, progname, variable_list,len(external_outgoing_connections))

def sanity_check(algorithms):
    '''Check if only the allowd algorithms feature in this simulation. Returns True if so, False otherwise.'''

    for algorithm in algorithms:
        if algorithm.attrib['type'] not in MESH_ALGORITHM_GROUP_LIST:
            return False
        else:
            return True

def mesh_algorithm_group(root):
    '''True if there are MeshAlgorithmGroup algorithms in the XML file, false otherwise.'''
    algorithms=root.findall('.//Algorithm')

    for algorithm in algorithms:
        if algorithm.attrib['type'] in ["MeshAlgorithmGroup","GridAlgorithmGroup"]:
            return True

    return False

def produce_mesh_algorithm_version(dirname, filename, modname, root, enable_mpi, enable_openmp, enable_root, cuda):
    '''Entry point for the vector version of a MIIND C++ file. Filename is file name of the XML file, dirname is the user-specified directory hierarchy
    where the C++ file will be generated and the simulation will be stored. The simulation file will be placed in directory <dirname>/<xml_file_name>.'''

    if not directories.PATH_VARS_DEFINED:
        directories.initialize_global_variables()

    for xmlfile in filename:
        progname = directories.check_and_strip_name(xmlfile)
        dirpath = directories.create_dir(os.path.join(dirname, progname))
        SOURCE_FILE = progname + '.cpp'
        if cuda:
            SOURCE_FILE = progname + '.cu'
        directories.insert_cmake_template_lib(progname,dirpath,enable_mpi, enable_openmp, enable_root,cuda,SOURCE_FILE)
        create_cpp_file(xmlfile, dirpath, progname, modname, cuda)
        directories.move_model_files(xmlfile,dirpath)
        xmlfilename = xmlfile.split(os.path.sep)[-1]
        shutil.copyfile(xmlfile, os.path.join(dirpath,xmlfilename))

def generate_vectorized_network_lib(dirname, filename, modname, enable_mpi, enable_openmp, enable_root, enable_cuda):
    fn = filename[0]
    root=parse(fn)
    if mesh_algorithm_group(root) == True:
        if not enable_cuda:
            raise Exception('Vectorised mode (for MeshAlgorithmGroup/GridAlgorithmGroup) currently not supported without a CUDA enabled MIIND installation. ')
        # Run the MeshAlgorithm version
        produce_mesh_algorithm_version(dirname, filename, modname, root, enable_mpi, enable_openmp, enable_root, enable_cuda)
    else:
        # Simply run the old script
        directories.add_shared_library(dirname, filename, modname, enable_mpi, enable_openmp, enable_root, False)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Generate C++ from XML descriptions.')
    parser.add_argument('--d', help = 'Provide a packaging directory.',nargs = '?')
    parser.add_argument('-c', '--cuda', action="store_true", dest="cuda", help="if specified, cuda will be generated")
    parser.add_argument('-m','--m', help = 'A list of model and matrix files that will be copied to every executable directory.''', nargs='+')
    parser.add_argument('xml file', metavar='XML File', nargs = '*', help = 'Will create an entry in the build tree for each XML file, provided the XML file is valid.')
    parser.add_argument('--mpi', help = 'Enable MPI.', action='store_true')
    parser.add_argument('--openmp', help = 'Enable OPENMP.', action='store_true')
    parser.add_argument('--no_root', help = 'Disable ROOT.', action='store_true')

    args = parser.parse_args()


    filename = vars(args)['xml file']
    dirname  = vars(args)['d']
    modname  = vars(args)['m']
    enable_mpi = vars(args)['mpi']
    enable_openmp = vars(args)['openmp']
    disable_root = vars(args)['no_root']
    enable_cuda = vars(args)['cuda']

    fn = filename[0]
    root=parse(fn)
    if mesh_algorithm_group(root) == True:
        if not enable_cuda:
            raise Exception('Vectorised mode (for MeshAlgorithmGroup/GridAlgorithmGroup) currently not supported without a CUDA enabled MIIND installation. ')
        # Run the MeshAlgorithm version
        produce_mesh_algorithm_version(dirname, filename, modname, root, vars(args)['cuda'])
    else:
        # Simply run the old script
        if dirname == None:
            raise ValueError("This option is deprecated")
            fn = filename[0]
            directories.add_shared_library(fn,modname, enable_mpi, enable_openmp, disable_root)
        else:
            directories.add_shared_library(dirname, filename, modname, enable_mpi, enable_openmp, disable_root, enable_cuda)
