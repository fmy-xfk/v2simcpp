#pragma once

#include <libsumo/libsumo.h>
#include "triplogger.h"
#include "cslist.h"

class V2SimCore {
private:
	TripsLogger* tlog;
	string roadnet_path;
	int start, end, step;
	int ctime;
	EVMap& evs;
	FastCSMap& fcs;
	SlowCSMap& scs;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> dq; // time, vid
	priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> fq; // time, vid

	void addVeh(EV& ev, const string& from, const string& to) {
		ev.Distance = 0;
		AddVehToSUMO(ev.ID, from, to);
	}

	int getBestCS(EV& ev, const string& edge, double x, double y);
	auto& getEdgePos(const string& edge) {
		return libsumo::Lane::getShape(edge + "_0").value[0];
	}

	void assignCSPos();
	bool startTrip(int vid);
	void endTrip(int vid);

	Point getNearestFCS(const string& edge) {
		auto& pos = getEdgePos(edge);
		return fcs.FindNearestCS(pos.x, pos.y);
	}
	void batchDepart();

	void setDepleted2(EV& ev, int vid, const string& edge) {
		ev.Status = VehStatus::Depleted;
		ev.TargetCS = getNearestFCS(edge).label;
		fq.push({ ctime + 3600, vid }); // Drag to nearest CS after an hour.
		if(tlog) tlog->fault_deplete(ctime, ev, ev.TargetCS >= 0 ? fcs[ev.TargetCS].ID : "None", -1);
	}
	void setDepleted(EV& ev, int vid, const string& vname) {
		const string& edge = libsumo::Vehicle::getRoadID(vname);
		setDepleted2(ev, vid, edge);
	}
	V2SimCore(V2SimCore&) = delete;
	V2SimCore& operator=(V2SimCore&) = delete;
public:
	V2SimCore(int start_time, int end_time, int step_length, const string& roadnet, EVMap& evs, FastCSMap& fcs, SlowCSMap& scs, TripsLogger* tlog) :
		start(start_time), ctime(start_time), end(end_time), step(step_length), roadnet_path(roadnet), evs(evs), fcs(fcs), scs(scs), tlog(tlog) {
	}
	EVMap& EVs() { return evs; }
	FastCSMap& FCSs() { return fcs; }
	SlowCSMap& SCSs() { return scs; }
	const EVMap& EVs() const { return evs; }
	const FastCSMap& FCSs() const { return fcs; }
	const SlowCSMap& SCSs() const { return scs; }
	int getTime() const { return ctime; }
	int getStartTime() const { return start; }
	int getEndTime() const { return end; }
	int getStepLength() const { return step; }

	void Start();

	void Step(int len = -1);

	void Stop() {
		libsumo::Simulation::close("V2Sim completed.");
	}
};