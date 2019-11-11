
#if !defined(MOBIUS_MODEL_H)


//NOTE: The purpose of having unit_h, input_h, equation_h etc. be structs that contain a numeric handle rather than just letting them be a handle directly is that we can then use the C++ type system to get type safety. Unfortunately, C++ does not allow you to typedef a unique copy of a type that is not interchangable with others. However the type system WILL distinguish between two differently named structs even though they are otherwise equal.

typedef u32 entity_handle;


#define MODEL_ENTITY_HANDLE(Type) struct Type \
{ \
	entity_handle Handle; \
}; \
bool operator==(const Type &A, const Type &B) { return A.Handle == B.Handle; } \
bool operator!=(const Type &A, const Type &B) { return A.Handle != B.Handle; } \
bool operator<(const Type &A, const Type &B) { return A.Handle < B.Handle; } \
inline bool IsValid(Type H) { return H.Handle > 0; }

MODEL_ENTITY_HANDLE(module_h)

MODEL_ENTITY_HANDLE(unit_h)

MODEL_ENTITY_HANDLE(input_h)
MODEL_ENTITY_HANDLE(equation_h)
MODEL_ENTITY_HANDLE(parameter_double_h)
MODEL_ENTITY_HANDLE(parameter_uint_h)
MODEL_ENTITY_HANDLE(parameter_bool_h)
MODEL_ENTITY_HANDLE(parameter_time_h)

MODEL_ENTITY_HANDLE(solver_h)

MODEL_ENTITY_HANDLE(index_set_h)

MODEL_ENTITY_HANDLE(parameter_group_h)

#undef MODEL_ENTITY_HANDLE



/*
//NOTE: This alternative code (which complies more to certain standards of C++) produces the same result and is about as fast to compile, but it produces less legible error messages when the user makes a mistake.

enum entity_type
{
	EntityType_Parameter,
	EntityType_Input,
	EntityType_Equation,
	EntityType_ParameterDouble,
	EntityType_ParameterBool,
	EntityType_ParameterUInt,
	EntityType_ParameterTime,
	EntityType_Solver,
	EntityType_Unit,
	EntityType_IndexSet,
	EntityType_ParameterGroup,
};

template <entity_type Type>
struct handle_type
{
	entity_handle Handle;
};

template <entity_type Type>
bool operator==(const handle_type<Type> &A, const handle_type<Type> &B) {return A.Handle == B.Handle;}

template <entity_type Type>
bool operator!=(const handle_type<Type> &A, const handle_type<Type> &B) {return A.Handle != B.Handle;}

template <entity_type Type>
bool operator<(const handle_type<Type> &A, const handle_type<Type> &B) {return A.Handle < B.Handle;}

template <entity_type Type>
IsValid(handle_type<Type> H) { return H.Handle != 0; }

typedef handle_type<EntityType_Input> input_h;
typedef handle_type<EntityType_Equation> equation_h;
typedef handle_type<EntityType_ParameterDouble> parameter_double_h;
typedef handle_type<EntityType_ParameterBool> parameter_bool_h;
typedef handle_type<EntityType_ParameterUInt> parameter_uint_h;
typedef handle_type<EntityType_ParameterTime> parameter_time_h;
typedef handle_type<EntityType_Solver> solver_h;
typedef handle_type<EntityType_Unit> unit_h;
typedef handle_type<EntityType_IndexSet> index_set_h;
typedef handle_type<EntityType_ParameterGroup> parameter_group_h;
*/

enum entity_type   //NOTE: Is currently only used so that the storage_structure knows what it is storing and can ask the Model for the name associated to a handle if an error occurs.
{
	EntityType_Parameter,
	EntityType_Input,
	EntityType_Equation,
};

inline const char *
GetEntityTypeName(entity_type Type)
{
	//NOTE: It is important that this matches the above enum:
	const char *Typenames[3] = {"parameter", "input", "equation"};
	return Typenames[(size_t)Type];
}


struct index_t
{
	entity_handle IndexSetHandle;
	u32           Index;
	
	index_t() {};
	
	index_t(index_set_h IndexSet, u32 Index) : IndexSetHandle(IndexSet.Handle), Index(Index) {};
	
	index_t(entity_handle IndexSetHandle, u32 Index) : IndexSetHandle(IndexSetHandle), Index(Index) {};
	
	void operator++()
	{
		Index++;
	}
	
	index_t operator+(s32 Add) const
	{
		index_t Result = *this;
		Result.Index += Add;     //NOTE: This could result in an underflow. Have to see if that is a problem
		return Result;
	}
	
	index_t operator-(s32 Subtract) const
	{
		index_t Result = *this;
		Result.Index -= Subtract;     //NOTE: This could result in an underflow. Have to see if that is a problem
		return Result;
	}
	
	bool operator<=(const index_t& Other) const
	{
		//NOTE: This does NOT check if they came from a different index set. That check has to be done by the caller if needed.
		return Index <= Other.Index;
	}
	
	bool operator<(const index_t& Other) const
	{
		//NOTE: This does NOT check if they came from a different index set. That check has to be done by the caller if needed.
		return Index < Other.Index;
	}
	
	operator size_t() const
	{
		return (size_t)Index;
	}
};

union parameter_value
{
	double ValDouble;
	u64 ValUInt;
	u64 ValBool; //NOTE: Since this is a union we don't save space by making the bool smaller any way.
	datetime ValTime; //NOTE: From datetime.h
	
	parameter_value() : ValTime() {}; //NOTE: 0-initializes it.
};


enum parameter_type
{
	ParameterType_Double = 0,
	ParameterType_UInt,
	ParameterType_Bool,
	ParameterType_Time,
};

inline const char *
GetParameterTypeName(parameter_type Type)
{
	//NOTE: It is important that this matches the above parameter_type enum:
	const char *Typenames[4] = {"double", "uint", "bool", "time"};
	return Typenames[(size_t)Type];
}

struct parameter_spec
{
	const char *Name;
	parameter_type Type;
	parameter_value Min;
	parameter_value Max;
	parameter_value Default;
	
	unit_h Unit;
	const char *Description;
	
	equation_h IsComputedBy; //NOTE: We allow certain parameters to be computed by an initial value equation rather than being provided by a parameter file.
	bool ShouldNotBeExposed; //NOTE: Any user interface or file handler should not deal with a parameter if ShouldNotBeExposed = true;
	
	parameter_group_h Group;
	
	//NOTE: This not set before EndModelDefinition:
	//TODO: Should not really store it here though.
	std::vector<index_set_h> IndexSetDependencies;
};

struct unit_spec
{
	const char *Name;
	//NOTE: We don't need to put anything else here at the moment. Maybe eventually?
};

struct module_spec
{
	const char *Name;
	const char *Version;
};

struct model_run_state;

typedef std::function<double(model_run_state *)> mobius_equation;

typedef std::function<void(double *, double *)> mobius_solver_equation_function;
typedef std::function<void(size_t, size_t, double)> mobius_matrix_insertion_function;
typedef std::function<void(double *, mobius_matrix_insertion_function &)> mobius_solver_jacobi_function;

#define MOBIUS_SOLVER_FUNCTION(Name) void Name(double h, size_t n, double* x0, double* wk, const mobius_solver_equation_function &EquationFunction, const mobius_solver_jacobi_function &JacobiFunction, double AbsErr, double RelErr)
typedef MOBIUS_SOLVER_FUNCTION(mobius_solver_function);

struct parameter_group_spec
{
	const char *Name;
	std::vector<index_set_h> IndexSets;
	
	module_h Module;
	
