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
				.taxiingTime = doc.GetCell<int>(TAXIING_TIME_COLUMN, i),
				.occupiedSegments = {},
				.wideOccupiedSegments = {}
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
				.handlingTimes = {0, 0},
				.adjustedTimestamp = 0
			});
		}
		return flights;
	}
};
