#pragma once

#include <exception>
#include <vector>
#include <string>
#include <format>
#include <queue>
#include <optional>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <libsumo/libsumo.h>
#include "tinyxml2.h"
#include <iostream>
using namespace std;

void AddVehToSUMO(const string& name, const string& from_edge, const string& to_edge);

class V2SimError : public exception {
private:
	string message;
public:
	V2SimError(const string &msg) : message(msg)
	{
		std::cerr << msg << endl;
	}
	V2SimError(const string &&msg) : message(msg)
	{
		std::cerr << msg << endl;
	}
	const char* what() const throw ()
	{
		return message.c_str();
	}
};

class RangeList {
private:
	vector<pair<int, int>> d;
	int loop_period; // Period for the loop, in second. 0 for no loop. Should not be negative.
	int loop_times; // Times for the loop. Should be positive or -1 (infinite).
	bool forced = false;
	bool forced_value = false;
	void check();
public:
	RangeList(bool always = false) : loop_period(0), loop_times(0) { SetForce(always); }
	RangeList(tinyxml2::XMLElement* xml, bool allow_null = false, bool null_value = false);
	RangeList(const vector<pair<int, int>>& d, int period = 0, int times = 1);
	RangeList(const std::initializer_list<pair<int,int>> d, int period = 0, int times = 1);
	bool Contains(int time) const;
	void SetForce(bool b) { forced = true; forced_value = b; }
	void ClearForce() { forced = false; }
	size_t size() const { return d.size(); }
};