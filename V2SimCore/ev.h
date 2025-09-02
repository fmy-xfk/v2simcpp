#pragma once

#include<string>
#include<vector>
#include<functional>
#include<xutility>
#include "utils.h"
using namespace std;

enum class VehStatus {
	Driving = 0,
	Pending = 1,
	Charging = 2,
	Parking = 3,
	Depleted = 4,
};

class Trip {
private:
	vector<string> route;
public:
	// ID of the trip
	string ID; 

	// Time of departure
	int DepartTime; 

	// TAZ of the origin
	string FromTAZ; 

	// TAZ of the destination
	string ToTAZ;	

	// Edges covered in the trip. 
	// It is possible that only origin edge and destination edge instead of all the edges passed throguh are included.
	vector<string>& Route() noexcept { return route; }

	const string& FromEdge() const noexcept { return route.front(); }

	const string& ToEdge() const noexcept { return route.back(); }
	// Whether the route is fixed. Do not fix the route if it only contains the origin edge and destination edge.
	bool FixedRoute;

	Trip(const string& id, int dpt_time, const string& fTAZ, const string& tTAZ, const vector<string>& route, bool auto_detect_fixed_route = true, bool fixed_route = false) noexcept;
	Trip(const string& id, int dpt_time, const string& fTAZ, const string& tTAZ, const string& route, bool auto_detect_fixed_route = true, bool fixed_route = false);
	Trip(const tinyxml2::XMLElement* e);

	const string __repr__() const {
		return std::format("{}->{}@{}", ToEdge(), FromEdge(), DepartTime);
	}
};

// PcNominal, BattCap, SoC -> RealPc
using BattCorrFunc = function<double(double, double, double)>;

class BattCorrFuncPool {
private:
	static unordered_map<string, BattCorrFunc> _mp;
public:
	static void Add(const string& id, BattCorrFunc bcf) noexcept {
		BattCorrFuncPool::_mp[id] = bcf;
	}
	static BattCorrFunc& Get(const string& id) {
		const auto it = BattCorrFuncPool::_mp.find(id);
		if (it == BattCorrFuncPool::_mp.end()) {
			throw V2SimError(std::format("Battery correction function not found: {}", id));
		}
		return it->second;
	}
};


class EV {
private:
	int trip_idx = 0;
	vector<Trip> trips;
	double pc = 0.0; //kWh/s
	BattCorrFunc rmod;
	int lastTime = -1;

public:
	string ID;
	VehStatus Status = VehStatus::Parking;
	int TargetCS = -1;
	double Cost = 0.0;
	double Revenue = 0.0;

	//Battery capacity, kWh
	double BattCap;

	//Current electricity in the battery, kWh
	double BattElec;

	//Fast charging power, kWh/s
	double PcFast;			
	double PcFast_kW() const { return PcFast * 3.6e3; }

	//Slow charging power, kWh/s
	double PcSlow;			
	double PcSlow_kW() const { return PcSlow * 3.6e3; }

	//Charging efficiency
	double EtaC;

	//V2G discharging power, kWh/s
	double PdV2G;			
	double PdV2G_kW() const { return PdV2G * 3.6e3; }

	//Discharging efficiency
	double EtaD;

	//Electricity consumed for each meter driven, kWh/m
	double Consumption;

	double Omega;
	double KRel;
	double KFast;
	double KSlow;
	double KV2G;
	double Distance = 0.0;	//Distance drived since the beginning of the trip.
	RangeList SlowChargeTime;
	double MaxSlowChargeCost;
	RangeList V2GTime;
	double MinV2GRevenue;
	bool CacheRoute;

	EV(const string& id, const vector<Trip>& trips, double eta_c, double eta_d, double cap_kWh, double soc,
		double range_km, double pc_fast_kW, double pc_slow_kW, double pd_v2g_kW, double omega, double k_rel, double k_fast, double k_slow,
		double k_v2g, const string& rmod, const RangeList& sc_time, double max_sc_cost, const RangeList& v2g_time, 
		double min_v2g_revenue, bool cache_route);

	EV(tinyxml2::XMLElement* e);
	
	void ClearPc() { pc = 0.0; }

	// Trips
	vector<Trip>& Trips() { return trips; }

	// State of charge
	double SoC() const { return BattElec / BattCap; }

