includes = ['#include <boost/timer/timer.hpp>','#include <GeomLib.hpp>','#include <TwoDLib.hpp>', '#include <MPILib/include/MPINetworkCode.hpp>','#include <MPILib/include/RateAlgorithmCode.hpp>','#include <MPILib/include/SimulationRunParameter.hpp>', '#include <MPILib/include/report/handler/RootReportHandler.hpp>','#include <MPILib/include/WilsonCowanAlgorithm.hpp>','#include <MPILib/include/PersistantAlgorithm.hpp>', '#include <MPILib/include/DelayAlgorithmCode.hpp>','#include <MPILib/include/RateFunctorCode.hpp>', '#include <display.hpp>','#include <MPILib/include/report/handler/InactiveReportHandler.hpp>']

lib_includes = ['#include <boost/timer/timer.hpp>','#include <GeomLib.hpp>','#include <TwoDLib.hpp>', '#include <MPILib/include/MPINetworkCode.hpp>','#include <MPILib/include/RateAlgorithmCode.hpp>','#include <MPILib/include/SimulationRunParameter.hpp>', '#ifdef ENABLE_ROOT_REPORTER\n#include <MPILib/include/report/handler/RootReportHandler.hpp>\n#else\n#include <MPILib/include/report/handler/InactiveReportHandler.hpp>\n#endif','#include <MPILib/include/WilsonCowanAlgorithm.hpp>','#include <MPILib/include/PersistantAlgorithm.hpp>', '#include <MPILib/include/DelayAlgorithmCode.hpp>','#include <MPILib/include/RateFunctorCode.hpp>','#include <MPILib/include/MiindTvbModelAbstract.hpp>', '#include <MPILib/include/utilities/Exception.hpp>', '#include <display.hpp>']
