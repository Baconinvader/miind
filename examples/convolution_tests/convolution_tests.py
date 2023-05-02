import subprocess
import os
import sys
import shutil

if len(sys.argv) >= 2:
    print("Moving to:",sys.argv[1])
    os.chdir(sys.argv[1])

python_command =  'python'
if len(sys.argv) >= 3:
    print("Setting python run command to",sys.argv[2])
    python_command = sys.argv[2]

    

miindio_command = python_command + ' -m miind.miindio'
miindrun_command = python_command + ' -m miind.run'

test_descriptions = [
    'The RateAlgorithm Node is given an empty kernel:\n( <kernel> </kernel> )',
    'The RateAlgorithm Node is given a kernel with one value:\n( <kernel><value>1.0</value></kernel> )\n,using the name-based value declaration.',
    'The RateAlgorithm Node is given a kernel with multiple values:\n(<kernel>\n<value>0.25</value>\n<value>0.25</value>\n<value>0.25</value>\n<value>0.25</value>\n</kernel>),\nusing the name-based value declaration.',
    'The RateAlgorithm Node is given a kernel with multiple values:\n(<kernel>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n</kernel>),\nusing the attribute-based value declaration.',
    'A simulation is ran where the receiving node uses a GridAlgorithm instead of a MeshAlgorithm.\nThe RateAlgorithm Node is given a kernel with multiple values:\n(<kernel>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n</kernel>).\nBecause creating a LiF simulation setup is difficult with GridAlgorithm, a separate, conductance-based algorithm is used.'
    'A simulation is run where the receiving node uses a MeshAlgorithmGroup instead of a MeshAlgorithm, effectively meaning the simulation is run on the GPU.\nThe RateAlgorithm Node is given a kernel with multiple values:\n(<kernel>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n</kernel>).',
    'A simulation is ran where the receiving node uses a GridAlgorithmGroup instead of a MeshAlgorithm, effectively meaning the simulation is run on the GPU.\nThe RateAlgorithm Node is given a kernel with multiple values:\n(<kernel>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n<value v="0.25"/>\n</kernel>).\nBecause creating a LiF simulation setup is difficult with GridAlgorithm, a separate, conductance-based algorithm is used'
    ]

#This set of tests is designed to test convolution using a weighted kernel of past histories
#For the LiF simulations, the setup is as follows
#The simulation consists of a single LiF MeshAlgorithm Node with inputs from a single excitatory RateFunctor Node
#The LiF Node has a threshold potential of 20, a resting/reset potential of 0, and minimum potential of -25
#The RateFunctor Node has a firing rate of 800, and the efficacy of the connection between the two Nodes is 0.03.
#Generation of the relevant .mat,.model,.mesh,.res,.rev and .stat files can be done using the following MIINDIO commands
#> generate-lif-mesh lif 0.05 20 0 -25 0.001 1000
#> generate-model lif 0 20
#> generate-empty-fid lif
#> generate-matrix lif 0.03 10000 0.0 0.0 True

#For the conductance simulations, the setup is as follows
#The simulation consists of a single conductance GridAlgorithm Node with inputs from a single excitatory RateFunctor Node
#The conductance-nased Node has a starting potential of -0.065
#The RateFunctor Node has a firing rate of 800, and the efficacy of the connection between the two Nodes is 0.1
#Generation of the relevant .tmat,.model,.mesh,.res,.rev and .stat files can be done by running the generateCondFiles.py file in the /quick_start example directory

print("\n## Start Tests. ##")
for test in range(5):
    #setup
    os.chdir(str(test+1))

    desc = test_descriptions[test]
    print("#Test",test+1,"#\nDescription:\n"+desc,"\n")
    
    if os.path.exists("lif.xml"):
        name = "lif"
    else:
        name = "cond"
    if os.path.exists(name+"_"):
        shutil.rmtree(name+"_")

    print("Running",miindrun_command + " "+ name +".xml","...")
    #run  
    results = subprocess.run(miindrun_command + " "+ name +".xml", shell=True, check=True)
    print("Finished!")
    print("Results:",results,"\n")

    #cleanup
    if os.path.exists(name+"_"):
        shutil.rmtree(name+"_")
    if os.path.exists('miind_cwd'):
        os.remove('miind_cwd')
    os.chdir('..')
print("\n## All Tests complete. ##")