	// Current Real Pc
	double Pc() const { return pc; }

	// Current Real Pc in kW
	double Pc_kW() const { return pc * 3600.0; }

	// Time required to complete charging at the current charge level, target charge level and charging rate
	double EstChargeTime() const {
		if (pc > 0) {
			return max((BattCap - BattElec) / pc, 0.0);
		}
		else {
			return -1;
		}
	}

	// Drive till the new distance (m)
	void Drive(double new_dist, int ctime) {
		if (new_dist < Distance - 1) {
			throw V2SimError(std::format("EV {}: Current distance {} @ {} > new distance {} @ {}, cur_trip = {}", ID, Distance, lastTime, new_dist, ctime, TripID()));
		}
		BattElec -= (new_dist - Distance) * Consumption;
		Distance = new_dist;
		lastTime = ctime;
	}
	
	//Charge for t seconds, return electricity charged (kWh)
	double Charge(int t, double unit_cost, double pc_nominal_kWhps) {
		double elec = BattElec;
		pc = rmod(pc_nominal_kWhps, BattCap, SoC());
		BattElec += pc * t * EtaC;
		if (BattElec > BattCap) {
			BattElec = BattCap;
		}
		double d_elec = BattElec - elec;
		Cost += (d_elec / EtaC) * unit_cost;
		return d_elec;
	}

	//Discharge for t seconds
	double Discharge(double k, int t, double unit_revenue) {
		double elec = BattElec;
		BattElec -= PdV2G * t * k;
		if (SoC() <= KV2G) {
			BattElec = BattCap * KV2G;
		}
		double d_elec = (elec - BattElec) * EtaD;
		Revenue += d_elec * unit_revenue;
		return d_elec;
	}

	// Whether be willing to join V2G at given time and revenue
	bool CanV2G(int t, double revenue) const {
		return SoC() > KV2G && revenue >= MinV2GRevenue && V2GTime.Contains(t);
	}

	// Whether be willing to slow charge at given time and cost
	bool CanSlowCharge(int t, double cost) const {
		return SoC() < KSlow && cost <= MaxSlowChargeCost && SlowChargeTime.Contains(t);
	}

	const Trip& CurrentTrip() const {
		return trips.at(trip_idx);
	}

	const Trip& TripAt(int idx) const {
		return trips.at(idx);
	}

	const size_t TripsCount() const {
		return trips.size();
	}

	const int TripID() const {
		return trip_idx;
	}

	int NextTrip() {
		if (trip_idx == trips.size() - 1) {
			return -1;
		}
		return ++trip_idx;
	}

	double MaxMileage() const {
		return BattElec / Consumption;
	}

	bool IsBattEnough(double dist) const {
		return MaxMileage() >= KRel * dist;
	}

	const string brief() const {
		return std::format("{},{:.1f}%,{}", ID, SoC() * 100, trip_idx);
	}
};

class EVMap {
	vector<EV> evs;
	unordered_map<string, size_t> mp;
	EVMap(EVMap&) = delete;
	EVMap& operator=(EVMap&) = delete;
	void load(const char* filename);
public:
	EVMap() {}
	EVMap(const char* filename) { 
		load(filename); 
	}
	EVMap(const string& filename) {
		load(filename.c_str());
	}
	auto begin() const {
		return evs.begin();
	}
	auto end() const {
		return evs.end();
	}
	auto cbegin() const {
		return evs.cbegin();
	}
	auto cend() const {
		return evs.cend();
	}
	EV& operator[](size_t idx) {
		return evs.at(idx);
	}
	const EV& operator[](size_t idx) const{
		return evs.at(idx);
	}
	EV& Get(const string& s) {
		return evs[IndexOf(s)];
	}
	const EV& Get(const string& s) const{
		return evs[IndexOf(s)];
	}
	size_t IndexOf(const string& s) const {
		auto it = mp.find(s);
		if (it == mp.end()) {
			throw V2SimError(std::format("Vehicle {} not found.", s));
		}
		return it->second;
	}
	void Add(const EV& v) {
		mp[v.ID] = evs.size();
		evs.push_back(v);
	}
	void Add(EV&& v) {
		mp[v.ID] = evs.size();
		evs.push_back(v);
	}
	void Clear() {
		mp.clear();
		evs.clear();
	}
	size_t size() {
		return evs.size();
	}
};