	std::vector<entity_handle> Parameters;
};

enum index_set_type
{
	IndexSetType_Basic,
	IndexSetType_Branched,
};

struct index_set_spec
{
	const char *Name;
	index_set_type Type;
	std::vector<const char *> RequiredIndexes;
};

enum equation_type
{
	EquationType_Basic,
	EquationType_ODE,
	EquationType_InitialValue,
	EquationType_Cumulative,
};

inline const char *
GetEquationTypeName(equation_type Type)
{
	//NOTE: It is important that this matches the above enum:
	const char *Typenames[4] = {"basic", "ode", "initialvalue", "cumulative"};
	return Typenames[(size_t)Type];
}

struct dependency_registration
{
	entity_handle Handle;
	size_t NumExplicitIndexes;
};

struct result_dependency_registration
{
	entity_handle Handle;
	std::vector<index_t> Indexes;
};

//TODO: See if we could unionize some of the data below. Not everything is needed by every type of equation.
struct equation_spec
{
	const char *Name;
	equation_type Type;
	
	module_h Module;
	
	unit_h Unit;
	
	parameter_double_h InitialValue;
	double ExplicitInitialValue;
	bool HasExplicitInitialValue;
	equation_h InitialValueEquation;
	
	bool ResetEveryTimestep;         //NOTE: Only used for Type == EquationType_ODE.
	
	bool EquationIsSet;              //NOTE: Whether or not the equation body has been provided.
	
	index_set_h CumulatesOverIndexSet;   //NOTE: Only used for Type == EquationType_Cumulative.
	equation_h Cumulates;                //NOTE: Only used for Type == EquationType_Cumulative.
	parameter_double_h CumulationWeight; //NOTE: Only used for Type == EquationType_Cumulative.
	
	solver_h Solver;
		
		
	//NOTE: It would be nice to remove the following from the equation_spec and instead just store it in a temporary structure in EndModelDefinition, however it is reused in debug printouts etc. in the model run, so we have to store it in the model object somewhere anyway.
	
	//NOTE: The below are built during EndModelDefinition:
	std::set<index_set_h> IndexSetDependencies;          //NOTE: If the equation is run on a solver, the final index set dependencies of the equation will be those of the solver, not the ones stored here. You should generally use the storage structure to determine the final dependencies rather than this vector unless you are doing something specific in EndModelDefinition.
	std::set<entity_handle>  ParameterDependencies;
	std::set<input_h>     InputDependencies;
	std::set<equation_h>  DirectResultDependencies;
	std::set<equation_h>  DirectLastResultDependencies;
	std::set<equation_h>  CrossIndexResultDependencies;
	
	//TODO: The following should probably just be stored separately in a temporary structure in the EndModelDefinition procedure, as it is not reused outside of that procedure.
	std::vector<result_dependency_registration> IndexedResultAndLastResultDependencies;
	
	bool TempVisited; //NOTE: For use in a graph traversal algorithm while resolving dependencies in EndModelDefinition.
	bool Visited;     //NOTE: For use in a graph traversal algorithm while resolving dependencies in EndModelDefinition.
};

struct solver_spec
{
	const char *Name;
	
	double h;           //The desired step size of the solver when errors are tolerable (0.0 - 1.0)
	double RelErr;      //Relative error tolerance (used by some solvers)
	double AbsErr;      //Absolute error tolerance (used by some solvers)
	
	parameter_double_h hParam; //What parameter handle to read in h from (if this is provided).
	
	mobius_solver_function *SolverFunction;
	
	bool UsesErrorControl;
	bool UsesJacobian;
	
	//NOTE: It would be nice to remove the following from the solver_spec and instead just store it in a temporary structure in EndModelDefinition, however it is reused in debug printouts etc. in the model run, so we have to store it in the model object somewhere anyway.
	
	//NOTE: These are built during EndModelDefinition:
	std::set<index_set_h> IndexSetDependencies;
	std::vector<equation_h> EquationsToSolve;
	std::set<equation_h> DirectResultDependencies;
	std::set<equation_h> CrossIndexResultDependencies;
	
	//TODO: The following should probably just be stored separately in a temporary structure in the EndModelDefinition procedure, as it is not reused outside of that procedure.
	bool TempVisited; //NOTE: For use in a graph traversal algorithm while resolving dependencies in EndModelDefinition. 
	bool Visited;     //NOTE: For use in a graph traversal algorithm while resolving dependencies in EndModelDefinition.
};

struct input_spec
{
	const char *Name;
	
	unit_h Unit;
	
	bool IsAdditional; //If it was user-specified in the input file, as opposed to being registered by a model building procedure.
	
	std::vector<index_set_h> IndexSetDependencies;
};


//TODO: Find a better name for this struct?
struct iteration_data
{
	array<entity_handle> ParametersToRead;
	array<input_h>       InputsToRead;
	array<equation_h>    ResultsToRead;
	array<equation_h>    LastResultsToRead;
};

enum equation_batch_type
{
	BatchType_Regular,
	BatchType_Solver,
};

struct equation_batch
{
	equation_batch_type Type;
	solver_h Solver;                //NOTE: Only for Type==BatchType_Solver.
	array<equation_h> Equations;
	array<equation_h> EquationsODE; //NOTE: Only for Type==BatchType_Solver.
	
	//NOTE: These are used for optimizing estimation of the Jacobian in case that is needed by a solver.
	array<array<size_t>>     ODEIsDependencyOfODE;
	array<array<equation_h>> ODEIsDependencyOfNonODE;
	
	array<equation_h> InitialValueOrder; //NOTE: The initial value setup of equations happens in a different order than the execution order during model run because the intial value equations may have different dependencies than the equations they are initial values for.
};

struct equation_batch_group
{
	array<index_set_h> IndexSets;
	
	array<equation_h>     LastResultsToReadAtBase;  //Unfortunately we need this for LAST_RESULTs of equations with 0 index set dependencies.
	array<iteration_data> IterationData;
	size_t FirstBatch;
	size_t LastBatch;
};


struct storage_unit_specifier
{
	array<index_set_h> IndexSets;
	array<entity_handle> Handles;
};

struct mobius_model;

struct storage_structure
{
	array<storage_unit_specifier> Units;
	
	size_t *TotalCountForUnit;
	size_t *OffsetForUnit;
	size_t *UnitForHandle;
	size_t *LocationOfHandleInUnit;       // Units[UnitForHandle[H]].Handles[LocationOfHandleInUnit[H]] == H;
	size_t TotalCount;
	
	bool HasBeenSetUp = false;
	
	//NOTE: The following two are only here in case we need to look up the name of an index set or handle when reporting an error about misindexing if the MOBIUS_INDEX_BOUNDS_TESTS is turned on. It is not that clean to have this information here, though :(
	const mobius_model *Model;
	entity_type Type;
};

typedef std::unordered_map<token_string, entity_handle, token_string_hash_function> string_map;

struct mobius_data_set;
typedef std::function<void(mobius_data_set *)> mobius_preprocessing_step;


template <typename spec_type>
struct entity_registry
{
	std::vector<spec_type> Specs;
	string_map NameToHandle;   // Entries are organized so that Specs[NameToHandle["Some name"]].Name == "Some name";
	
	entity_registry()
	{
		Specs.push_back({}); //NOTE: Reserving the 0 index as invalid.
	}
	
	entity_handle Register(const char *Name)
	{
		auto Find = NameToHandle.find(Name);
		if(Find != NameToHandle.end())
		{
			return Find->second;
		}
		else
		{
			entity_handle Handle = (entity_handle)Specs.size();
			Specs.push_back({});
			Specs[Handle].Name = Name;
			NameToHandle[Name] = Handle;
			return Handle;
		}
	}
	
