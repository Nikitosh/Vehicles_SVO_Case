// Corresponds to Aircraft Handling event.
struct Event {
	// Start time, in minutes.
	int start;
	// Finish time, in minutes.
	int finish;
	// Flight ID of the aircraft.
	int flightId;
	Event() {}
	Event(int start_, int finish_, int flightId_): start(start_), finish(finish_), flightId(flightId_) {}
	bool operator<(const Event& event) const {
		if (start != event.start)
			return start < event.start;
		return finish < event.finish;
	}
};

// Returns list of all events that overlap with given event.
// Is used in optimization step of the algorithm to remove conflicting aircrafts.
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