#include <bits/stdc++.h>

#include "lib/rapidcsv.h"

using namespace std;

const string AIRCRAFT_CLASSES_PATH_PUBLIC = "../data/public/AirCraftClasses_Public.csv";
const string AIRCRAFT_STANDS_PATH_PUBLIC = "../data/public/AirCraft_Stands_Public.csv";
const string HANDLING_RATES_PATH_PUBLIC = "../data/public/Handling_Rates_Public.csv";
const string HANDLING_TIME_PATH_PUBLIC = "../data/public/Handling_Time_Public.csv";
const string TIMETABLE_PATH_PUBLIC = "../data/public/Timetable_Public.csv";
const string SOLUTION_PATH_PUBLIC = "../data/public/Solution_Public.csv";

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

int parseTimestamp(const string& timestamp) {
	static const int MONTH_DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int year = stoi(timestamp.substr(0, 4));
	int month = stoi(timestamp.substr(5, 2));
	int day = stoi(timestamp.substr(8, 2));
	int hour = stoi(timestamp.substr(11, 2));
	int minute = stoi(timestamp.substr(14, 2));
	
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
	return days * 24 * 60 + hour * 60 + minute;
}

struct Stand {
	int id;
	// 'N' -- no jetbridge, 'I' -- for intenational flights, 'D' -- for domestic flights.
	char jetBridgeArrival;
	// 'N' -- no jetbridge, 'I' -- for intenational flights, 'D' -- for domestic flights.
	char jetBridgeDeparture;
	vector<int> busRideTimeToTerminal;
	int terminal;
	int taxiingTime;	

	set<pair<int, int>> segments;

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

	bool canFit(const pair<int, int>& segment) const {
		auto it = segments.lower_bound(make_pair(segment.first, segment.first));
		if (it != segments.end() && it->first <= segment.second)
			return false;
		if (it != segments.begin()) {
			it--;
			if (it->second >= segment.first)
				return false;
		}
		return true;
	}

	void addSegment(const pair<int, int>& segment) {
		segments.insert(segment);
	}
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

	inline pair<int, int> getHandlingSegment(int taxiingTime, int handlingTime) const {
		if (adType == 'A')
			return {timestamp - taxiingTime - handlingTime, timestamp - taxiingTime};
		return {timestamp + taxiingTime, timestamp + taxiingTime + handlingTime};
	}
};

struct Solution {
	int score = 0;
	vector<int> stands;
	
	void write(const string& timetablePath, const string& solutionPath) {
		rapidcsv::Document doc(timetablePath);
		int rows = (int) doc.GetRowCount();
		for (int i = 0; i < rows; i++) 
			doc.SetCell<int>(doc.GetColumnIdx(AIRCRAFT_STAND_COLUMN), i, stands[i]);
		doc.Save(solutionPath);
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

	string getAircraftClassByCapacity(int capacity) {
		return lower_bound(maxSeatsClasses.begin(), maxSeatsClasses.end(), pair<int, string>(capacity, ""))->second;
	}

	int getCost(const Flight& flight, Stand& stand) {
		int cost = stand.taxiingTime * taxiingCost;
		char jetBridge = (flight.adType == 'A' ? stand.jetBridgeArrival : stand.jetBridgeDeparture);
		int parkingCost = (jetBridge == 'N' ? parkingCostAway : parkingCostJetBridge); 
		int handlingTime = 0;
		if (jetBridge == flight.idType && flight.terminal == stand.terminal) {
			handlingTime = flight.handlingTimes.first;
		} else {
			handlingTime = flight.handlingTimes.second;
			cost += flight.passengers / BUS_CAPACITY * busCost * stand.busRideTimeToTerminal[flight.terminal - 1];
		}
		cost += handlingTime * parkingCost;
		pair<int, int> handlingSegment = flight.getHandlingSegment(stand.taxiingTime, handlingTime);
		if (!stand.canFit(handlingSegment))
			return -1;
		stand.addSegment(handlingSegment);
		return cost;
	}

	void adjustFlights() {
		for (auto& flight : flights) {
			flight.aircraftClass = getAircraftClassByCapacity(flight.capacity), 
			flight.isWide = (flight.aircraftClass == WIDE_BODY_CLASS),
			flight.handlingTimes = aircraftClassHandlingTime[flight.aircraftClass];
			
		}
		int minTimestamp = min_element(flights.begin(), flights.end(), 
			[](const Flight &f1, const Flight &f2) { return f1.timestamp < f2.timestamp; })->timestamp;
		int maxTaxiingTime = max_element(stands.begin(), stands.end(), 
			[](const Stand &s1, const Stand &s2) { return s1.taxiingTime < s2.taxiingTime; })->taxiingTime;
		int maxHandlingTime = numeric_limits<int>::min();
		for (const auto& handlingTimes : aircraftClassHandlingTime)
			maxHandlingTime = max(maxHandlingTime, max(handlingTimes.second.first, handlingTimes.second.second));
		
		for (auto& flight: flights)
			flight.timestamp += maxTaxiingTime + maxHandlingTime - minTimestamp;
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

class Solver {
public:
	virtual Solution solve(Configuration& config) = 0;
};

class RandomSolver: public Solver {
public:
	virtual Solution solve(Configuration& config) override {
		Solution solution;
		for (const auto& flight : config.flights) {
			for (auto& stand: config.stands) {
				int cost = config.getCost(flight, stand);
				if (cost != -1) {
					solution.stands.push_back(stand.id);
					solution.score += cost;
					break;
				}
			}
		}
		return solution;
	}
};

int main() {
	Configuration config = Configuration::readConfiguration(
		AIRCRAFT_CLASSES_PATH_PUBLIC,
		AIRCRAFT_STANDS_PATH_PUBLIC, 
		HANDLING_RATES_PATH_PUBLIC, 
		HANDLING_TIME_PATH_PUBLIC, 
		TIMETABLE_PATH_PUBLIC);
	RandomSolver random_solver;
	Solution solution = random_solver.solve(config);
	solution.write(TIMETABLE_PATH_PUBLIC, SOLUTION_PATH_PUBLIC);
	//cout << solution.score << "\n";
}