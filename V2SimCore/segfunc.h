#pragma once

#include "utilbase.h"
using namespace std;

class SegFunc {
private:
	vector<int> tl;
	vector<double> d;
	int loop_period; // Period for the loop, in second. 0 for no loop. Should not be negative.
	int loop_times; // Times for the loop. Should be positive or -1 (infinite).
	bool overrided = false;
	double overrided_val = 0.0;
	void check();
public:
	size_t size() const { return tl.size(); }
	SegFunc() : loop_period(0), loop_times(1) { }
	SegFunc(tinyxml2::XMLElement* e, bool allow_null = true, 
		const char* tag = "item", const char* time_attr = "time", const char* val_attr = "value");
	SegFunc(vector<int>&& timelist, vector<double>&& data, int period = 0, int times = 1) :
		tl(timelist), d(data), loop_period(period), loop_times(times) {
		check();
	}
	SegFunc(const vector<int>& timelist, const vector<double>& data, int period = 0, int times = 1) :
		tl(timelist), d(data), loop_period(period), loop_times(times) {
		check();
	}
	SegFunc(std::initializer_list<pair<int, double>> d, int period = 0, int times = 1) :
		loop_period(period), loop_times(times) {
		for (auto& e : d) {
			this->tl.emplace_back(e.first);
			this->d.emplace_back(e.second);
		}
		check();
	}
	int GetPeriod() const { return loop_period; }
	int GetRepeatTimes() const { return loop_times; }
	double Get(int time) const;
	void Add(int time, double data) {
		if (time <= tl.back()) {
			throw V2SimError(std::format("New time ({}) must be greater than the last time ({}) in the SegFunc.", time, tl.back()));
		}
		tl.push_back(time);
		d.push_back(data);
	}
	double operator()(int time) const {
		return overrided ? overrided_val : Get(time);
	}
	SegFunc operator*(double db) const {
		SegFunc ret(tl, d, loop_period, loop_times);
		for (auto& d0 : ret.d) {
			d0 *= db;
		}
		return ret;
	}
	void SetOverride(double val) {
		overrided_val = val;
		overrided = true;
	}
	void ClearOverride() {
		overrided = false;
	}
	double GetOverride() const {
		return overrided_val;
	}
	const SegFunc operator-() const {
		SegFunc ret(tl, d, loop_period, loop_times);
		for (auto i = ret.d.begin(); i != ret.d.end(); ++i) {
			*i = -*i;
		}
		return ret;
	}
	SegFunc Expand() const {
		if (loop_times == -1) {
			throw V2SimError("Cannot expand an infinite SegFunc.");
		}
		SegFunc ret(tl, d, 0, 1);
		size_t n = tl.size();
		for (int i = 1; i < loop_times; ++i) {
			for (int j = 0; j < n; ++j) {
				ret.Add(tl[j] + loop_period * i, d[j]);
			}
		}
		return ret;
	}
	void SelfExpand() {
		if (loop_times == -1) {
			throw V2SimError("Cannot expand an infinite SegFunc.");
		}
		size_t n = tl.size();
		for (int i = 1; i < loop_times; ++i) {
			for (int j = 0; j < n; ++j) {
				Add(tl[j] + loop_period * i, d[j]);
			}
		}
		loop_times = 1;
		loop_period = 0;
	}
	int TimeLine(size_t i) const {
		return tl.at(i);
	}
	const double Data(size_t i) const {
		return d.at(i);
	}
};

SegFunc QuickSum(const vector<SegFunc>& funcs);