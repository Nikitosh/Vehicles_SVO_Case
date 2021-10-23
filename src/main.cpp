#include <bits/stdc++.h>
#include <optional>

#include "lib/rapidcsv.h"

using namespace std;

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
const string TIMETABLE_PATH_PRIVATE = "../data/private/Timetable_Private.csv";
const string SOLUTION_PATH_PRIVATE = "../data/private/Solution_Private";

const string AIRCRAFT_CLASS_COLUMN = "Aircraft_Class";
const string MAX_SEATS_COLUMN = "Max_Seats";
const string JET_BRIDGE_HANDLING_TIME_COLUMN = "JetBridge_Handling_Time";
const string AWAY_HANDLING_TIME_COLUMN = "Away_Handling_Time";
const string VALUE_COLUMN = "Value";
const string AIRCRAFT_STAND_COLUMN = "Aircraft_Stand";
const string JET_BRIDGE_ON_ARRIVAL_COLUMN = "JetBridge_on_Arrival";
const string JET_BRIDGE_ON_DEPARTURE_COLUMN = "JetBridge_on_Departure";
const string T1_COLUMN = "1";
const string T2_COLUMN = "2";
const string T3_COLUMN = "3";
const string T4_COLUMN = "4";
const string T5_COLUMN = "5";
const string TERMINAL_COLUMN = "Terminal";
const string TAXIING_TIME_COLUMN = "Taxiing_Time";
const string AD_COLUMN = "flight_AD";
const string DATETIME_COLUMN = "flight_datetime";
const string AL_CODE_COLUMN = "flight_AL_Synchron_code";
const string FLIGHT_NUMBER_COLUMN = "flight_number";
const string FLIGHT_TYPE_COLUMN = "flight_ID";
const string FLIGHT_TERMINAL_COLUMN = "flight_terminal_#";
const string FLIGHT_AP_COLUMN = "flight_AP"; // DO WE NEED?
const string AC_COLUMN = "flight_AC_Synchron_code"; // DO WE NEED?
const string FLIGHT_CAPACITY_COLUMN = "flight_AC_PAX_capacity_total";
const string FLIGHT_PASSENGERS_COLUMN = "flight_PAX";

const string BUS_COST_PER_MINUTE_ROW = "Bus_Cost_per_Minute";
const string AWAY_AIRCRAFT_STAND_COST_PER_MINUTE_ROW = "Away_Aircraft_Stand_Cost_per_Minute";
const string JET_BRIDGE_AIRCRAFT_STAND_COST_PER_MINUTE_ROW = "JetBridge_Aircraft_Stand_Cost_per_Minute";
const string AIRCRAFT_TAXIING_COST_PER_MINUTE_ROW = "Aircraft_Taxiing_Cost_per_Minute";

const string WIDE_BODY_CLASS = "Wide_Body";
const int BUS_CAPACITY = 80;

bool parseDateFormat1(const string& date, int& year, int& month, int& day) {
	try {
		year = stoi(date.substr(0, 4));
		month = stoi(date.substr(5, 2));
		day = stoi(date.substr(8, 2));
	} catch (const std::invalid_argument& exception) {
		return false;
	}
	return true;
}

bool parseDateFormat2(const string& date, int& year, int& month, int& day) {
	try {
		day = stoi(date.substr(0, 2));
		month = stoi(date.substr(3, 2));
		year = stoi(date.substr(6, 4));
	} catch (const std::invalid_argument& exception) {
		return false;
	}
	return true;
}

int parseDate(const string& date) {
	int year = 0, month = 0, day = 0;
	if (!parseDateFormat1(date, year, month, day)) {
		if (!parseDateFormat2(date, year, month, day)) {
			cerr << "Incorrect date format: " << date << "\n";
		}
	}

	static const int MONTH_DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int days = 0;
	for (int i = 2001; i < year; i++) {
		if (i % 4 == 0)
			days += 366;
		else
			days += 365;
	}
	for (int i = 0; i < month - 1; i++) {
		days += MONTH_DAYS[i];
		if (year % 4 == 0 && i == 1)
			days++;
	}
	days += day - 1;
	return days;
}

int parseTime(const string& time) {
	int colonIndex = (int) time.find(':');
	int hours = stoi(time.substr(0, colonIndex));
	int minutes = stoi(time.substr(colonIndex + 1, 2));
	return hours * 60 + minutes;
}