	size_t Count() const
	{
		return Specs.size();
	}
};


struct mobius_model
{
	const char *Name;
	
	bucket_allocator BucketMemory;  //NOTE: Used for batch group structure and some string copies.
	
	module_h CurrentModule = {};
	
	entity_registry<module_spec> Modules;
	
	entity_registry<equation_spec> Equations;
	std::vector<mobius_equation> EquationBodies;
	
	entity_registry<input_spec> Inputs;
	
	entity_registry<parameter_spec> Parameters;
	
	entity_registry<index_set_spec> IndexSets;
	
	entity_registry<parameter_group_spec> ParameterGroups;
	
	entity_registry<solver_spec> Solvers;
	
	entity_registry<unit_spec> Units;
	
	array<equation_batch> EquationBatches;
	array<equation_batch_group> BatchGroups;
	//std::vector<equation_batch> EquationBatches;
	//std::vector<equation_batch_group> BatchGroups;
	
	std::vector<mobius_preprocessing_step> PreprocessingSteps;
	
	timer DefinitionTimer;
	bool Finalized;
	
	~mobius_model();
};


template<typename batch_like>
inline void
ForAllBatchEquations(const batch_like &Batch, std::function<bool(equation_h)> Do)
{
	bool ShouldBreak = false;
	for(equation_h Equation : Batch.Equations)
	{
		ShouldBreak = Do(Equation);
		if(ShouldBreak) break;
	}
	if(!ShouldBreak && Batch.Type == BatchType_Solver)
	{
		for(equation_h Equation : Batch.EquationsODE)
		{
			ShouldBreak = Do(Equation);
			if(ShouldBreak) break;
		}
	}
}


//TODO: The name "inputs" here is confusing, since there is already a different concept called input.
//TODO: Couldn't this just be a std::vector?
/*
struct branch_inputs
{
	size_t Count;
	index_t *Inputs;
};
*/

struct mobius_data_set
{
	const mobius_model *Model;
	
	bucket_allocator BucketMemory;   //NOTE: Important! This should only be used for storage of "small" arrays such as IndexCounts or IndexNames, NOT the large arrays such as InputData or ResultData.
	
	parameter_value *ParameterData;
	storage_structure ParameterStorageStructure;
	
	double *hSolver;
	
	double *InputData;
	bool   *InputTimeseriesWasProvided;
	storage_structure InputStorageStructure;
	datetime InputDataStartDate;
	bool InputDataHasSeparateStartDate = false; //NOTE: Whether or not a start date was provided for the input data, which is potentially different from the start date of the model run.
	u64 InputDataTimesteps;
	
	double *ResultData;
	storage_structure ResultStorageStructure;
	
	index_t *IndexCounts;
	const char ***IndexNames;  // IndexNames[IndexSet.Handle][IndexNamesToHandle[IndexSet.Handle][IndexName]] == IndexName;
	std::vector<string_map> IndexNamesToHandle;
	bool AllIndexesHaveBeenSet;
	
	//TODO: could make this array<array<array<index_t>>>, but I don't know if it improves the code..
	array<index_t> **BranchInputs; //BranchInputs[ReachIndexSet][ReachIndex] ...

	bool HasBeenRun;
	u64 TimestepsLastRun;
	datetime StartDateLastRun;
	
	
	~mobius_data_set();
};

#if !defined(MOBIUS_EQUATION_PROFILING)
#define MOBIUS_EQUATION_PROFILING 0
#endif

struct model_run_state
{
	// The purpose of the model_run_state is to store temporary state that is needed during a model run as well as providing an access point to data that is needed when evaluating equations.
	// There are two use cases.
	// If Running=false, this is a setup run, where the purpose is to register the accesses of all the equations to later determine their dependencies.
	// If Running=true, this is the actual run of the model, where equations should have access to the actual parameter values and so on.
	
	bool Running;
	const mobius_model *Model;
	mobius_data_set *DataSet;
	
	s32 Year;
	s32 DayOfYear;
	s32 DaysThisYear;
	s64 Timestep; //NOTE: We make this a signed integer so that it can be set to -1 during the "initial value" step.


	bucket_allocator BucketMemory;

	//NOTE: For use during model execution		
	parameter_value *CurParameters;
	double          *CurResults;
	double          *LastResults;
	double          *CurInputs;
	bool            *CurInputWasProvided;
	
	index_t *CurrentIndexes; //NOTE: Contains the current index of each index set during execution.	
	
	double *AllCurResultsBase;
	double *AllLastResultsBase;
	
	double *AllCurInputsBase;
	
	double *AtResult;
	double *AtLastResult;
	
	array<parameter_value> FastParameterLookup;
	array<size_t>          FastInputLookup;
	array<size_t>          FastResultLookup;
	array<size_t>          FastLastResultLookup;
	
	parameter_value *AtParameterLookup;
	size_t *AtInputLookup;
	size_t *AtResultLookup;
	size_t *AtLastResultLookup;
	
	double *SolverTempX0;          //NOTE: Temporary storage for use by solvers
	double *SolverTempWorkStorage; //NOTE: Temporary storage for use by solvers
	double *JacobianTempStorage;   //NOTE: Temporary storage for use by Jacobian estimation
	

	
	//NOTE: For use during dependency registration:
	std::vector<dependency_registration> ParameterDependencies;
	std::vector<dependency_registration> InputDependencies;
	std::vector<result_dependency_registration> ResultDependencies;
	std::vector<result_dependency_registration> LastResultDependencies;
	std::vector<index_set_h> DirectIndexSetDependencies;

	
#if MOBIUS_EQUATION_PROFILING
	size_t *EquationHits;
	u64 *EquationTotalCycles;
#endif
	
	//NOTE: For dependency registration run:
	model_run_state(const mobius_model *Model)
	{
		Running = false;
		DataSet = 0;
		this->Model = Model;
	}
	
	//NOTE: For proper run:
	model_run_state(mobius_data_set *DataSet)
	{
		Running = true;
		this->DataSet = DataSet;
		this->Model = DataSet->Model;

		BucketMemory.Initialize(1024*1024);

		CurInputs      = BucketMemory.Allocate<double>(Model->Inputs.Count());
		CurParameters  = BucketMemory.Allocate<parameter_value>(Model->Parameters.Count());
		CurResults     = BucketMemory.Allocate<double>(Model->Equations.Count());
		LastResults    = BucketMemory.Allocate<double>(Model->Equations.Count());
		CurInputWasProvided = BucketMemory.Allocate<bool>(Model->Inputs.Count());
		
		CurrentIndexes = BucketMemory.Allocate<index_t>(Model->IndexSets.Count());
		for(entity_handle IndexSetHandle = 1; IndexSetHandle < Model->IndexSets.Count(); ++IndexSetHandle)
		{
			CurrentIndexes[IndexSetHandle].IndexSetHandle = IndexSetHandle;
		}
		
		DayOfYear = 0;
		DaysThisYear = 365;
		Timestep = 0;
		
		SolverTempX0 = nullptr;
		SolverTempWorkStorage = nullptr;
		JacobianTempStorage = nullptr;
	}
	
	~model_run_state()
	{
		if(Running)
		{
			BucketMemory.DeallocateAll();
		}
	}
	
