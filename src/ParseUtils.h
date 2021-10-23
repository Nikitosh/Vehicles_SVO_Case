// All the parsing utils.

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
