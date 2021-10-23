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