	void Clear()
	{
		if(Running)
		{
			memset(CurParameters,  0, sizeof(parameter_value)*Model->Parameters.Count());
			memset(CurInputs,      0, sizeof(double)*Model->Inputs.Count());
			memset(CurInputWasProvided,      0, sizeof(bool)*Model->Inputs.Count());
			memset(CurResults,     0, sizeof(double)*Model->Equations.Count());
			memset(LastResults,    0, sizeof(double)*Model->Equations.Count());
			memset(CurrentIndexes, 0, sizeof(index_t)*Model->IndexSets.Count());
			
			for(entity_handle IndexSetHandle = 1; IndexSetHandle < Model->IndexSets.Count(); ++IndexSetHandle)
			{
				CurrentIndexes[IndexSetHandle].IndexSetHandle = IndexSetHandle;
			}
		}
		else
		{
			ParameterDependencies.clear();
			InputDependencies.clear();
			ResultDependencies.clear();
			LastResultDependencies.clear();
			DirectIndexSetDependencies.clear();
		}
	}
};

inline double
CallEquation(const mobius_model *Model, model_run_state *RunState, equation_h Equation)
{
#if MOBIUS_EQUATION_PROFILING
	u64 Begin = __rdtsc();
#endif
	double ResultValue = Model->EquationBodies[Equation.Handle](RunState);
#if MOBIUS_EQUATION_PROFILING
	u64 End = __rdtsc();
	RunState->EquationHits[Equation.Handle]++;
	RunState->EquationTotalCycles[Equation.Handle] += (End - Begin);
#endif
	return ResultValue;
}



#define GET_ENTITY_NAME(Type, NType) \
inline const char * GetName(const mobius_model *Model, Type H) \
{ \
	return Model->NType.Specs[H.Handle].Name; \
}

GET_ENTITY_NAME(equation_h, Equations)
GET_ENTITY_NAME(input_h, Inputs)
GET_ENTITY_NAME(parameter_double_h, Parameters)
GET_ENTITY_NAME(parameter_uint_h, Parameters)
GET_ENTITY_NAME(parameter_bool_h, Parameters)
GET_ENTITY_NAME(parameter_time_h, Parameters)
GET_ENTITY_NAME(index_set_h, IndexSets)
GET_ENTITY_NAME(parameter_group_h, ParameterGroups)
GET_ENTITY_NAME(solver_h, Solvers)
GET_ENTITY_NAME(unit_h, Units)

#undef GET_ENTITY_NAME

inline const char *
GetParameterName(const mobius_model *Model, entity_handle ParameterHandle) //NOTE: In case we don't know the type of the parameter and just want the name.
{
	return Model->Parameters.Specs[ParameterHandle].Name;
}

inline const char *
GetName(const mobius_model *Model, entity_type Type, entity_handle Handle)
{
	switch(Type)
	{
		case EntityType_Parameter:
		return GetParameterName(Model, Handle);
		break;
		
		case EntityType_Input:
		return GetName(Model, input_h {Handle});
		break;
		
		case EntityType_Equation:
		return GetName(Model, equation_h {Handle});
		break;
		
		default:
		MOBIUS_FATAL_ERROR("ERROR: (internal) ended up with wrong entity type!?" << std::endl);
		return "unknown entity type";
	}
}

#define GET_ENTITY_HANDLE(Type, Registry, Typename) \
inline Type Get##Typename##Handle(const mobius_model *Model, const token_string &Name) \
{ \
	entity_handle Handle = 0; \
	auto Find = Model->Registry.NameToHandle.find(Name); \
	if(Find != Model->Registry.NameToHandle.end()) \
	{ \
		Handle = Find->second; \
	} \
	else \
	{ \
		MOBIUS_FATAL_ERROR("ERROR: Tried to look up the handle of the " << #Typename << " \"" << Name << "\", but it was not registered with the model." << std::endl); \
	} \
	return { Handle }; \
}

GET_ENTITY_HANDLE(equation_h, Equations, Equation)
GET_ENTITY_HANDLE(input_h, Inputs, Input)
GET_ENTITY_HANDLE(parameter_double_h, Parameters, ParameterDouble)
GET_ENTITY_HANDLE(parameter_uint_h, Parameters, ParameterUInt)
GET_ENTITY_HANDLE(parameter_bool_h, Parameters, ParameterBool)
GET_ENTITY_HANDLE(parameter_time_h, Parameters, ParameterTime)
GET_ENTITY_HANDLE(index_set_h, IndexSets, IndexSet)
GET_ENTITY_HANDLE(parameter_group_h, ParameterGroups, ParameterGroup)
GET_ENTITY_HANDLE(solver_h, Solvers, Solver)
GET_ENTITY_HANDLE(module_h, Modules, Module)

#undef GET_ENTITY_HANDLE

inline entity_handle
GetParameterHandle(const mobius_model *Model, const token_string &Name) //NOTE: In case we don't know the type of the parameter and just want the handle.
{
	entity_handle Handle = 0;
	auto Find = Model->Parameters.NameToHandle.find(Name);
	if(Find != Model->Parameters.NameToHandle.end())
	{
		Handle = Find->second;
	}
	else
	{
		MOBIUS_FATAL_ERROR("ERROR: Tried to find the Parameter \"" << Name << "\", but it was not registered with the model." << std::endl);
	}
	return Handle;
}


#define REGISTRATION_BLOCK(Model) \
if(Model->Finalized) \
{ \
	MOBIUS_FATAL_ERROR("ERROR: You can not call the function " << __func__ << " on the model after it has been finalized using EndModelDefinition." << std::endl); \
}



void AddPreprocessingStep(mobius_model *Model, mobius_preprocessing_step PreprocessingStep)
{
	REGISTRATION_BLOCK(Model);
	
	Model->PreprocessingSteps.push_back(PreprocessingStep);
}

inline unit_h
RegisterUnit(mobius_model *Model, const char *Name = "dimensionless")
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Unit = Model->Units.Register(Name);
	
	return {Unit};
}

inline module_h
BeginModule(mobius_model *Model, const char *Name, const char *Version)
{
	entity_handle Module = Model->Modules.Register(Name);
	
	Model->Modules.Specs[Module].Version = Version;
	
	Model->CurrentModule = {Module};
	
	return {Module};
}

inline void
EndModule(mobius_model *Model)
{
	Model->CurrentModule = {};
}

inline index_set_h
RegisterIndexSet(mobius_model *Model, const char *Name, index_set_type Type = IndexSetType_Basic)
{
	REGISTRATION_BLOCK(Model)

	entity_handle IndexSet = Model->IndexSets.Register(Name);
	
	Model->IndexSets.Specs[IndexSet].Type = Type;
	
	return {IndexSet};
}

inline index_set_h
RegisterIndexSetBranched(mobius_model *Model, const char *Name)
{
	REGISTRATION_BLOCK(Model)
	
	index_set_h IndexSet = RegisterIndexSet(Model, Name, IndexSetType_Branched);
	
	return IndexSet;
}

inline index_t
RequireIndex(mobius_model *Model, index_set_h IndexSet, const char *IndexName)
{
	REGISTRATION_BLOCK(Model)
	
	index_set_spec &Spec = Model->IndexSets.Specs[IndexSet.Handle];
	if(Spec.Type != IndexSetType_Basic)
	{
		//TODO: Get rid of this requirement? However that may lead to issues with index order in branched index sets later.
		MOBIUS_FATAL_ERROR("ERROR: We only allow requiring indexes for basic index sets, " << Spec.Name << " is of a different type." << std::endl);
	}
	auto Find = std::find(Spec.RequiredIndexes.begin(), Spec.RequiredIndexes.end(), IndexName);
	if(Find != Spec.RequiredIndexes.end())
	{
		return index_t(IndexSet, (u32)std::distance(Spec.RequiredIndexes.begin(), Find)); //NOTE: This is its position in the vector.
	}
	else
	{
		Spec.RequiredIndexes.push_back(IndexName);
		return index_t(IndexSet, (u32)(Spec.RequiredIndexes.size() - 1));
	}
}


