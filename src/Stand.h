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
