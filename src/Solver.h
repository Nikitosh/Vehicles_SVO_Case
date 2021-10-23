// Solver interface that solves the original problem for `flightIds` subset of the flights 
// and applies it to `solution`.
class Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) = 0;
};

// Theoretical minimum solver.
class TheoreticalMinimumSolver: public Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		for (int flightId : flightIds)
			solution.score += config.sortedCosts[flightId][0].first;
	}
};

// Solver that handles all flights in the given order and picks the best stand for each of them.
class StupidSolver: public Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		for (int flightId : flightIds) {
			auto& flight = config.flights[flightId];
			for (int i = 0; i < (int) config.stands.size(); i++) {
				int standIndex = config.sortedCosts[flightId][i].second;
				if (config.canFit(flight, standIndex)) {
					solution.assign(config, flightId, standIndex);
					break;
				}
			}
		}
	}
};

// Solver that sorts all the flights in ascending order of timestamps and solves them greedily.
class GreedyTimestampSolver: public Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		vector<int> sortedFlightIds = flightIds;
		sort(sortedFlightIds.begin(), sortedFlightIds.end(), 
			[&config](int flightId1, int flightId2) {
				if (config.flights[flightId1].timestamp != config.flights[flightId2].timestamp)
					return config.flights[flightId1].timestamp < config.flights[flightId2].timestamp;
				return flightId1 < flightId2;
			});
		StupidSolver().solve(config, sortedFlightIds, solution);
	}
};

// Solver that sorts all the flights in ascending order of timestamps (taking into account 
// aircraft class) and solves them greedily.
class GreedyAircraftClassSolver: public Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		vector<int> sortedFlightIds = flightIds;
		sort(sortedFlightIds.begin(), sortedFlightIds.end(), 
			[&config](int flightId1, int flightId2) {
				if (config.flights[flightId1].isWide != config.flights[flightId2].isWide)
					return config.flights[flightId1].isWide > config.flights[flightId2].isWide;
				if (config.flights[flightId1].timestamp != config.flights[flightId2].timestamp)
					return config.flights[flightId1].timestamp < config.flights[flightId2].timestamp;
				return flightId1 < flightId2;
			});
		StupidSolver().solve(config, sortedFlightIds, solution);
	}
};

// Solver that sorts all the flights in ascending order of modeled timestamps (taking into account 
// aircraft class) and solves them greedily.
class GreedyAircraftClassAdjustedTimestampsSolver: public Solver {
public:
	GreedyAircraftClassAdjustedTimestampsSolver(int meanTaxiingTime): meanTaxiingTime_(meanTaxiingTime) {}

	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		vector<int> sortedFlightIds = flightIds;
		int meanTaxiingTime = meanTaxiingTime_;
		sort(sortedFlightIds.begin(), sortedFlightIds.end(), 
			[&config, meanTaxiingTime](int flightId1, int flightId2) {
				if (config.flights[flightId1].isWide != config.flights[flightId2].isWide)
					return config.flights[flightId1].isWide > config.flights[flightId2].isWide;
				int f1Timestamp = config.flights[flightId1].getAdjustedTimestamp(meanTaxiingTime);
				int f2Timestamp = config.flights[flightId2].getAdjustedTimestamp(meanTaxiingTime);
				if (f1Timestamp != f2Timestamp)
					return f1Timestamp < f2Timestamp;
				return flightId1 < flightId2;
			});
		StupidSolver().solve(config, sortedFlightIds, solution);
	}

private:
	int meanTaxiingTime_;
};

// Solver that sorts all the flights in ascending order of the cost of the best stand and solves them greedily.
class GreedyCostSolver: public Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		vector<int> sortedFlightIds = flightIds;
		sort(sortedFlightIds.begin(), sortedFlightIds.end(), 
			[&config](int flightId1, int flightId2) {
				int cost1 = config.sortedCosts[flightId1][0].first;
				int cost2 = config.sortedCosts[flightId2][0].first;
				if (cost1 != cost2)
					return cost1 > cost2;
				return flightId1 < flightId2;
			});
		StupidSolver().solve(config, sortedFlightIds, solution);
	}
};