int parseTimestamp(const string& timestamp) {
	int whitespaceIndex = (int) timestamp.find(' ');
	int days = parseDate(timestamp.substr(0, whitespaceIndex));
	int minutes = parseTime(timestamp.substr(whitespaceIndex + 1, timestamp.length() - (whitespaceIndex + 1)));
	return days * 24 * 60 + minutes;
}

struct Event {
	int start;
	int finish;
	int flightId;
	Event() {}
	Event(int start_, int finish_, int flightId_): start(start_), finish(finish_), flightId(flightId_) {}
	bool operator<(const Event& event) const {
		if (start != event.start)
			return start < event.start;
		return finish < event.finish;
	}
};

vector<Event> findOverlappingEvents(const set<Event>& segments, const Event& event) {
	auto it = segments.lower_bound(event);
	vector<Event> overlappingEvents;
	if (it != segments.begin()) {
		auto prevIt = it;
		--prevIt;
		if (event.start <= prevIt->finish)
			overlappingEvents.push_back(*prevIt);
	}
	while (it != segments.end()) {
		if (it->start > event.finish)
			break;
		overlappingEvents.push_back(*it);
		it++;
	}
	return overlappingEvents;
}

struct Configuration;
struct Solution; 

struct Stand {
	int id;
	// 'N' -- no jetbridge, 'I' -- for intenational flights, 'D' -- for domestic flights.
	char jetBridgeArrival;
	// 'N' -- no jetbridge, 'I' -- for intenational flights, 'D' -- for domestic flights.
	char jetBridgeDeparture;
	vector<int> busRideTimeToTerminal;
	int terminal;
	int taxiingTime;	

	set<Event> occupiedSegments;
	set<Event> wideOccupiedSegments;

	bool operator<(const Stand& stand) const {
		if (jetBridgeArrival != stand.jetBridgeArrival)
			return jetBridgeArrival < stand.jetBridgeArrival;
		if (jetBridgeDeparture != stand.jetBridgeDeparture)
			return jetBridgeDeparture < stand.jetBridgeDeparture;
		if (busRideTimeToTerminal != stand.busRideTimeToTerminal)
			return busRideTimeToTerminal < stand.busRideTimeToTerminal;
		if (terminal != stand.terminal)
			return terminal < stand.terminal;
		if (taxiingTime != stand.taxiingTime)
			return taxiingTime < stand.taxiingTime;
		return false;
	} 

	void clear() {
		occupiedSegments.clear();
		wideOccupiedSegments.clear();
	}

	bool canFit(const Event& segment, bool isWide) const {
		const auto& segments = isWide ? wideOccupiedSegments : occupiedSegments;
		auto it = segments.lower_bound(Event(segment.start, segment.start, 0));
		if (it != segments.end() && it->start <= segment.finish)
			return false;
		if (it != segments.begin()) {
			it--;
			if (it->finish >= segment.start)
				return false;
		}
		return true;
	}

	void addSegment(const Event& flightSegment, bool isWide) {
		occupiedSegments.insert(flightSegment);
		if (isWide)
			wideOccupiedSegments.insert(flightSegment);
	}

	void removeSegment(const Event& flightSegment) {
		occupiedSegments.erase(flightSegment);
		wideOccupiedSegments.erase(flightSegment);		
	}

	vector<int> eraseAllOverlappingEvents(const Event& flightSegment, bool isWide, const Configuration& config, Solution& solution);
};

struct Flight {
	int id;
	// 'A' -- arrival, 'D' -- departure.
	char adType; 
	// Timestamp in minutes starting from the minimum timestamp across all flight - C 
	// (to account handling and taxiing for it).  
	int timestamp;
	string airlines;
	int number;
	// 'I' -- international, 'D' -- domestic.
	char idType;
	int terminal;
	int capacity;
	int passengers;
	string aircraftClass;
	bool isWide;
	// Handling times: <jet bridge, away>.
	pair<int, int> handlingTimes;
	int adjustedTimestamp;

	inline Event getHandlingSegment(int taxiingTime, int handlingTime) const {
		if (adType == 'D')
			return Event(timestamp - taxiingTime - handlingTime, timestamp - taxiingTime, id);
		return Event(timestamp + taxiingTime, timestamp + taxiingTime + handlingTime, id);
	}