template<typename... T>
inline parameter_group_h
RegisterParameterGroup(mobius_model *Model, const char *Name, T... IndexSets)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle ParameterGroup = Model->ParameterGroups.Register(Name);
	
	Model->ParameterGroups.Specs[ParameterGroup].IndexSets = {IndexSets...};
	Model->ParameterGroups.Specs[ParameterGroup].Module = Model->CurrentModule;
	
	return {ParameterGroup};
}


inline input_h
RegisterInput(mobius_model *Model, const char *Name, unit_h Unit = {0}, bool IsAdditional = false)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Input = Model->Inputs.Register(Name);

	input_spec &Spec = Model->Inputs.Specs[Input];
	Spec.IsAdditional = IsAdditional;
	Spec.Unit = Unit;
	
	return {Input};
}

inline parameter_double_h
RegisterParameterDouble(mobius_model *Model, parameter_group_h Group, const char *Name, unit_h Unit, double Default, double Min = -DBL_MAX, double Max = DBL_MAX, const char *Description = 0)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Parameter = Model->Parameters.Register(Name);
	
	parameter_spec &Spec = Model->Parameters.Specs[Parameter];
	Spec.Type = ParameterType_Double;
	Spec.Default.ValDouble = Default;
	Spec.Min.ValDouble = Min;
	Spec.Max.ValDouble = Max;
	Spec.Group = Group;
	Spec.Unit = Unit;
	Spec.Description = Description;
	
	Model->ParameterGroups.Specs[Group.Handle].Parameters.push_back(Parameter);

	return {Parameter};
}

inline parameter_uint_h
RegisterParameterUInt(mobius_model *Model, parameter_group_h Group, const char *Name, unit_h Unit, u64 Default, u64 Min = 0, u64 Max = 0xffffffffffffffff, const char *Description = 0)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Parameter = Model->Parameters.Register(Name);
	
	parameter_spec &Spec = Model->Parameters.Specs[Parameter];
	Spec.Type = ParameterType_UInt;
	Spec.Default.ValUInt = Default;
	Spec.Min.ValUInt = Min;
	Spec.Max.ValUInt = Max;
	Spec.Group = Group;
	Spec.Unit = Unit;
	Spec.Description = Description;
	
	Model->ParameterGroups.Specs[Group.Handle].Parameters.push_back(Parameter);
	
	return {Parameter};
}

inline parameter_bool_h
RegisterParameterBool(mobius_model *Model, parameter_group_h Group, const char *Name, bool Default, const char *Description = 0)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Parameter = Model->Parameters.Register(Name);
	
	parameter_spec &Spec = Model->Parameters.Specs[Parameter];
	Spec.Type = ParameterType_Bool;
	Spec.Default.ValBool = Default;
	Spec.Min.ValBool = false;
	Spec.Max.ValBool = true;
	Spec.Group = Group;
	Spec.Description = Description;
	
	Model->ParameterGroups.Specs[Group.Handle].Parameters.push_back(Parameter);
	
	return {Parameter};
}

inline parameter_time_h
RegisterParameterDate(mobius_model *Model, parameter_group_h Group, const char *Name, const char *Default, const char *Min = "1000-1-1", const char *Max = "3000-12-31", const char *Description = 0)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Parameter = Model->Parameters.Register(Name);
	
	parameter_spec &Spec = Model->Parameters.Specs[Parameter];
	Spec.Type = ParameterType_Time;
	
	bool ParseSuccessAll = true;
	bool ParseSuccess;
	Spec.Default.ValTime = datetime(Default, &ParseSuccess);
	ParseSuccessAll = ParseSuccessAll && ParseSuccess;
	Spec.Min.ValTime = datetime(Min, &ParseSuccess);
	ParseSuccessAll = ParseSuccessAll && ParseSuccess;
	Spec.Max.ValTime = datetime(Max, &ParseSuccess);
	ParseSuccessAll = ParseSuccessAll && ParseSuccess;
	
	if(!ParseSuccessAll)
	{
		MOBIUS_FATAL_ERROR("ERROR: Unrecognized date format for default, min or max value when registering the parameter " << Name << std::endl);
	}
	
	Spec.Group = Group;
	Spec.Description = Description;
	
	Model->ParameterGroups.Specs[Group.Handle].Parameters.push_back(Parameter);

	return {Parameter};
}

inline void
ParameterIsComputedBy(mobius_model *Model, parameter_double_h Parameter, equation_h Equation, bool ShouldNotBeExposed = true)
{
	REGISTRATION_BLOCK(Model)
	
	parameter_spec &Spec  = Model->Parameters.Specs[Parameter.Handle];
	equation_spec &EqSpec = Model->Equations.Specs[Equation.Handle];
	if(EqSpec.Type != EquationType_InitialValue)
	{
		MOBIUS_FATAL_ERROR("ERROR: Tried to set the equation " << EqSpec.Name << " to compute the parameter " << Spec.Name << ", but " << EqSpec.Name << " is not an initial value equation." << std::endl);
	}
	if(EqSpec.Unit != Spec.Unit)
	{
		MOBIUS_FATAL_ERROR("ERROR: The equation " << EqSpec.Name << " has a different unit from the parameter " << Spec.Name << ", that it is trying to compute." << std::endl);
	}
	
	Spec.IsComputedBy = Equation;
	Spec.ShouldNotBeExposed = ShouldNotBeExposed;
}

inline void
ParameterIsComputedBy(mobius_model *Model, parameter_uint_h Parameter, equation_h Equation, bool ShouldNotBeExposed = true)
{
	REGISTRATION_BLOCK(Model)
	
	parameter_spec &Spec  = Model->Parameters.Specs[Parameter.Handle];
	equation_spec &EqSpec = Model->Equations.Specs[Equation.Handle];
	if(EqSpec.Type != EquationType_InitialValue)
	{
		MOBIUS_FATAL_ERROR("ERROR: Tried to set the equation " << EqSpec.Name << " to compute the parameter " << Spec.Name << ", but " << EqSpec.Name << " is not an initial value equation." << std::endl);
	}
	if(EqSpec.Unit != Spec.Unit)
	{
		MOBIUS_FATAL_ERROR("ERROR: The equation " << EqSpec.Name << " has a different unit from the parameter " << Spec.Name << ", that it is trying to compute." << std::endl);
	}
	
	Spec.IsComputedBy = Equation;
	Spec.ShouldNotBeExposed = ShouldNotBeExposed;
}

inline void
SetEquation(mobius_model *Model, equation_h Equation, mobius_equation EquationBody, bool Override = false)
{
	//REGISTRATION_BLOCK(Model) //NOTE: We can't use REGISTRATION_BLOCK since the user don't call the SetEquation explicitly, it is called through the macro EQUATION, and so they would not understand the error message.
	if(Model->Finalized)
	{
		MOBIUS_FATAL_ERROR("ERROR: You can not define an EQUATION body for the model after it has been finalized using EndModelDefinition." << std::endl);
	}
	
	if(!Override && Model->Equations.Specs[Equation.Handle].EquationIsSet)
	{
		MOBIUS_FATAL_ERROR("ERROR: The equation body for " << GetName(Model, Equation) << " is already defined. It can not be defined twice unless it is explicitly overridden." << std::endl);
	}
	
	Model->EquationBodies[Equation.Handle] = EquationBody;

	Model->Equations.Specs[Equation.Handle].EquationIsSet = true;
}

