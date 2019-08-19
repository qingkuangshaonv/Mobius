

#define MOBIUS_TIMESTEP_VERBOSITY 0
#define MOBIUS_TEST_FOR_NAN 0
#define MOBIUS_EQUATION_PROFILING 0
#define MOBIUS_PRINT_TIMING_INFO 0
#define MOBIUS_INDEX_BOUNDS_TESTS 0

#include "../../mobius_dll.h"

#include "../../Modules/Persist.h"
#include "../../Modules/INCA-Sed.h"



DLLEXPORT void *
DllSetupModel(char *ParameterFilename, char *InputFilename) {
    
	CHECK_ERROR_BEGIN
	
	mobius_model *Model = BeginModelDefinition("INCA-Sed", "0.0");
	
	AddPersistModel(Model);
	
	ReadInputDependenciesFromFile(Model, InputFilename);
	
	AddPersistModel(Model);
	AddINCASedModel(Model);
	
	mobius_data_set *DataSet = GenerateDataSet(Model);
	
	ReadParametersFromFile(DataSet, ParameterFilename);
	ReadInputsFromFile(DataSet, InputFilename);
	
	return (void *)DataSet;
	
	CHECK_ERROR_END
	
	return 0;
}