	inline int getAdjustedTimestamp(int taxiingTime) const {
		int handlingTime = (handlingTimes.first + handlingTimes.second) / 2;
		return getHandlingSegment(taxiingTime, handlingTime).start;
	}
};

struct Configuration {
	// Handling times: <jet bridge, away>.
	unordered_map<string, pair<int, int>> aircraftClassHandlingTime;
	vector<pair<int, string>> maxSeatsClasses;
	int busCost;
	int parkingCostAway;
	int parkingCostJetBridge;
	int taxiingCost;
	vector<Stand> stands;
	vector<Flight> flights;
	vector<vector<int>> costs;
	vector<vector<pair<int, int>>> sortedCosts;
	vector<vector<Event>> handlingSegments;
	vector<vector<int>> neighboringStands;
	
	void clear() {
		for (Stand& stand : stands)
			stand.clear();
	}

	string getAircraftClassByCapacity(int capacity) {
		return lower_bound(maxSeatsClasses.begin(), maxSeatsClasses.end(), pair<int, string>(capacity, ""))->second;
	}

	bool canFit(Flight& flight, int standIndex) {
		int flightId = flight.id;
		Event handlingSegment = handlingSegments[flightId][standIndex];
		if (!stands[standIndex].canFit(handlingSegment, /*isWide=*/false))
			return false;
		if (flight.isWide) {
			for (int neighboringStandId : neighboringStands[standIndex])
				if (!stands[neighboringStandId].canFit(handlingSegment, /*isWide=*/true))
					return false;
		}
		return true;
	}

	void adjustFlights() {
		for (auto& flight : flights) {
			flight.aircraftClass = getAircraftClassByCapacity(flight.capacity);
			flight.isWide = (flight.aircraftClass == WIDE_BODY_CLASS);
			flight.handlingTimes = aircraftClassHandlingTime[flight.aircraftClass];
		}
		int minTimestamp = min_element(flights.begin(), flights.end(), 
			[](const Flight &f1, const Flight &f2) { return f1.timestamp < f2.timestamp; })->timestamp;
		int maxTaxiingTime = max_element(stands.begin(), stands.end(), 
			[](const Stand &s1, const Stand &s2) { return s1.taxiingTime < s2.taxiingTime; })->taxiingTime;
		int maxHandlingTime = numeric_limits<int>::min();
		for (const auto& handlingTime : aircraftClassHandlingTime)
			maxHandlingTime = max(maxHandlingTime, max(handlingTime.second.first, handlingTime.second.second));
		
		for (auto& flight: flights)
			flight.timestamp += maxTaxiingTime + maxHandlingTime - minTimestamp;
	}

	int calculateCost(int flightIndex, int standIndex, Event& handlingSegment) {
		Flight& flight = flights[flightIndex];
		Stand& stand = stands[standIndex];
		int cost = stand.taxiingTime * taxiingCost;
		char jetBridge = (flight.adType == 'A' ? stand.jetBridgeArrival : stand.jetBridgeDeparture);
		int parkingCost = (jetBridge == 'N' ? parkingCostAway : parkingCostJetBridge); 
		int handlingTime = 0;
		if (jetBridge == flight.idType && flight.terminal == stand.terminal) {
			handlingTime = flight.handlingTimes.first;
		} else {
			handlingTime = flight.handlingTimes.second;
			cost += (flight.passengers + BUS_CAPACITY - 1) / BUS_CAPACITY * busCost 
				* stand.busRideTimeToTerminal[flight.terminal - 1];
		}
		cost += handlingTime * parkingCost;
		handlingSegment = flight.getHandlingSegment(stand.taxiingTime, handlingTime);
		return cost;
	}

	void calculateCosts() {
		costs.resize(flights.size(), vector<int>(stands.size()));
		sortedCosts.resize(flights.size(), vector<pair<int, int>>(stands.size()));
		handlingSegments.resize(flights.size(), vector<Event>(stands.size()));
		for (int i = 0; i < (int) flights.size(); i++) {
			for (int j = 0; j < (int) stands.size(); j++) {
				costs[i][j] = calculateCost(i, j, handlingSegments[i][j]);
				sortedCosts[i][j] = make_pair(costs[i][j], j);
			}
			sort(sortedCosts[i].begin(), sortedCosts[i].end());
		}
	}

