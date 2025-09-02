#include "inst.h"

void V2SimCore::assignCSPos() {
	if (!fcs.TreeInitialized()) {
		for (auto& cs : fcs) {
			if (isinf(cs.X) || isinf(cs.Y)) {
				auto& pos = getEdgePos(cs.Edge);
				cs.X = pos.x;
				cs.Y = pos.y;
			}
		}
		fcs.UpdateTree();
	}
	if (!scs.TreeInitialized()) {
		for (auto& cs : scs) {
			if (isinf(cs.X) || isinf(cs.Y)) {
				auto& pos = getEdgePos(cs.Edge);
				cs.X = pos.x;
				cs.Y = pos.y;
			}
		}
		scs.UpdateTree();
	}
}

bool V2SimCore::startTrip(int vid) {
	auto& ev = evs[vid];
	auto& trip = ev.CurrentTrip();
	if (ev.SoC() >= ev.KFast) {
		ev.TargetCS = -1;
		addVeh(ev, trip.FromEdge(), trip.ToEdge());
	} else {
		auto& e = trip.FromEdge();
		auto& pos = getEdgePos(e);
		int best_cs = getBestCS(ev, e, pos.x, pos.y);
		if (best_cs == -1) {
			return false;
		}
		ev.TargetCS = best_cs;
		addVeh(ev, e, fcs[best_cs].Edge);
	}
	scs.PopVeh(vid);
	ev.ClearPc();
	ev.Status = VehStatus::Pending;
	return true;
}

void V2SimCore::endTrip(int vid) {
	auto& ev = evs[vid];
	ev.Status = VehStatus::Parking;
	auto arr_sta = TripsLogger::ARRIVAL_NO_CHARGE;
	if (ev.SoC() < ev.KSlow) {
		if (scs.AddVeh(vid, ev.CurrentTrip().ToEdge())) {
			arr_sta = TripsLogger::ARRIVAL_CHARGE_SUCCESSFULLY;
		}
		else {
			arr_sta = TripsLogger::ARRIVAL_CHARGE_FAILED;
		}
	}
	if (tlog) {
		tlog->arrive(ctime, ev, arr_sta);
		if (arr_sta == TripsLogger::ARRIVAL_CHARGE_SUCCESSFULLY) {
			tlog->join_SCS(ctime, ev, scs[ev.TargetCS].ID);
		}
	}
	int tid = ev.NextTrip();
	if (tid != -1) {
		dq.push(make_pair(ev.CurrentTrip().DepartTime, vid));
	}
}

void V2SimCore::batchDepart() {
	while (!dq.empty() && dq.top().first <= ctime) {
		pair<int,int> top = dq.top(); // ²»×¼ÓÃconst auto& !
		int dtime = top.first;
		int vid = top.second;
		dq.pop();
		auto& ev = evs[vid];
		auto& trip = ev.CurrentTrip();
		if (ev.Status != VehStatus::Charging && ev.Status != VehStatus::Parking) {
			throw V2SimError(std::format("You cannot depart EV {} @ {}, which is neither charging nor parking.", ev.ID, ctime));
		}
		if (startTrip(vid)) {
			int depart_delay = max(0, ctime - dtime);
			string csname = ev.TargetCS < 0 ? "None" : fcs[ev.TargetCS].ID;
			if (tlog) tlog->depart(ctime, ev, depart_delay, csname);
		}
		else{
			if (scs.IsCharging(vid)) {
				if (tlog) tlog->depart_delay(ctime, ev, -1, 60 * 15);
				dq.push({ dtime + 60 * 15, vid }); // Delay 15min and try to re-depart
			}
			else {
				if (tlog) tlog->depart_failed(ctime, ev, -1, "Not supported", -1);
				setDepleted2(ev, vid, trip.FromEdge());
			}
		}
	}
}

int V2SimCore::getBestCS(EV& ev, const string& edge, double x, double y) {
	auto near_cs = fcs.SelectNear(x, y, 10);
	double min_weight = 1e10;
	int best_cs = -1;
	if (near_cs.has_value()) {
		for (auto& p : near_cs.value()) {
			auto& cs = fcs[p.label];
			if (!cs.IsOnline(ctime)) continue;
			auto stage = libsumo::Simulation::findRoute(edge, cs.Edge, "", -1.0, libsumo::ROUTING_MODE_AGGREGATED);
			if (stage.length > ev.MaxMileage()) continue;
			double t_drive = stage.travelTime / 60;
			double t_wait = max(0, (int)cs.VehCount() - cs.Slots) * 30;
			double weight = ev.Omega * (t_drive + t_wait) + (ev.BattCap - ev.BattElec) * cs.PriceBuy(ctime);
			if (weight < min_weight) {
				min_weight = weight;
				best_cs = p.label;
			}
		}
	}
	else {
		int i = 0;
		for (auto& cs : fcs) {
			if (!cs.IsOnline(ctime)) continue;
			auto stage = libsumo::Simulation::findRoute(edge, cs.Edge, "", -1.0, libsumo::ROUTING_MODE_AGGREGATED);
			if (stage.length > ev.MaxMileage()) continue;
			double t_drive = stage.travelTime / 60;
			double t_wait = max(0, (int)cs.VehCount() - cs.Slots) * 30;
			double weight = ev.Omega * (t_drive + t_wait) + (ev.BattCap - ev.BattElec) * cs.PriceBuy(ctime);
			if (weight < min_weight) {
				min_weight = weight;
				best_cs = i;
			}
			++i;
		}
	}
	return best_cs;
}