static equation_h
RegisterEquation(mobius_model *Model, const char *Name, unit_h Unit, equation_type Type = EquationType_Basic)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Equation = Model->Equations.Register(Name);
	
	if(Model->EquationBodies.size() <= Equation)
	{
		Model->EquationBodies.resize(Equation + 1, {});
	}
	
	Model->Equations.Specs[Equation].Type = Type;
	Model->Equations.Specs[Equation].Unit = Unit;
	Model->Equations.Specs[Equation].Module = Model->CurrentModule;
	
	return {Equation};
}

inline equation_h
RegisterEquationODE(mobius_model *Model, const char *Name, unit_h Unit)
{
	REGISTRATION_BLOCK(Model)
	
	return RegisterEquation(Model, Name, Unit, EquationType_ODE);
}

inline equation_h
RegisterEquationInitialValue(mobius_model *Model, const char *Name, unit_h Unit)
{
	REGISTRATION_BLOCK(Model)
	
	return RegisterEquation(Model, Name, Unit, EquationType_InitialValue);
}

//NOTE: CumulateResult is implemented in mobius_data_set.cpp
static double CumulateResult(mobius_data_set *DataSet, equation_h Result, index_set_h CumulateOverIndexSet, index_t *CurrentIndexes, double *LookupBase);
static double CumulateResult(mobius_data_set *DataSet, equation_h Result, index_set_h CumulateOverIndexSet, index_t *CurrentIndexes, double *LookupBase, parameter_double_h Weight);

inline equation_h
RegisterEquationCumulative(mobius_model *Model, const char *Name, equation_h Cumulates, index_set_h CumulatesOverIndexSet, parameter_double_h Weight = {})
{
	REGISTRATION_BLOCK(Model)
	
	equation_spec &CumulateSpec = Model->Equations.Specs[Cumulates.Handle];
	if(CumulateSpec.Type == EquationType_InitialValue)
	{
		MOBIUS_FATAL_ERROR("ERROR: The cumulation equation " << Name << " was set to cumulate an initial value equation (" << CumulateSpec.Name << "). This is not supported." << std::endl);
	}
	
	unit_h Unit = Model->Equations.Specs[Cumulates.Handle].Unit;
	equation_h Equation = RegisterEquation(Model, Name, Unit, EquationType_Cumulative);
	Model->Equations.Specs[Equation.Handle].CumulatesOverIndexSet = CumulatesOverIndexSet;
	Model->Equations.Specs[Equation.Handle].Cumulates = Cumulates;
	Model->Equations.Specs[Equation.Handle].CumulationWeight = Weight;
	
	if(IsValid(Weight))
	{
		SetEquation(Model, Equation,
			[Cumulates, CumulatesOverIndexSet, Weight] (model_run_state *RunState) -> double
			{
				return CumulateResult(RunState->DataSet, Cumulates, CumulatesOverIndexSet, RunState->CurrentIndexes, RunState->AllCurResultsBase, Weight);
			}
		);
	}
	else
	{
		SetEquation(Model, Equation,
			[Cumulates, CumulatesOverIndexSet] (model_run_state *RunState) -> double
			{
				return CumulateResult(RunState->DataSet, Cumulates, CumulatesOverIndexSet, RunState->CurrentIndexes, RunState->AllCurResultsBase);
			}
		);
	}
	
	return Equation;
}

//TODO: Give warnings or errors when setting a initial value on an equation that already has one.
inline void
SetInitialValue(mobius_model *Model, equation_h Equation, parameter_double_h InitialValue)
{
	REGISTRATION_BLOCK(Model)
	
	Model->Equations.Specs[Equation.Handle].InitialValue = InitialValue;
	
	if(Model->Equations.Specs[Equation.Handle].Unit != Model->Parameters.Specs[InitialValue.Handle].Unit)
	{
		std::cout << "WARNING: The equation " << GetName(Model, Equation) << " was registered with a different unit than its initial value parameter " << GetName(Model, InitialValue) << std::endl;
	}
}

inline void
SetInitialValue(mobius_model *Model, equation_h Equation, double Value)
{
	REGISTRATION_BLOCK(Model)
	
	Model->Equations.Specs[Equation.Handle].ExplicitInitialValue = Value;
	Model->Equations.Specs[Equation.Handle].HasExplicitInitialValue = true;
}

inline void
SetInitialValue(mobius_model *Model, equation_h Equation, equation_h InitialValueEquation)
{
	REGISTRATION_BLOCK(Model)
	if(Model->Equations.Specs[InitialValueEquation.Handle].Type != EquationType_InitialValue)
	{
		MOBIUS_FATAL_ERROR("ERROR: Tried to set the equation " << GetName(Model, InitialValueEquation) << " as an initial value of another equation, but it was not registered as an equation of type EquationInitialValue." << std::endl);
	}
	
	Model->Equations.Specs[Equation.Handle].InitialValueEquation = InitialValueEquation;
	
	if(Model->Equations.Specs[Equation.Handle].Unit != Model->Equations.Specs[InitialValueEquation.Handle].Unit)
	{
		std::cout << "WARNING: The equation " << GetName(Model, Equation) << " was registered with a different unit than its initial value equation " << GetName(Model, InitialValueEquation) << std::endl;
	}
}

inline void
ResetEveryTimestep(mobius_model *Model, equation_h Equation)
{
	REGISTRATION_BLOCK(Model)
	
	equation_spec &Spec = Model->Equations.Specs[Equation.Handle];
	
	if(Spec.Type != EquationType_ODE)
	{
		MOBIUS_FATAL_ERROR("ERROR: Called ResetEveryTimestep on the equation " << Spec.Name << ", but this functionality is only available for ODE equations." << std::endl);
	}
	
	Spec.ResetEveryTimestep = true;
}

#define MOBIUS_SOLVER_SETUP_FUNCTION(Name) void Name(solver_spec *SolverSpec)
typedef MOBIUS_SOLVER_SETUP_FUNCTION(mobius_solver_setup_function);


static solver_h
RegisterSolver(mobius_model *Model, const char *Name, parameter_double_h hParam, mobius_solver_setup_function *SetupFunction)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Solver = Model->Solvers.Register(Name);
	
	solver_spec &Spec = Model->Solvers.Specs[Solver];
	
	SetupFunction(&Spec);
	
	Spec.hParam = hParam;
	
	return {Solver};
}

static solver_h
RegisterSolver(mobius_model *Model, const char *Name, double h, mobius_solver_setup_function *SetupFunction)
{
	REGISTRATION_BLOCK(Model)
	
	entity_handle Solver = Model->Solvers.Register(Name);
	
	solver_spec &Spec = Model->Solvers.Specs[Solver];
	
	SetupFunction(&Spec);
	
	Spec.h = h;
	
	return {Solver};
}

//TODO: Make version of this that takes parametric h too.
static solver_h
RegisterSolver(mobius_model *Model, const char *Name, double h, mobius_solver_setup_function *SetupFunction, double RelErr, double AbsErr)
{
	REGISTRATION_BLOCK(Model)
	
	solver_h Solver = RegisterSolver(Model, Name, h, SetupFunction);
	
	solver_spec &Spec = Model->Solvers.Specs[Solver.Handle];
	
	if(!Spec.UsesErrorControl)
	{
		std::cout << "WARNING: Registered error tolerances with the solver " << Name << ", but the attached solver function does not support error control." << std::endl;
	}
	
	Spec.RelErr = RelErr;
	Spec.AbsErr = AbsErr;
	
	return Solver;
}