	void calculateNeighboringStands() {
		neighboringStands.resize(stands.size());
		for (int i = 0; i < (int) stands.size(); i++) {
			if (stands[i].jetBridgeArrival == 'N')
				continue;
			for (int j = i - 1; j <= i + 1; j += 2)
				if (j >= 0 && j < (int) stands.size() && abs(stands[i].id - stands[j].id) == 1 
					&& stands[i].terminal == stands[j].terminal 
					&& stands[j].jetBridgeArrival != 'N') {
					neighboringStands[i].push_back(j);
				}
		}
	}

	static Configuration readConfiguration(const string& aircraftClassesPath,
		                                   const string& aircraftStandsPath,
		                                   const string& handlingRatesPath,
		                                   const string& handlingTimePath,
		                                   const string& timetablePath) {
		Configuration config;
		config.aircraftClassHandlingTime = readHandlingTime(handlingTimePath);
		config.maxSeatsClasses = readAircraftClasses(aircraftClassesPath);
		vector<int> handlingRates = readHandlingRates(handlingRatesPath);
		config.busCost = handlingRates[0];
		config.parkingCostAway = handlingRates[1];
		config.parkingCostJetBridge = handlingRates[2];
		config.taxiingCost = handlingRates[3];
		config.stands = readAircraftStands(aircraftStandsPath);
		config.flights = readTimetable(timetablePath);
		config.adjustFlights();
		config.calculateCosts();
		config.calculateNeighboringStands();
		return config;
	}

	static unordered_map<string, pair<int, int>> readHandlingTime(const string& handlingTimePath) {
		rapidcsv::Document doc(handlingTimePath);
		int rows = (int) doc.GetRowCount();
		unordered_map<string, pair<int, int>> aircraftClassHandlingTime;
		for (int i = 0; i < rows; i++) {
			string aircraftClass = doc.GetCell<string>(AIRCRAFT_CLASS_COLUMN, i);
			int handlingTimeJetBridge = doc.GetCell<int>(JET_BRIDGE_HANDLING_TIME_COLUMN, i);
			int handlingTimeAway = doc.GetCell<int>(AWAY_HANDLING_TIME_COLUMN, i);
			aircraftClassHandlingTime[aircraftClass] = {handlingTimeJetBridge, handlingTimeAway};
		}	
		return aircraftClassHandlingTime;
	}

	static vector<pair<int, string>> readAircraftClasses(const string& aircraftClassesPath) {
		rapidcsv::Document doc(aircraftClassesPath);
		int rows = (int) doc.GetRowCount();
		vector<pair<int, string>> maxSeatsClasses;
		for (int i = 0; i < rows; i++) {
			string aircraftClass = doc.GetCell<string>(AIRCRAFT_CLASS_COLUMN, i);
			int maxSeats = doc.GetCell<int>(MAX_SEATS_COLUMN, i);
			maxSeatsClasses.push_back({maxSeats, aircraftClass});
		}	
		sort(maxSeatsClasses.begin(), maxSeatsClasses.end());
		return maxSeatsClasses;
	}

	static vector<int> readHandlingRates(const string& handlingRatesPath) {
		rapidcsv::Document doc(handlingRatesPath, rapidcsv::LabelParams(0, 0));
		int busCost = doc.GetCell<int>(VALUE_COLUMN, BUS_COST_PER_MINUTE_ROW);
		int parkingCostAway = doc.GetCell<int>(VALUE_COLUMN, AWAY_AIRCRAFT_STAND_COST_PER_MINUTE_ROW);
		int parkingCostJetBridge = doc.GetCell<int>(VALUE_COLUMN, JET_BRIDGE_AIRCRAFT_STAND_COST_PER_MINUTE_ROW);
		int taxiingCost = doc.GetCell<int>(VALUE_COLUMN, AIRCRAFT_TAXIING_COST_PER_MINUTE_ROW);
		return vector<int>({busCost, parkingCostAway, parkingCostJetBridge, taxiingCost});
	}

