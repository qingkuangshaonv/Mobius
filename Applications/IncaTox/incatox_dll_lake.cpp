

#define MOBIUS_TIMESTEP_VERBOSITY 0
#define MOBIUS_TEST_FOR_NAN 0
#define MOBIUS_EQUATION_PROFILING 0
#define MOBIUS_PRINT_TIMING_INFO 0
#define MOBIUS_INDEX_BOUNDS_TESTS 0

#include "../../mobius_dll.h"

#include "../../Modules/Persist_Manning.h"
#include "../../Modules/INCA-Sed.h"
#include "../../Modules/SoilTemperature.h"
#include "../../Modules/WaterTemperature.h"
#include "../../Modules/SolarRadiation.h"
#include "../../Modules/INCA-Tox-C.h"
#include "../../Modules/INCA-Tox.h"
#include "../../Modules/INCA-Tox-Lake.h"


void
DllBuildModel(mobius_model *Model)
{
	Model->Name = "INCA-Tox";
	
	AddPersistModel(Model);
	AddINCASedModel(Model);
	AddSoilTemperatureModel(Model);
	AddWaterTemperatureModel(Model);
	AddMaxSolarRadiationModule(Model);
	AddIncaToxDOCModule(Model);
	AddIncaToxModule(Model);
	AddIncaToxLakeModule(Model);
}