static void
SetSolver(mobius_model *Model, equation_h Equation, solver_h Solver)
{
	REGISTRATION_BLOCK(Model)
	
	equation_type Type = Model->Equations.Specs[Equation.Handle].Type;
	if(Type != EquationType_Basic && Type != EquationType_ODE)
	{
		MOBIUS_FATAL_ERROR("ERROR: Tried to set a solver for the equation " << GetName(Model, Equation) << ", but it is not a basic equation or ODE equation, and so can not be given a solver." << std::endl);
	}
	Model->Equations.Specs[Equation.Handle].Solver = Solver;
}

inline void
AddInputIndexSetDependency(mobius_model *Model, input_h Input, index_set_h IndexSet)
{
	REGISTRATION_BLOCK(Model)
	
	input_spec &Spec = Model->Inputs.Specs[Input.Handle];
	Spec.IndexSetDependencies.push_back(IndexSet);
}

#undef REGISTRATION_BLOCK





////////////////////////////////////////////////
// All of the below are value accessors for use in EQUATION bodies (ONLY)!
////////////////////////////////////////////////


#define PARAMETER(ParH, ...) (RunState__->Running ? GetCurrentParameter(RunState__, ParH, ##__VA_ARGS__) : RegisterParameterDependency(RunState__, ParH, ##__VA_ARGS__))
#define INPUT(InputH) (RunState__->Running ? GetCurrentInput(RunState__, InputH) : RegisterInputDependency(RunState__, InputH))
#define RESULT(ResultH, ...) (RunState__->Running ? GetCurrentResult(RunState__, ResultH, ##__VA_ARGS__) : RegisterResultDependency(RunState__, ResultH, ##__VA_ARGS__))
#define LAST_RESULT(ResultH, ...) (RunState__->Running ? GetLastResult(RunState__, ResultH, ##__VA_ARGS__) : RegisterLastResultDependency(RunState__, ResultH, ##__VA_ARGS__))
#define EARLIER_RESULT(ResultH, StepBack, ...) (RunState__->Running ? GetEarlierResult(RunState__, ResultH, (StepBack), ##__VA_ARGS__) : RegisterLastResultDependency(RunState__, ResultH, ##__VA_ARGS__))
#define INPUT_WAS_PROVIDED(InputH) (RunState__->Running ? GetIfInputWasProvided(RunState__, InputH) : RegisterInputDependency(RunState__, InputH))
#define IF_INPUT_ELSE_PARAMETER(InputH, ParameterH) (RunState__->Running ? GetCurrentInputOrParameter(RunState__, InputH, ParameterH) : RegisterInputAndParameterDependency(RunState__, InputH, ParameterH))


#define CURRENT_DAY_OF_YEAR() (RunState__->DayOfYear)
#define DAYS_THIS_YEAR() (RunState__->DaysThisYear)
#define CURRENT_TIMESTEP() (RunState__->Timestep)

#define EQUATION(Model, ResultH, Def) \
SetEquation(Model, ResultH, \
 [=] (model_run_state *RunState__) { \
 Def \
 } \
);

#define EQUATION_OVERRIDE(Model, ResultH, Def) \
SetEquation(Model, ResultH, \
 [=] (model_run_state *RunState__) { \
 Def \
 } \
 , true \
);


//NOTE: These inline functions are used for type safety, which we don't get from macros.
//NOTE: We don't provide direct access to Time parameters since we want to encapsulate their storage. Instead we have accessor macros like CURRENT_DAY_OF_YEAR.
inline double
GetCurrentParameter(model_run_state *RunState, parameter_double_h Parameter)
{
	return RunState->CurParameters[Parameter.Handle].ValDouble;
}

inline u64
GetCurrentParameter(model_run_state *RunState, parameter_uint_h Parameter)
{
	return RunState->CurParameters[Parameter.Handle].ValUInt;
}

inline bool
GetCurrentParameter(model_run_state *RunState, parameter_bool_h Parameter)
{
	return RunState->CurParameters[Parameter.Handle].ValBool;
}


size_t OffsetForHandle(storage_structure &Structure, const index_t* CurrentIndexes, const index_t *IndexCounts, const index_t *OverrideIndexes, size_t OverrideCount, entity_handle Handle);



template<typename... T> double
GetCurrentParameter(model_run_state *RunState, parameter_double_h Parameter, T... Indexes)
{
	mobius_data_set *DataSet = RunState->DataSet;
	const size_t OverrideCount = sizeof...(Indexes);
	index_t OverrideIndexes[OverrideCount] = {Indexes...};
	size_t Offset = OffsetForHandle(DataSet->ParameterStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, OverrideIndexes, OverrideCount, Parameter.Handle);
	return DataSet->ParameterData[Offset].ValDouble;
}

template<typename... T> u64
GetCurrentParameter(model_run_state *RunState, parameter_uint_h Parameter, T... Indexes)
{
	mobius_data_set *DataSet = RunState->DataSet;
	const size_t OverrideCount = sizeof...(Indexes);
	index_t OverrideIndexes[OverrideCount] = {Indexes...};
	size_t Offset = OffsetForHandle(DataSet->ParameterStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, OverrideIndexes, OverrideCount, Parameter.Handle);
	return DataSet->ParameterData[Offset].ValUInt;
}

template<typename... T> bool
GetCurrentParameter(model_run_state *RunState, parameter_bool_h Parameter, T... Indexes)
{
	mobius_data_set *DataSet = RunState->DataSet;
	const size_t OverrideCount = sizeof...(Indexes);
	index_t OverrideIndexes[OverrideCount] = {Indexes...};
	size_t Offset = OffsetForHandle(DataSet->ParameterStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, OverrideIndexes, OverrideCount, Parameter.Handle);
	return DataSet->ParameterData[Offset].ValBool;
}


inline double
GetCurrentResult(model_run_state *RunState, equation_h Result)
{
	return RunState->CurResults[Result.Handle];
}

inline double 
GetLastResult(model_run_state *RunState, equation_h LastResult)
{
	return RunState->LastResults[LastResult.Handle];
}

template<typename... T> double
GetCurrentResult(model_run_state *RunState, equation_h Result, T... Indexes)
{
	mobius_data_set *DataSet = RunState->DataSet;
	const size_t OverrideCount = sizeof...(Indexes);
	index_t OverrideIndexes[OverrideCount] = {Indexes...};
	size_t Offset = OffsetForHandle(DataSet->ResultStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, OverrideIndexes, OverrideCount, Result.Handle);
	return RunState->AllCurResultsBase[Offset];
}

template<typename... T> double
GetLastResult(model_run_state *RunState, equation_h Result, T... Indexes)
{
	mobius_data_set *DataSet = RunState->DataSet;
	const size_t OverrideCount = sizeof...(Indexes);
	index_t OverrideIndexes[OverrideCount] = {Indexes...};

	size_t Offset = OffsetForHandle(DataSet->ResultStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, OverrideIndexes, OverrideCount, Result.Handle);
	return RunState->AllLastResultsBase[Offset];
}

