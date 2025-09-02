#pragma once

#include "cs.h"

inline static double _dattrp(const tinyxml2::XMLElement* e, const char* attr, const char* desc, const char* cid, double def = -1) {
	double val = e->DoubleAttribute(attr, def);
	if (val < 0.0) {
		throw V2SimError(std::format("{} ({}) is not defined or invalid for EVCS '{}' on line {}! It must be positive.", desc, attr, cid, e->GetLineNum()));
	}
	return val;
}

inline static int _iattrp(const tinyxml2::XMLElement* e, const char* attr, const char* desc, const char* cid, int def = -1) {
	int val = e->IntAttribute(attr, def);
	if (val < 0) {
		throw V2SimError(std::format("{} ({}) is not defined or invalid for EVCS '{}' on line {}! It must be positive.", desc, attr, cid, e->GetLineNum()));
	}
	return val;
}

constexpr double inf = std::numeric_limits<double>::infinity();

EVCS::EVCS(tinyxml2::XMLElement* e):
	X(e->DoubleAttribute("x", inf)),
	Y(e->DoubleAttribute("y", inf)),
	offline(e->FirstChildElement("offline"), true, false)
{
	const char* id = e->Attribute("name");
	if (!id) {
		throw V2SimError(std::format("EVCS ID is not defined on line {}!", e->GetLineNum()));
	}
	ID = id;
	
	const char* edge = e->Attribute("edge");
	if (!edge) {
		throw V2SimError(std::format("Edge (edge) is not foudn for EVCS {}!", id));
	}
	Edge = edge;

	Slots = _iattrp(e, "slots", "Number of chargers", id, -1);
	const char* bus = e->Attribute("bus");
	if (!bus) {
		throw V2SimError(std::format("Bus (bus) not found for EVCS {}!", id));
	}
	Bus = bus;
	double tot_max_pc = _dattrp(e, "max_pc", "Maximum charging power", id, 1e9);
	TotalPcLimit = tot_max_pc;
	SinglePcLimit.assign(Slots, tot_max_pc / Slots);
	
	double tot_max_pd = _dattrp(e, "max_pd", "Maximum discharging pwoer", id, 1e9);
	TotalPdLimit = tot_max_pd;

	SinglePdActual.assign(Slots, 0.0);
	
	auto* pbuy_elem = e->FirstChildElement("pbuy");
	if (!pbuy_elem) {
		throw V2SimError(std::format("User price for charging (pbuy) not found for EVCS {}!", id));
	}
	pbuy = SegFunc(pbuy_elem, false, "item", "btime", "price");

	psell = SegFunc(e->FirstChildElement("psell"), true, "item", "btime", "price");

	const char* pdalloc = e->Attribute("pd_alloc");
	PdAlloc = V2GAllocPool::Get(pdalloc ? pdalloc : "Average");
}

unordered_map<string, V2GAlloc> V2GAllocPool::_mp = {
	{"", [](EVMap& mp, vector<int>& vids, double cap, int ctime, double ratio)->vector<double> {
		throw V2SimError("Empty V2GAlloc function is only a placeholder that cannot be really called.");
	}},
	{"Average", [](EVMap& mp, vector<int>& vids, double cap, int ctime, double ratio)->vector<double> {
		return vector<double>(vids.size(), ratio);
	}}
};

vector<int> SlowCS::Update(EVMap& mp, int sec, int ctime, double v2g_k) {
	double Wcharge = 0;
	double Wdischarge = 0;
	if (not IsOnline(ctime)) {
		// Do nothing when the charging station fails
		cload = dload = 0;
		return vector<int>();
	}
	int i = 0;
	vector<int> ret;
	for (auto it = chi.begin(); it != chi.end(); ++it, ++i) {
		int vid = *it;
		auto& ev = mp[vid];
		auto pb = pbuy(ctime);
		if (ev.CanSlowCharge(ctime, pb)) {
			// If V2G discharge is in progress, don't charge to full
			Wcharge += ev.Charge(sec, pb, min(SinglePcLimit[i], ev.PcSlow));
			auto k = v2g_k > 0 ? min(1.0, ev.KV2G) : 1;
			if (ev.BattElec >= ev.BattCap * k) {
				chi.erase(vid);
				free.insert(vid);
			}
		}
	}
	if (v2g_k > 0) {
		auto ps = psell(ctime);
		vector<int> v2g_vehs;
		for (auto& vid : free) {
			auto& ev = mp[vid];
			if (ev.CanV2G(ctime, ps)) {
				v2g_vehs.push_back(vid);
			}
		}
		SinglePdActual = PdAlloc(mp, v2g_vehs, v2g_cap, ctime, v2g_k);
		if (SinglePdActual.size() != v2g_vehs.size()) {
			throw V2SimError(std::format("PdAlloc do not return a vector with propoer size: {}", v2g_vehs.size()));
		}
		int i = 0;
		for (auto& vid : v2g_vehs) {
			auto& ev = mp[vid];
			Wdischarge += ev.Discharge(v2g_k * SinglePdActual[i], sec, ps);
			++i;
		}
	}
	cload = Wcharge / sec;
	dload = Wdischarge / sec;
	return vector<int>();
}

double SlowCS::V2GCapacity(EVMap& mp, int ctime) {
	if (!IsOnline(ctime)) {
		return 0.0;
	}
	double tot_rate_ava = 0.0;
	// Do not check if psell is None due to performance considerations
	double v2gp = psell(ctime);
	for (auto& vid : chi) {
		auto& ev = mp[vid];
		if (ev.CanV2G(ctime, v2gp)) {
			tot_rate_ava += ev.PdV2G * ev.EtaD;
		}
	}
	for (auto& vid : free) {
		auto& ev = mp[vid];
		if (ev.CanV2G(ctime, v2gp)) {
			tot_rate_ava += ev.PdV2G * ev.EtaD;
		}
	}
	v2g_cap = tot_rate_ava;
	return tot_rate_ava;
}
vector<int> FastCS::Update(EVMap& mp, int sec, int ctime, double v2g_k) {
	double Wcharge = 0;
	vector<int> ret;
	if (not IsOnline(ctime)) {
		// Do nothing when the charging station fails
		cload = 0;
		for (auto& e : chi) {
			ret.push_back(e);
		}
		for (auto& e : buf) {
			ret.push_back(e);
		}
		chi.clear();
		buf.clear();
		return ret;
	}
	int i = 0;
	for (auto it = chi.begin(); it != chi.end(); ++it, ++i) {
		int vid = *it;
		auto& ev = mp[vid];
		Wcharge += ev.Charge(sec, pbuy(ctime), min(SinglePcLimit[i], ev.PcFast));
		if (ev.BattElec >= ev.BattCap) {
			ret.push_back(vid);
		}
	}
	for (auto& vid : ret) {
		PopVeh(vid);
		auto t = buf.pop();
		if (t.has_value()) {
			chi.insert(t.value());
		}
	}
	cload = Wcharge / sec;
	return ret;
}