void V2SimCore::Start() {
	libsumo::Simulation::start({ "sumo", "-n", roadnet_path, "-b", to_string(start), "-e", to_string(end) });
	ctime = (int)libsumo::Simulation::getTime();
	size_t n = evs.size();
	while (!dq.empty()) {
		dq.pop();
	}
	for (int i = 0; i < n; ++i) {
		int t = evs[i].CurrentTrip().DepartTime;
		dq.push(make_pair(t, i));
	}
	assignCSPos();
	batchDepart();
}
void V2SimCore::Step(int len) {
	if (len == -1) {
		len = step;
	}
	libsumo::Simulation::step(ctime + step);
	int new_time = (int)libsumo::Simulation::getTime();
	int dt = new_time - ctime;
	ctime = new_time;

	auto cur_vehs = libsumo::Vehicle::getIDList();
	auto arr_vehs = libsumo::Simulation::getArrivedIDList();

	for (auto& vname : arr_vehs) {
		size_t vid = evs.IndexOf(vname);
		auto& ev = evs.Get(vname);
		if (ev.TargetCS == -1) {
			endTrip((int)vid);
		}
		else {
			ev.Status = VehStatus::Charging;
			fcs.AddVeh((int)vid, ev.TargetCS);
			if (tlog) tlog->arrive_FCS(ctime, ev, fcs[ev.TargetCS].ID);
		}
	}
	for (auto& vname : cur_vehs) {
		size_t vid = evs.IndexOf(vname);
		auto& ev = evs.Get(vname);
		ev.Drive(libsumo::Vehicle::getDistance(vname), ctime);
		if (ev.BattElec <= 0) {
			setDepleted(ev, (int)vid, vname);
			libsumo::Vehicle::remove(vname);
			if (tlog) tlog->fault_deplete(ctime, ev, "Not supported", -1);
			continue;
		}
		if (ev.Status == VehStatus::Pending) {
			ev.Status = VehStatus::Driving;
		}
		if (ev.Status == VehStatus::Driving) {
			if (ev.TargetCS != -1 && !fcs[ev.TargetCS].IsOnline(ctime)) {
				const string& edge = libsumo::Vehicle::getRoadID(vname);
				auto& pos = getEdgePos(edge);
				int cs_id = getBestCS(ev, libsumo::Vehicle::getRoadID(vname), pos.x, pos.y);
				auto cs_name = ev.TargetCS >= 0 ? fcs[ev.TargetCS].ID : "None";
				if (cs_id == -1) {
					setDepleted(ev, (int)vid, vname);
					libsumo::Vehicle::remove(vname);
					if (tlog) tlog->fault_nocharge(ctime, ev, cs_name);
				}
				else {
					ev.TargetCS = cs_id;
					libsumo::Vehicle::changeTarget(vname, fcs[cs_id].Edge);
					if (tlog) tlog->fault_redirect(ctime, ev, cs_name, fcs[cs_id].ID);
				}
			}
		}
		else {
			throw V2SimError(std::format("SUMO vehicles is not synchoronous with V2Sim for vehicle {} (Status: {}) at time {}", vname, (int)ev.Status, ctime));
		}
	}
	fcs.Update(evs, dt, ctime, tlog);
	scs.Update(evs, dt, ctime, tlog);
	batchDepart();
	while (!fq.empty() && fq.top().first <= ctime) {
		int vid = fq.top().second;
		fq.pop();
		auto& ev = evs[vid];
		ev.Status = VehStatus::Charging;
		if (!fcs.AddVeh(vid, ev.TargetCS)) {
			if (tlog) tlog->fault_nocharge(ctime, ev, "Cannot add depeleted EV to given CS");
			ev.BattElec = ev.BattCap * 0.5;
		}
		else {
			if (tlog) tlog->arrive_FCS(ctime, ev, fcs[ev.TargetCS].ID);
		}
	}
}