	static vector<Stand> readAircraftStands(const string& aircraftStandsPath) {
		rapidcsv::Document doc(aircraftStandsPath);
		int rows = (int) doc.GetRowCount();
		vector<Stand> stands;
		for (int i = 0; i < rows; i++) {
			stands.push_back(Stand{
				.id = doc.GetCell<int>(AIRCRAFT_STAND_COLUMN, i),
				.jetBridgeArrival = doc.GetCell<char>(JET_BRIDGE_ON_ARRIVAL_COLUMN, i),
				.jetBridgeDeparture = doc.GetCell<char>(JET_BRIDGE_ON_DEPARTURE_COLUMN, i),
				.busRideTimeToTerminal = {
					doc.GetCell<int>(T1_COLUMN, i),
					doc.GetCell<int>(T2_COLUMN, i),
					doc.GetCell<int>(T3_COLUMN, i),
					doc.GetCell<int>(T4_COLUMN, i),
					doc.GetCell<int>(T5_COLUMN, i)			
				},
				.terminal = doc.GetCell<int>(TERMINAL_COLUMN, i, 
					[](const string& t, int& val) { val = t.empty() ? 0 : stoi(t); }),
				.taxiingTime = doc.GetCell<int>(TAXIING_TIME_COLUMN, i)	
			});
		}
		return stands;		
	}

	static vector<Flight> readTimetable(const string& timetablePath) {
		rapidcsv::Document doc(timetablePath);
		int rows = (int) doc.GetRowCount();
		vector<Flight> flights;
		for (int i = 0; i < rows; i++) {
			flights.push_back(Flight{
				.id = i,
				.adType = doc.GetCell<char>(AD_COLUMN, i),
				.timestamp = doc.GetCell<int>(DATETIME_COLUMN, i, 
					[](const string& t, int& val) {
						val = parseTimestamp(t);
					}),
				.airlines = doc.GetCell<string>(AL_CODE_COLUMN, i),
				.number = doc.GetCell<int>(FLIGHT_NUMBER_COLUMN, i),
				.idType = doc.GetCell<char>(FLIGHT_TYPE_COLUMN, i),
				.terminal = doc.GetCell<int>(FLIGHT_TERMINAL_COLUMN, i),
				.capacity = doc.GetCell<int>(FLIGHT_CAPACITY_COLUMN, i),
				.passengers = doc.GetCell<int>(FLIGHT_PASSENGERS_COLUMN, i),
				.aircraftClass = "",
				.isWide = false,
				.handlingTimes = {0, 0}
			});
		}
		return flights;
	}
};

struct Solution {
	int score = 0;
	vector<int> stands;
	
	void write(const Configuration& config, const string& timetablePath, const string& solutionPath) {
		rapidcsv::Document doc(timetablePath);
		int rows = (int) doc.GetRowCount();
		for (int i = 0; i < rows; i++) 
			doc.SetCell<int>(doc.GetColumnIdx(AIRCRAFT_STAND_COLUMN), i, config.stands[stands[i]].id);
		doc.Save(solutionPath + "_" + to_string(score) + ".csv");
	}

	void assign(Configuration& config, int flightId, int standIndex) {
		score += config.costs[flightId][standIndex];
		stands[flightId] = standIndex;
		config.stands[standIndex].addSegment(config.handlingSegments[flightId][standIndex], config.flights[flightId].isWide);
	}
};

vector<int> Stand::eraseAllOverlappingEvents(const Event& flightSegment, bool isWide, const Configuration& config, Solution& solution) {
	vector<Event> overlappingEvents = isWide ? findOverlappingEvents(wideOccupiedSegments, flightSegment) 
		: findOverlappingEvents(occupiedSegments, flightSegment);
	vector<int> flightIds;
	for (const Event& event : overlappingEvents) {
		occupiedSegments.erase(event);
		wideOccupiedSegments.erase(event);
		int standIndex = solution.stands[event.flightId];
		solution.score -= config.costs[event.flightId][standIndex];
		solution.stands[event.flightId] = -1;
		flightIds.push_back(event.flightId);
	} 	
	return flightIds;
}

class Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) = 0;
};

class TheoreticalMinimumSolver: public Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		for (int flightId : flightIds)
			solution.score += config.sortedCosts[flightId][0].first;
	}
};


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

