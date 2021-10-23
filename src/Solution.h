// Solution concept that stores score and stands mapping.
struct Solution {
	int score = 0;
	vector<int> stands;
	
	void write(const Configuration& config, const string& timetablePath, const string& solutionPath) {
		rapidcsv::Document doc(timetablePath);
		int rows = (int) doc.GetRowCount();
		for (int i = 0; i < rows; i++) 
			doc.SetCell<int>(doc.GetColumnIdx(AIRCRAFT_STAND_COLUMN), i, config.stands[stands[i]].id);
		doc.Save(solutionPath);
	}

	void assign(Configuration& config, int flightId, int standIndex) {
		score += config.costs[flightId][standIndex];
		stands[flightId] = standIndex;
		config.stands[standIndex].addSegment(config.handlingSegments[flightId][standIndex], config.flights[flightId].isWide);
	}

	static Solution readSolution(const string& solutionPath, Configuration& config) {
		rapidcsv::Document doc(solutionPath);
		int rows = (int) doc.GetRowCount();
		Solution solution;
		solution.stands.resize(rows);
		vector<Flight> flights;
		for (int flightId = 0; flightId < rows; flightId++) {
			int standId = doc.GetCell<int>(AIRCRAFT_STAND_COLUMN, flightId);
			for (int standIndex = 0; standIndex < (int) config.stands.size(); standIndex++) {
				if (config.stands[standIndex].id == standId) {
					solution.assign(config, flightId, standIndex);
				}
			}
		}
		return solution;	
	}
};