template<typename... T> double
GetEarlierResult(model_run_state *RunState, equation_h Result, u64 StepBack, T...Indexes)
{
	mobius_data_set *DataSet = RunState->DataSet;
	const size_t OverrideCount = sizeof...(Indexes);
	index_t OverrideIndexes[OverrideCount] = {Indexes...};
	size_t Offset = OffsetForHandle(DataSet->ResultStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, OverrideIndexes, OverrideCount, Result.Handle);
	
	//TODO: Make proper accessor for this that belongs to mobius_data_set.cpp so that this file does not need to have knowledge of the inner workings of the storage system.
	double *Initial = DataSet->ResultData + Offset;
	//NOTE: Initial points to the initial value (adding TotalCount once gives us timestep 0)
	if(StepBack > RunState->Timestep)
	{
		return *Initial;
	}
	return *(Initial + ( (RunState->Timestep+1) - StepBack)*(RunState->DataSet->ResultStorageStructure.TotalCount));
}

inline double
GetCurrentInput(model_run_state *RunState, input_h Input)
{
	return RunState->CurInputs[Input.Handle];
}


inline bool
GetIfInputWasProvided(model_run_state * RunState, input_h Input)
{
	return RunState->CurInputWasProvided[Input.Handle];
}

inline double
GetCurrentInputOrParameter(model_run_state *RunState, input_h Input, parameter_double_h Parameter)
{
	if(GetIfInputWasProvided(RunState, Input)) return GetCurrentInput(RunState, Input);
	return GetCurrentParameter(RunState, Parameter);
}




template<typename... T> double
RegisterParameterDependency(model_run_state *RunState, parameter_double_h Parameter, T... Indexes)
{
	size_t OverrideCount = sizeof...(Indexes);
	RunState->ParameterDependencies.push_back({Parameter.Handle, OverrideCount});
	
	return 0.0;
}

template<typename... T> double
RegisterParameterDependency(model_run_state *RunState, parameter_uint_h Parameter, T... Indexes)
{
	size_t OverrideCount = sizeof...(Indexes);
	RunState->ParameterDependencies.push_back({Parameter.Handle, OverrideCount});
	
	return 0.0;
}

template<typename... T> double
RegisterParameterDependency(model_run_state *RunState, parameter_bool_h Parameter, T... Indexes)
{
	size_t OverrideCount = sizeof...(Indexes);
	RunState->ParameterDependencies.push_back({Parameter.Handle, OverrideCount});
	
	return 0.0;
}

inline double
RegisterInputDependency(model_run_state *RunState, input_h Input)
{
	RunState->InputDependencies.push_back({Input.Handle, 0});
	return 0.0;
}

inline double
RegisterInputAndParameterDependency(model_run_state *RunState, input_h Input, parameter_double_h Parameter)
{
	RegisterInputDependency(RunState, Input);
	RegisterParameterDependency(RunState, Parameter);

	return 0.0;
}

template<typename... T> double
RegisterResultDependency(model_run_state *RunState, equation_h Result, T... Indexes)
{
	std::vector<index_t> IndexVec = {Indexes...};
	RunState->ResultDependencies.push_back({Result.Handle, IndexVec});
	
	return 0.0;
}

template<typename... T> double
RegisterLastResultDependency(model_run_state *RunState, equation_h Result, T... Indexes)
{
	std::vector<index_t> IndexVec = {Indexes...};
	RunState->LastResultDependencies.push_back({Result.Handle, IndexVec});
	
	return 0.0;
}

//TODO: SET_RESULT is not that nice, and can interfere with how the dependency system works if used incorrectly. It is included to get PERSiST and some other models to work, but should be used with care!
#define SET_RESULT(ResultH, Value, ...) {if(RunState__->Running){SetResult(RunState__, Value, ResultH, ##__VA_ARGS__);}}

template<typename... T> void
SetResult(model_run_state *RunState, double Value, equation_h Result, T... Indexes)
{
	mobius_data_set *DataSet = RunState->DataSet;
	const size_t OverrideCount = sizeof...(Indexes);
	index_t OverrideIndexes[OverrideCount] = {Indexes...};
	size_t Offset = OffsetForHandle(DataSet->ResultStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, OverrideIndexes, OverrideCount, Result.Handle);
	RunState->AllCurResultsBase[Offset] = Value;
}

void
SetResult(model_run_state *RunState, double Value, equation_h Result)
{
	mobius_data_set *DataSet = RunState->DataSet;
	size_t Offset = OffsetForHandle(DataSet->ResultStorageStructure, RunState->CurrentIndexes, DataSet->IndexCounts, nullptr, 0, Result.Handle);
	RunState->AllCurResultsBase[Offset] = Value;
}


#define INDEX_COUNT(IndexSetH) (RunState__->Running ? (RunState__->DataSet->IndexCounts[IndexSetH.Handle]) : 1)
#define CURRENT_INDEX(IndexSetH) (RunState__->Running ? GetCurrentIndex(RunState__, IndexSetH) : RegisterIndexSetDependency(RunState__, IndexSetH))
#define FIRST_INDEX(IndexSetH) (index_t(IndexSetH, 0))
#define INDEX_NUMBER(IndexSetH, Index) (index_t(IndexSetH, (u32)Index))
#define INPUT_COUNT(IndexSetH) (RunState__->Running ? GetInputCount(RunState__, IndexSetH) : 0)

inline index_t
RegisterIndexSetDependency(model_run_state *RunState, index_set_h IndexSet)
{
	RunState->DirectIndexSetDependencies.push_back(IndexSet);
	return index_t(IndexSet, 0);
}

inline index_t
GetCurrentIndex(model_run_state *RunState, index_set_h IndexSet)
{
	return RunState->CurrentIndexes[IndexSet.Handle];
}

inline size_t
GetInputCount(model_run_state *RunState, index_set_h IndexSet)
{
	index_t Current = GetCurrentIndex(RunState, IndexSet);
	return RunState->DataSet->BranchInputs[IndexSet.Handle][Current].Count;
}


//TODO: The branch input iterator is way more complicated than it needs to be. Could be redesigned to just iterate over the BranchInputs[IndexSet.Handle][Branch] array. The only complicated part is that it can't do that in the registration run, and instead has to iterate over another object that has just one index.

inline size_t
BranchInputIteratorEnd(model_run_state *RunState, index_set_h IndexSet, index_t Branch)
{
	return RunState->Running ? RunState->DataSet->BranchInputs[IndexSet.Handle][Branch].Count : 1;
}

struct branch_input_iterator
{
	index_t *InputIndexes;
	size_t CurrentInputIndexIndex;
	
	void operator++() { CurrentInputIndexIndex++; }
	
	const index_t& operator*(){ return InputIndexes[CurrentInputIndexIndex]; }

	bool operator!=(const size_t& Idx) { return CurrentInputIndexIndex != Idx; }
};

inline branch_input_iterator
BranchInputIteratorBegin(model_run_state *RunState, index_set_h IndexSet, index_t Branch)
{
	static index_t DummyData;
	DummyData = index_t(IndexSet, 0);
	
	branch_input_iterator Iterator;
	Iterator.InputIndexes = RunState->Running ? RunState->DataSet->BranchInputs[IndexSet.Handle][Branch].Data : &DummyData;
	Iterator.CurrentInputIndexIndex = 0;
	return Iterator;
}

#define BRANCH_INPUT_BEGIN(IndexSetH) (BranchInputIteratorBegin(RunState__, IndexSetH, CURRENT_INDEX(IndexSetH)))
#define BRANCH_INPUT_END(IndexSetH) (BranchInputIteratorEnd(RunState__, IndexSetH, CURRENT_INDEX(IndexSetH)))

#define FOREACH_INPUT(IndexSetH, Body) \
for(auto Input = BRANCH_INPUT_BEGIN(IndexSetH); Input != BRANCH_INPUT_END(IndexSetH); ++Input) \
{ \
Body \
}

#define MOBIUS_MODEL_H
#endif
