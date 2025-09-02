#pragma once

#include <format>
#include "tinyxml2.h"
#include "ev.h"

void Stringsplit(const string& str, const char split, vector<string>& res)
{
	if (str == "")	return;
	size_t pos = -1;
	size_t pos2 = str.find(split);

	while (pos2 != str.npos)
	{
		if (pos2 - pos > 1) {
			res.push_back(str.substr(pos + 1, pos2 - pos - 1));
		}
		pos = pos2;
		pos2 = str.find(split, pos + 1);
	}
	if (pos != str.size() - 1) {
		res.push_back(str.substr(pos + 1));
	}
}

Trip::Trip(const string& id, int dpt_time, const string& fTAZ, const string& tTAZ, 
	const vector<string>& route, bool auto_detect_fixed_route, bool fixed_route) noexcept :
	ID(id), DepartTime(dpt_time), FromTAZ(fTAZ), ToTAZ(tTAZ), route(route) {
	if (auto_detect_fixed_route) {
		FixedRoute = route.size() > 2;
	}
	else {
		FixedRoute = fixed_route;
	}
}

Trip::Trip(const string& id, int dpt_time, const string& fTAZ, const string& tTAZ, 
	const string& route, bool auto_detect_fixed_route, bool fixed_route) :
	ID(id), DepartTime(dpt_time), FromTAZ(fTAZ), ToTAZ(tTAZ) {
	Stringsplit(route, ' ', this->route);
	if (auto_detect_fixed_route) {
		FixedRoute = route.size() > 2;
	}
	else {
		FixedRoute = fixed_route;
	} 
}

const char* strattr(const tinyxml2::XMLElement* e, const char* attr) {
	const char* ret = e->Attribute(attr);
	if (ret == NULL) return "";
	return ret;
}

static string strlower(const char* s) {
	if (s == NULL) return "";
	string ret(s);
	for (auto& c : ret) {
		c = tolower(c);
	}
	return ret;
}

Trip::Trip(const tinyxml2::XMLElement* e) : ID(strattr(e, "id")), DepartTime(e->IntAttribute("depart", -1)),
	FromTAZ(strattr(e, "fromTaz")), ToTAZ(strattr(e, "toTaz")) {
	if (ID.empty()) {
		throw V2SimError(std::format("Trip ID must be specified. It cannot be empty string. Line {}", e->GetLineNum()));
	}
	const char* route = e->Attribute("route_edges");
	if (route == NULL) {
		throw V2SimError(std::format("Trip ID must be specified. It cannot be empty string. Line {}", e->GetLineNum()));
	}
	Stringsplit(route, ' ', this->route);
	if (this->route.size() < 2) {
		throw V2SimError(std::format("The route of a trip must contain 2 edges at least. Line {}", e->GetLineNum()));
	}
	const char* fixed_route = e->Attribute("fixed_route");
	if (fixed_route == NULL || strlower(fixed_route) == "none") {
		FixedRoute = this->route.size() > 2;
	} else if (strlower(fixed_route) == "true") {
		FixedRoute = true;
	} else if (strlower(fixed_route) == "false") {
		FixedRoute = false;
	} else {
		throw V2SimError(std::format("Invalid value for 'fixed_route' attribute in trip '{}'. It must be 'True', 'False', or 'None' in any case. Line {}", ID, e->GetLineNum()));
	}
}

unordered_map<string, BattCorrFunc> BattCorrFuncPool::_mp = {
	{"Equal", [](double p, double c, double soc) -> double { return p; } },
	{"Linear", [](double p, double c, double soc) -> double { return soc <= 0.8 ? p : p * (3.4 - 3 * soc); }}
};

EV::EV(const string& id, const vector<Trip>& trips, double eta_c, double eta_d, double cap_kWh, double soc,
	double range_km, double pc_fast_kW, double pc_slow_kW, double pd_v2g, double omega, double k_rel, double k_fast, double k_slow,
	double k_v2g, const string& rmod, const RangeList& sc_time, double max_sc_cost, const RangeList& v2g_time,
	double min_v2g_revenue, bool cache_route) : ID(id), EtaC(eta_c), EtaD(eta_d), BattCap(cap_kWh), BattElec(soc * cap_kWh), 
	Consumption(cap_kWh / (range_km * 1e3)), PcFast(pc_fast_kW / 3.6e3), PcSlow(pc_slow_kW / 3.6e3), PdV2G(pd_v2g / 3.6e3), 
	Omega(omega), KRel(k_rel), KFast(k_fast), KSlow(k_slow), KV2G(k_v2g), SlowChargeTime(sc_time), MaxSlowChargeCost(max_sc_cost),
	V2GTime(v2g_time), MinV2GRevenue(min_v2g_revenue), CacheRoute(cache_route) {
	this->rmod = BattCorrFuncPool::Get(rmod);
}

