// Corresponds to the Flight concept.
struct Flight {
	int id;
	// 'A' -- arrival, 'D' -- departure.
	char adType; 
	// Timestamp in minutes starting from the minimum timestamp across all flight - C 
	// (to account handling and taxiing for it).  
	int timestamp;
	// Name of the airlines.
	string airlines;
	int number;
	// 'I' -- international, 'D' -- domestic.
	char idType;
	int terminal;
	int capacity;
	int passengers;
	// One of three predefined classes.
	string aircraftClass;
	// If the aircraft has "Wide_Body" class.
	bool isWide;
	// Handling times: <jet bridge, away>.
	pair<int, int> handlingTimes;
	// Modeled timestamp of handling event start.
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
