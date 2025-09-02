#include "utils.h"

void RangeList::check() {
	if (loop_period < 0) {
		throw V2SimError(std::format("Invalid loop period: {}", loop_period));
	}
	if (loop_times < -1 || loop_times == 0) {
		throw V2SimError(std::format("Invalid loop times: {}", loop_times));
	}
	if (loop_period > 0 && d.back().second > loop_period) {
		throw V2SimError(std::format(
			"Time range ({}) exceeds loop period ({}).", d.back().second, loop_period));
	}
	for (auto& e : d) {
		if (e.first < 0 || e.second < 0) {
			throw V2SimError(std::format(
				"All values in time range must be non-negative.", e.first, e.second));
		}
		if (e.first > e.second) {
			throw V2SimError(std::format(
				"Left value ({}) must be smaller or equal to right value ({}).", e.first, e.second));
		}
	}
	size_t n = d.size();
	for (size_t i = 1; i < n; ++i) {
		if (d[i - 1].second >= d[i].first) {
			throw V2SimError(std::format(
				"Time ranges ({},{}) and ({},{}) must be non-overlapping and increasing.",
				d[i - 1].first, d[i - 1].second, d[i].first, d[i].second));
		}
	}
}

RangeList::RangeList(tinyxml2::XMLElement* xml, bool allow_null, bool null_value)
{
	if (!xml) {
		loop_period = 0;
		loop_times = 0;
		if (allow_null) {
			SetForce(null_value);
			return;
		}
		throw V2SimError("XML element is null.");
	}
	loop_period = xml->IntAttribute("loop_period", 0);
	loop_times = xml->IntAttribute("loop_times", 1);

	for (auto* e = xml->FirstChildElement("range"); e; e = e->NextSiblingElement("range")) {
		int left = e->IntAttribute("btime", -1);
		int right = e->IntAttribute("etime", -1);
		if(left < 0 || right < 0) {
			throw V2SimError(std::format("btime or etime is missed on line {}", e->GetLineNum()));
		}
		d.emplace_back(make_pair(left, right));
	}
	check();
}

RangeList::RangeList(const vector<pair<int, int>>& data, int period, int times) :
	d(data), loop_period(period), loop_times(times) {
	check();
}

RangeList::RangeList(const std::initializer_list<pair<int, int>> d, int period, int times) :
	loop_period(period), loop_times(times) {
	for (auto &e: d) {
		this->d.emplace_back(e);
	}
	check();
}

bool RangeList::Contains(int time) const {
	if (forced) {
		return forced_value;
	}
	if (loop_period > 0) {
		if (loop_times > 0 && time > loop_period * loop_times) {
			return false;
		}
		time %= loop_period;
	}
	for (auto& e : d) {
		if (e.first <= time && time <= e.second) {
			return true;
		}
	}
	return false;
}

vector<string> cross_list(const vector<string>& a, const vector<string>& b) {
	vector<string> ret;
	for (auto& a0 : a) {
		for (auto& b0 : b) {
			ret.push_back(a0 + "#" + b0);
		}
	}
	return ret;
}

constexpr const char* _DIGITS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

string to_base62(int num) {
	if (num == 0) {
		return "0";
	}
	string result;
	while (num) {
		int rem = num % 62;
		num /= 62;
		result.push_back(_DIGITS[rem]);

	}
	return result;
}