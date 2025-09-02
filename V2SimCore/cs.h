#pragma once

#include<unordered_set>
#include<ranges>
#include "ev.h"

// EVMap, Vehicle Names, min(V2G_Capacity, MaxPdLimit), Current_Time, ActualRatio
using V2GAlloc = function<vector<double>(EVMap&, vector<int>&, double, int, double)>;

class V2GAllocPool {
private:
	static unordered_map<string, V2GAlloc> _mp;
public:
	static void Add(const string& id, V2GAlloc v2galloc) noexcept {
		V2GAllocPool::_mp[id] = v2galloc;
	}
	static V2GAlloc& Get(const string& id) {
		const auto it = V2GAllocPool::_mp.find(id);
		if (it == V2GAllocPool::_mp.end()) {
			throw V2SimError(std::format("V2G allocation function not found: {}", id));
		}
		return it->second;
	}
};

class EVCS {
protected:
	RangeList offline;
	SegFunc pbuy;
	SegFunc psell;
	double cload = 0.0;
	double dload = 0.0;
	double v2g_cap = 0.0;

public:
	string ID;
	string Edge;
	int Slots;
	string Bus;
	double X, Y;

	// Pc limit for each charger, kWh/s
	vector<double> SinglePcLimit;

	// Total Pc limit for all the chargers, kWh/s
	double TotalPcLimit;

	// Actual Pd for each charger, kWh/s
	vector<double> SinglePdActual;

	// Total Pd limit for all the chargers, kWh/s
	double TotalPdLimit;

	V2GAlloc PdAlloc;

	SegFunc& PriceBuy() { return pbuy; }
	double PriceBuy(int t) const { return pbuy(t); }
	SegFunc& PriceSell() { return psell; }
	double PriceSell(int t) const { return psell(t); }
	bool SupportV2G() const { return psell.size() > 0; }
	bool IsOnline(int t) const { return !offline.Contains(t); }
	void ForceShutdown() { offline.SetForce(true); }
	void ForceReopen() { offline.SetForce(false); }
	void ClearForceOffline() { offline.ClearForce(); }

	double Pc() const { return cload; }
	double Pc_kW() const { return cload * 3600; }
	double Pc_MW() const { return cload * 3.6; }
	double Pd() const { return dload; }
	double Pd_kW() const { return dload * 3600; }
	double Pd_MW() const { return dload * 3.6; }
	double Pv2g() const { return v2g_cap; }
	double Pv2g_kW() const { return v2g_cap * 3600; }
	double Pv2g_MW() const { return v2g_cap * 3.6; }

	virtual bool AddVeh(int vid) = 0;
	virtual bool PopVeh(int vid) = 0;
	virtual bool HasVeh(int vid) const = 0;
	virtual bool IsCharging(int vid) const = 0;
	virtual size_t size() const = 0;
	virtual size_t VehCount(bool only_charging = false) const = 0;

	virtual vector<int> Update(EVMap& mp, int sec, int ctime, double v2g_k) = 0;
	virtual double V2GCapacity(EVMap& mp, int ctime) = 0;
	virtual double V2GCapBuffer() const = 0;

	EVCS(const string& id, const string& edge, int slots, const string& bus, double x, double y, const RangeList& offline,
		double tot_max_pc, double tot_max_pd, const SegFunc& pbuy, const SegFunc& psell, const string& v2g_alloc) :
		ID(id), Edge(edge), Slots(slots), Bus(bus), X(x), Y(y), offline(offline), SinglePcLimit(slots, tot_max_pc / slots), SinglePdActual(slots, 0.0),
		TotalPcLimit(tot_max_pc), TotalPdLimit(tot_max_pd), pbuy(pbuy), psell(psell) {
		PdAlloc = V2GAllocPool::Get(v2g_alloc);
	}

	EVCS(tinyxml2::XMLElement* e);
};

class SlowCS : public EVCS {
protected:
	OrderedHashSet<int> chi;
	unordered_set<int> free;
public:
	SlowCS(const string& id, const string& edge, int slots, const string& bus, double x, double y, const RangeList& offline,
		double tot_max_pc, double tot_max_pd, const SegFunc& pbuy, const SegFunc& psell, const string& v2g_alloc) :
		EVCS(id, edge, slots, bus, x, y, offline, tot_max_pc, tot_max_pd, pbuy, psell, v2g_alloc) {

	}
	SlowCS(tinyxml2::XMLElement* e) : EVCS(e) {}

	virtual bool AddVeh(int vid) {
		if (HasVeh(vid)) {
			return false;
		}
		if (size() < Slots) {
			chi.insert(vid);
			return true;
		}
		return false;
	}
	virtual bool PopVeh(int vid) {
		return free.erase(vid) || chi.erase(vid);
	}
	virtual bool HasVeh(int vid) const {
		return chi.contains(vid) || free.contains(vid);
	}
	virtual bool IsCharging(int vid) const {
		return chi.contains(vid);
	}
	virtual size_t size() const {
		return chi.size() + free.size();
	}
	virtual size_t VehCount(bool only_charging = false) const {
		return only_charging ? chi.size() : size();
	}

	virtual vector<int> Update(EVMap& mp, int sec, int ctime, double v2g_k);

	virtual double V2GCapacity(EVMap& mp, int ctime);

	virtual double V2GCapBuffer() const { return v2g_cap; }
};

class FastCS : public EVCS {
	OrderedHashSet<int> chi, buf;
public:
	FastCS(const string& id, const string& edge, int slots, const string& bus, double x, double y, const RangeList& offline, double tot_max_pc, const SegFunc& pbuy) :
		EVCS(id, edge, slots, bus, x, y, offline, tot_max_pc, 0, pbuy, SegFunc(), "") { }
	FastCS(tinyxml2::XMLElement* e) : EVCS(e) {}

	virtual bool AddVeh(int vid) {
		if (HasVeh(vid)) {
			return false;
		}
		if (chi.size() < Slots) {
			chi.insert(vid);
		}else{
			buf.insert(vid);
		}
		return true;
	}
	virtual bool PopVeh(int vid) {
		return chi.erase(vid) || buf.erase(vid);
	}
	virtual bool HasVeh(int vid) const {
		return chi.contains(vid) || buf.contains(vid);
	}
	virtual bool IsCharging(int vid) const {
		return chi.contains(vid);
	}
	virtual size_t size() const {
		return chi.size() + buf.size();
	}
	virtual size_t VehCount(bool only_charging = false) const {
		return only_charging ? chi.size() : size();
	}

	virtual vector<int> Update(EVMap& mp, int sec, int ctime, double v2g_k);

	virtual double V2GCapacity(EVMap& mp, int ctime) { return 0.0; }

	virtual double V2GCapBuffer() const { return 0.0; }
};