class GreedyAircraftClassSolver: public Solver {
public:
	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		vector<int> sortedFlightIds = flightIds;
		sort(sortedFlightIds.begin(), sortedFlightIds.end(), 
			[&config](int flightId1, int flightId2) {
				if (config.flights[flightId1].isWide != config.flights[flightId2].isWide)
					return config.flights[flightId1].isWide < config.flights[flightId2].isWide;
				if (config.flights[flightId1].timestamp != config.flights[flightId2].timestamp)
					return config.flights[flightId1].timestamp < config.flights[flightId2].timestamp;
				return flightId1 < flightId2;
			});
		StupidSolver().solve(config, sortedFlightIds, solution);
	}
};

class GreedyAircraftClassAdjustedTimestampsSolver: public Solver {
public:
	GreedyAircraftClassAdjustedTimestampsSolver(int meanTaxiingTime): meanTaxiingTime_(meanTaxiingTime) {}

	virtual void solve(Configuration& config, const vector<int>& flightIds, Solution& solution) override {
		vector<int> sortedFlightIds = flightIds;
		int meanTaxiingTime = meanTaxiingTime_;
		sort(sortedFlightIds.begin(), sortedFlightIds.end(), 
			[&config, meanTaxiingTime](int flightId1, int flightId2) {
				if (config.flights[flightId1].isWide != config.flights[flightId2].isWide)
					return config.flights[flightId1].isWide < config.flights[flightId2].isWide;
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

class SolutionOptimizer {
public:
	virtual void optimize(Configuration& config, Solution& solution) = 0;
};

class RandomSolutionOptimizer : public SolutionOptimizer {
public:
	RandomSolutionOptimizer(Solver* solver, int iterations): solver_(solver), iterations_(iterations) {}

	void optimize(Configuration& config, Solution& solution) override {
		for (int iteration = 0; iteration < iterations_; iteration++) {
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
			
			cout << "Iteration: " << iteration << ", score: " << solution.score << "\n";
			if (solution.score > optimizedSolution.score) {
				solution = optimizedSolution;
			} else {
				config.clear();
				for (int i = 0; i < (int) config.flights.size(); i++) {
					int standIndex = solution.stands[i];
					config.stands[standIndex].addSegment(config.handlingSegments[i][standIndex], config.flights[i].isWide);	
				}
			}
		}
	}

private:
	mt19937 rng;
	Solver* solver_;
	int iterations_;
};

int main() {
	/*
	Configuration config = Configuration::readConfiguration(
		AIRCRAFT_CLASSES_PATH_PUBLIC,
		AIRCRAFT_STANDS_PATH_PUBLIC, 
		HANDLING_RATES_PATH_PUBLIC, 
		HANDLING_TIME_PATH_PUBLIC, 
		TIMETABLE_PATH_PUBLIC);
	*/
	Configuration config = Configuration::readConfiguration(
		AIRCRAFT_CLASSES_PATH_PRIVATE,
		AIRCRAFT_STANDS_PATH_PRIVATE, 
		HANDLING_RATES_PATH_PRIVATE, 
		HANDLING_TIME_PATH_PRIVATE, 
		TIMETABLE_PATH_PRIVATE);
	
	/*
	TheoreticalMinimumSolver theoreticalMinimumSolver;
	Solution solution = theoreticalMinimumSolver.solve(config);
	cout << "Theoretical minimum score: " << solution.score << "\n";
	*/

	vector<int> flightIds;
	for (const auto& flight : config.flights)
		flightIds.push_back(flight.id);
	Solution solution;
	solution.stands.resize(flightIds.size());
	GreedyAircraftClassAdjustedTimestampsSolver greedyAircraftClassAdjustedTimestampsSolver(13);
	greedyAircraftClassAdjustedTimestampsSolver.solve(config, flightIds, solution);
	cout << "Greedy aircraft class solution score: " << solution.score << "\n";
	solution.write(config, TIMETABLE_PATH_PRIVATE, SOLUTION_PATH_PRIVATE);

	/*
	GreedyCostSolver greedyCostSolver;
	solution = greedyCostSolver.solve(config);
	cout << "Greedy cost solution score: " << solution.score << "\n";
	solution.write(config, TIMETABLE_PATH_PRIVATE, SOLUTION_PATH_PRIVATE);
	*/

	StupidSolver stupidSolver;
	RandomSolutionOptimizer optimizer(&stupidSolver, 100000);
	optimizer.optimize(config, solution);
	cout << "Optimized solution score: " << solution.score << "\n";
	solution.write(config, TIMETABLE_PATH_PRIVATE, SOLUTION_PATH_PRIVATE);
}