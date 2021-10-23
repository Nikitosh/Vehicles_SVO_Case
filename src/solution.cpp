#include <climits>
#include <iostream>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "lib/rapidcsv.h"

using namespace std;

#include "ParseUtils.h"
#include "Event.h"
#include "Stand.h"
#include "Flight.h"
#include "Configuration.h"
#include "Solution.h"
#include "StandImpl.h"
#include "Solver.h"
#include "SolutionOptimizer.h"

const string AIRCRAFT_CLASSES_PATH_PUBLIC = "../data/public/AirCraftClasses_Public.csv";
const string AIRCRAFT_STANDS_PATH_PUBLIC = "../data/public/AirCraft_Stands_Public.csv";
const string HANDLING_RATES_PATH_PUBLIC = "../data/public/Handling_Rates_Public.csv";
const string HANDLING_TIME_PATH_PUBLIC = "../data/public/Handling_Time_Public.csv";
const string TIMETABLE_PATH_PUBLIC = "../data/public/Timetable_Public.csv";
const string SOLUTION_PATH_PUBLIC = "../data/public/Solution_Public.csv";

const string AIRCRAFT_CLASSES_PATH_PRIVATE = "../data/private/Aircraft_Classes_Private.csv";
const string AIRCRAFT_STANDS_PATH_PRIVATE = "../data/private/Aircraft_Stands_Private.csv";
const string HANDLING_RATES_PATH_PRIVATE = "../data/private/Handling_Rates_Private.csv";
const string HANDLING_TIME_PATH_PRIVATE = "../data/private/Handling_Time_Private.csv";
const string TIMETABLE_PATH_PRIVATE = "../data/private/Timetable_private.csv";
const string SOLUTION_PATH_PRIVATE = "../data/private/Solution_Private.csv";

const int MEAN_TAXIING_TIME = 13;

int main(int argc, char** argv) {
	if (argc < 3) {
		cerr << "ERROR: You should specify run time (in seconds) and run mode: private or custom.\n";
		return 0;
	}
	double startTime = double(clock()) / CLOCKS_PER_SEC;
	int runtime = atoi(argv[1]);
	string runMode = argv[2];

	string aircraftClassPath;
	string aircraftStandsPath;
	string handlingRatesPath;
	string handlingTimePath;
	string timetablePath;
	string outputSolutionPath;
	int inputSolutionPathIndex = -1;
	if (runMode == "private") {
		aircraftClassPath = AIRCRAFT_CLASSES_PATH_PRIVATE;
		aircraftStandsPath = AIRCRAFT_STANDS_PATH_PRIVATE;
		handlingRatesPath = HANDLING_RATES_PATH_PRIVATE;
		handlingTimePath = HANDLING_TIME_PATH_PRIVATE;
		timetablePath = TIMETABLE_PATH_PRIVATE;	
		outputSolutionPath = SOLUTION_PATH_PRIVATE;
		inputSolutionPathIndex = 3;
	} else {
		if (argc < 9) {
			cerr << "ERROR: For custom mode you should specify 6 paths of corresponding tables: " << 
				"aircraft_classes, aircraft_stands, handling_rates, handling_time, timetable, solution.\n";
			return 0;
		}
		aircraftClassPath = argv[3];
		aircraftStandsPath = argv[4];
		handlingRatesPath = argv[5];
		handlingTimePath = argv[6];
		timetablePath = argv[7];
		outputSolutionPath = argv[8];
		inputSolutionPathIndex = 9;
	}

	Configuration config = Configuration::readConfiguration(
		aircraftClassPath,
		aircraftStandsPath, 
		handlingRatesPath, 
		handlingTimePath, 
		timetablePath
	);

	Solution solution;
	if (argc > inputSolutionPathIndex) {
		string solutionPath = argv[inputSolutionPathIndex];
		solution = Solution::readSolution(solutionPath, config);		
	} else {
		vector<int> flightIds;
		for (const auto& flight : config.flights)
			flightIds.push_back(flight.id);
		solution.stands.resize(flightIds.size());
		GreedyAircraftClassAdjustedTimestampsSolver greedyAircraftClassAdjustedTimestampsSolver(MEAN_TAXIING_TIME);
		greedyAircraftClassAdjustedTimestampsSolver.solve(config, flightIds, solution);
	}

	StupidSolver stupidSolver;
	RandomSolutionOptimizer optimizer(startTime + runtime, &stupidSolver);
	optimizer.optimize(config, solution);
	solution.write(config, timetablePath, outputSolutionPath);
}