inline static double _dattrp(const tinyxml2::XMLElement* e, const char* attr, const char* desc, const char* vid, double def = -1) {
	double val = e->DoubleAttribute(attr, def);
	if (val < 0.0) {
		throw V2SimError(std::format("{} ({}) is not defined or invalid for vehicle '{}' on line {}! It must be positive.", desc, attr, vid, e->GetLineNum()));
	}
	return val;
}

inline static double _dattr01(const tinyxml2::XMLElement* e, const char* attr, const char* desc, const char* vid, double def = -1) {
	double val = e->DoubleAttribute(attr, def);
	if (val < 0.0) {
		throw V2SimError(std::format("{} ({}) is not defined or invalid for vehicle '{}' on line {}! It must be in [0.0, 1.0].", desc, attr, vid, e->GetLineNum()));
	}
	return val;
}

EV::EV(tinyxml2::XMLElement* cur) {
	const char* vid = cur->Attribute("id");
	if (!vid ) {
		throw V2SimError(std::format("Vehicle ID is not defined on line{}!", cur->GetLineNum()));
	}
	ID = vid;
	EtaC = _dattr01(cur, "eta_c", "Charging efficiency", vid, 0.9);
	EtaD = _dattr01(cur, "eta_d", "Discharging efficiency", vid, 0.9);
	BattCap = _dattrp(cur, "bcap", "Battery capcity", vid);
	BattElec = BattCap * _dattr01(cur, "soc", "SoC", vid, 0.9);
	Consumption = _dattrp(cur, "c", "Energy used per meter", vid) / 1e3; // convert from Wh/m to kWh/m
	PcFast = _dattrp(cur, "rf", "Fast charging power", vid) / 3.6e3; // convert from kW to kWh/s
	PcSlow = _dattrp(cur, "rs", "Slow charing power", vid) / 3.6e3; // convert from kW to kWh/s
	PdV2G = _dattrp(cur, "rv", "V2G discharging power", vid) / 3.6e3; // convert from kW to kWh/s
	Omega = _dattrp(cur, "omega", "Omega", vid);
	KRel = _dattrp(cur, "kr", "KRel", vid, 1.25);
	KFast = _dattr01(cur, "kf", "KFast", vid, 0.2);
	KSlow = _dattr01(cur, "ks", "KSlow", vid, 0.5);
	KV2G = _dattr01(cur, "kv", "KV2G", vid, 0.8);
	SlowChargeTime = RangeList(cur->FirstChildElement("sctime"), true, true);
	MaxSlowChargeCost = cur->DoubleAttribute("max_sc_cost", 100.0);
	V2GTime = RangeList(cur->FirstChildElement("v2gtime"), true, true);
	MinV2GRevenue = cur->DoubleAttribute("min_v2g_earn", 0.0);
	const char* rmod = cur->Attribute("rmod");
	if (!rmod) {
		rmod = "Linear";
	}
	this->rmod = BattCorrFuncPool::Get(rmod);
	const char* cache_route = cur->Attribute("cache_route");
	if (!cache_route || strlower(cache_route) != "true") {
		CacheRoute = false;
	}
	else {
		CacheRoute = true;
	}
	tinyxml2::XMLElement* tr = cur->FirstChildElement("trip");
	while (tr != NULL) {
		this->trips.emplace_back(Trip(tr));
		tr = tr->NextSiblingElement("trip");
	}
}

void EVMap::load(const char* fn) {
	using namespace tinyxml2;
	XMLDocument doc;
	XMLError err = doc.LoadFile(fn);
	if (err != XML_SUCCESS) {
		throw V2SimError(std::format("Fail to load '{}' (Code={}). Please ensure it is a valid XML file.", string(fn), (int)err));
	}
	XMLElement* root = doc.RootElement();
	if (!root) {
		throw V2SimError(std::format("Fail to load '{}'. Root element not found!", string(fn)));
	}
	XMLElement* cur = root->FirstChildElement("vehicle");
	while (cur != NULL) {
		this->Add(EV(cur));
		cur = cur->NextSiblingElement("vehicle");
	}
}