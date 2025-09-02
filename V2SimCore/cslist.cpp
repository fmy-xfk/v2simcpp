#include "cslist.h"


void FastCSMap::Update(EVMap& mp, int sec, int ctime, TripsLogger* tlog) {
	for (auto& c : cs) {
		auto ret = c.Update(mp, sec, ctime, 0);
		for (auto& vid : ret) {
			PopVeh(vid);
			auto& ev = mp[vid];
			if (ev.TargetCS == -1) {
				throw V2SimError("An EV without target CS ends charging at FCS.");
			}
			auto& trip = ev.CurrentTrip();
			ev.Distance = 0;
			AddVehToSUMO(ev.ID, cs[ev.TargetCS].Edge, ev.CurrentTrip().ToEdge());
			ev.TargetCS = -1;
			ev.Status = VehStatus::Pending;
			ev.ClearPc();
			if (tlog) {
				tlog->depart_FCS(ctime, ev, c.ID);
			}
		}
	}
}

void SlowCSMap::UpdateV2GCapacities(EVMap& mp, int t) {
	if (t == v2g_cap_res_time) return;
	size_t n = cs.size();
	for (size_t i = 0; i < n; ++i) {
		v2g_cap_res[i] = cs[i].V2GCapacity(mp, t);
	}
}

void SlowCSMap::ClearV2GDemand() {
	size_t n = v2g_demand.size();
	for (size_t i = 0; i < n; ++i) {
		v2g_demand[i] = 0;
	}
}

void SlowCSMap::Update(EVMap& mp, int sec, int ctime, TripsLogger *tlog) {
	size_t n = v2g_k.size();
	UpdateV2GCapacities(mp, ctime);
	for (size_t i = 0; i < n; ++i) {
		if (v2g_cap_res[i] > 0.0) {
			v2g_k[i] = min(1.0, v2g_demand[i] / v2g_cap_res[i]);
		}
		else {
			v2g_k[i] = 0.0;
		}
	}
	for (size_t i = 0; i < n; ++i) {
		auto ret = cs[i].Update(mp, sec, ctime, v2g_k[i]);
		for (auto& vid : ret) {
			PopVeh(vid);
			if (tlog) {
				tlog->leave_SCS(ctime, mp[vid], cs[i].ID);
			}
		}
	}
}