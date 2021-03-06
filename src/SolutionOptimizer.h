// Optimizer interface that solves the original problem.
// Accepts the solution and run time limit and tries to optimize it.
class SolutionOptimizer {
public:
	virtual void optimize(Configuration& config, Solution& solution) = 0;
};

class RandomSolutionOptimizer : public SolutionOptimizer {
public:
	RandomSolutionOptimizer(double finishTimeLimit, Solver* solver): 
		finishTimeLimit_(finishTimeLimit), solver_(solver) {}

	// Tries two optimization steps:
	// * Move given flight to more profitable stand by 
	//   removing all conflicting aircrafts and readding them in random order afterwards.
	// * Clear up several stands and readd them in random order afterwards.
	void optimize(Configuration& config, Solution& solution) override {
        double annealing_temperature = 10000.;
        int annealing_steps = 10000;
		for (int iteration = 0;; iteration++) {
			if (iteration % 100 == 0 && double(clock()) / CLOCKS_PER_SEC > finishTimeLimit_)
				break;

            if ((iteration + 1) % annealing_steps == 0) {
                annealing_temperature /= 2.;
            }
			Solution optimizedSolution = solution;
			
			for (int g = 0; g < 1; g++) {
				int flightId = rng() % (int) config.flights.size();
				int standIndex = solution.stands[flightId];
				int standSortedPosition = 0;
				for (int j = 0; j < (int) config.stands.size(); j++) {
					if (config.sortedCosts[flightId][j].second == standIndex) {
						standSortedPosition = j;
						break;
					}
				}
				if (standSortedPosition == 0)
					continue;
				config.stands[standIndex].removeSegment(config.handlingSegments[flightId][standIndex]);
				optimizedSolution.score -= config.costs[flightId][standIndex];
				optimizedSolution.stands[flightId] = -1;

				int newStandSortedPosition = rng() % standSortedPosition;
				if (newStandSortedPosition == standSortedPosition)
					newStandSortedPosition--;
				int newStandIndex = config.sortedCosts[flightId][newStandSortedPosition].second;
				Event handlingSegment = config.handlingSegments[flightId][newStandIndex];
				vector<int> removedFlightIds = config.stands[newStandIndex].eraseAllOverlappingEvents(handlingSegment, /*isWide=*/false, config, optimizedSolution);
				if (config.flights[flightId].isWide) {
					for (int neighboringStandIndex : config.neighboringStands[newStandIndex]) {
						vector<int> removedNeighboringFlightIds = 
							config.stands[neighboringStandIndex].eraseAllOverlappingEvents(handlingSegment, /*isWide=*/true, config, optimizedSolution);
						removedFlightIds.insert(removedFlightIds.end(), removedNeighboringFlightIds.begin(), removedNeighboringFlightIds.end());
					}
				}

				optimizedSolution.assign(config, flightId, newStandIndex);
			
				shuffle(removedFlightIds.begin(), removedFlightIds.end(), rng);
				solver_->solve(config, removedFlightIds, optimizedSolution);
			}

			if (solution.score > optimizedSolution.score) {
				solution = optimizedSolution;
			} else {
                double prob = exp((double)(solution.score - optimizedSolution.score) / (double)annealing_temperature);
                if ((double)(rng() % INT_MAX) / (double)(INT_MAX) >= prob) {
                    config.clear();
                    for (int i = 0; i < (int) config.flights.size(); i++) {
                        int standIndex = solution.stands[i];
                        config.stands[standIndex].addSegment(config.handlingSegments[i][standIndex],
                                                             config.flights[i].isWide);
                    }
                } else {
                    solution = optimizedSolution;
                }
			}
		}
	}

private:
	double finishTimeLimit_;
	mt19937 rng;
	Solver* solver_;
	int iterations_;
};
