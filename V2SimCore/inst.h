#pragma once

#include "stat.h"

class V2SimInterface : public V2SimCore {
private:
	EVMap evs;
	FastCSMap fcs;
	SlowCSMap scs;
	TripsLogger tlog;
	vector<StatItem*> stats;
public:
	using V2SimCore::getTime;
	using V2SimCore::getStartTime;
	using V2SimCore::getEndTime;
	using V2SimCore::getStepLength;
	using V2SimCore::Start;
	using V2SimCore::Stop;

	V2SimInterface(int start_time, int end_time, int step_length, const string& roadnet,
		const string& ev_file, const string& fcs_file, const string& scs_file, const string& output_dir,
		bool log_fcs = true, bool log_scs = true, bool log_ev = false) :
		evs(ev_file), fcs(fcs_file.c_str(), "fcs"), scs(scs_file.c_str(), "scs"), tlog((output_dir + "/cproc.clog").c_str()),
		V2SimCore(start_time, end_time, step_length, roadnet, evs, fcs, scs, &tlog) {
		if (log_fcs) {
			stats.emplace_back(new StatFCS(output_dir + "/fcs.csv", fcs.CSIDs(), true));
		}
		if (log_scs) {
			stats.emplace_back(new StatSCS(output_dir + "/scs.csv", scs.CSIDs(), true));
		}
		/*if (log_ev) {
			stats.emplace_back(new StatEV(output_dir + "/ev.csv", false));
		}*/
	}

	void Step(int len = -1) {
		V2SimCore::Step(len);
		for (StatItem* si : stats) {
			si->recordItems(*this);
		}
	}

	~V2SimInterface() {
		for (StatItem* si : stats) {
			delete si;
		}
	}

	size_t EV_IndexOf(const string& vname) const { return evs.IndexOf(vname); }
	const string& EV_getName(size_t vid) const { return evs[vid].ID; }

	VehStatus EV_getStatus(size_t vid) const { return evs[vid].Status; }
	void EV_setStatus(size_t vid, VehStatus status) { evs[vid].Status = status; }

	int EV_getTargetCSIndex(size_t vid) const { return evs[vid].TargetCS; }
	void EV_setTargetCSIndex(size_t vid, int cs_index) { evs[vid].TargetCS = cs_index; }

	double EV_getCost(size_t vid) const { return evs[vid].Cost; }
	void EV_setCost(size_t vid, double cost) { evs[vid].Cost = cost; }

	double EV_getRevenue(size_t vid) const { return evs[vid].Revenue; }
	void EV_setRevenue(size_t vid, double revenue) { evs[vid].Revenue = revenue; }

	double EV_getBattCap(size_t vid) const { return evs[vid].BattCap; }
	void EV_setBattCap(size_t vid, double battcap) { evs[vid].BattCap = battcap; }

	double EV_getBattElec(size_t vid) const { return evs[vid].BattElec; }
	void EV_setBattElec(size_t vid, double battelec) { evs[vid].BattElec = battelec; }

	double EV_getPcFast(size_t vid) const { return evs[vid].PcFast; }
	void EV_setPcFast(size_t vid, double pcf) { evs[vid].PcFast = pcf; }

	double EV_getPcFast_kW(size_t vid) const { return evs[vid].PcFast_kW(); }
	void EV_setPcFast_kW(size_t vid, double pcf_kW) { evs[vid].PcFast = pcf_kW / 3.6e3; }

	double EV_getPcSlow(size_t vid) const { return evs[vid].PcSlow; }
	void EV_setPcSlow(size_t vid, double pcs) { evs[vid].PcSlow = pcs; }

	double EV_getPcSlow_kW(size_t vid) const { return evs[vid].PcSlow_kW(); }
	void EV_setPcSlow_kW(size_t vid, double pcs_kW) { evs[vid].PcSlow = pcs_kW / 3.6e3; }

	double EV_getEtaC(size_t vid) const { return evs[vid].EtaC; }
	void EV_setEtaC(size_t vid, double etac) { evs[vid].EtaC = etac; }

	double EV_getPdV2G(size_t vid) const { return evs[vid].PdV2G; }
	void EV_setPdV2G(size_t vid, double pdv2g) { evs[vid].PdV2G = pdv2g; }

	double EV_getPdV2G_kW(size_t vid) const { return evs[vid].PdV2G_kW(); }
	void EV_setPdV2G_kW(size_t vid, double pdv2g_kW) { evs[vid].PdV2G = pdv2g_kW / 3.6e3; }

	double EV_getEtaD(size_t vid) const { return evs[vid].EtaD; }
	void EV_setEtaD(size_t vid, double etad) { evs[vid].EtaD = etad; }

	double EV_getConsumption(size_t vid) const { return evs[vid].Consumption; }
	void EV_setConsumption(size_t vid, double consumption) { evs[vid].Consumption = consumption; }

	double EV_getOmega(size_t vid) const { return evs[vid].Omega; }
	void EV_setOmega(size_t vid, double omega) { evs[vid].Omega = omega; }

	double EV_getKRel(size_t vid) const { return evs[vid].KRel; }
	void EV_setKRel(size_t vid, double krel) { evs[vid].KRel = krel; }

	double EV_getKFast(size_t vid) const { return evs[vid].KFast; }
	void EV_setKFast(size_t vid, double kfast) { evs[vid].KFast = kfast; }

	double EV_getKSlow(size_t vid) const { return evs[vid].KSlow; }
	void EV_setKSlow(size_t vid, double kslow) { evs[vid].KSlow = kslow; }

	double EV_getKV2G(size_t vid) const { return evs[vid].KV2G; }
	void EV_setKV2G(size_t vid, double kv2g) { evs[vid].KV2G = kv2g; }

	double EV_getDistance(size_t vid) const { return evs[vid].Distance; }
	void EV_setDistance(size_t vid, double distance) { evs[vid].Distance = distance; }

	const RangeList& EV_getSlowChargeTime(size_t vid) const { return evs[vid].SlowChargeTime; }
	void EV_setSlowChargeTime(size_t vid, const RangeList& sct) { evs[vid].SlowChargeTime = sct; }

	double EV_getMaxSlowChargeCost(size_t vid) const { return evs[vid].MaxSlowChargeCost; }
	void EV_setMaxSlowChargeCost(size_t vid, double mscc) { evs[vid].MaxSlowChargeCost = mscc; }

	const RangeList& EV_getV2GTime(size_t vid) const { return evs[vid].V2GTime; }
	void EV_setV2GTime(size_t vid, const RangeList& v2gt) { evs[vid].V2GTime = v2gt; }

	double EV_getMinV2GRevenue(size_t vid) const { return evs[vid].MinV2GRevenue; }
	void EV_setMinV2GRevenue(size_t vid, double mv2gr) { evs[vid].MinV2GRevenue = mv2gr; }

	bool EV_getCacheRoute(size_t vid) const { return evs[vid].CacheRoute; }
	void EV_setCacheRoute(size_t vid, bool cr) { evs[vid].CacheRoute = cr; }

	void EV_ClearPc(size_t vid) { evs[vid].ClearPc(); }

	double EV_SoC(size_t vid) const { return evs[vid].SoC(); }
	double EV_Pc(size_t vid) const { return evs[vid].Pc(); }
	double EV_Pc_kW(size_t vid) const { return evs[vid].Pc_kW(); }
	double EV_EstChargeTime(size_t vid) const { return evs[vid].EstChargeTime(); }

	void EV_Drive(size_t vid, double new_dist, int ctime) { evs[vid].Drive(new_dist, ctime); }
	void EV_DriveNow(size_t vid, double new_dist) { evs[vid].Drive(new_dist, getTime()); }
	double EV_Charge(size_t vid, int t, double unit_cost, double pc_nominal_kWhps) { return evs[vid].Charge(t, unit_cost, pc_nominal_kWhps); }
	double EV_Discharge(size_t vid, double k, int t, double unit_revenue) { return evs[vid].Discharge(k, t, unit_revenue); }

	bool EV_CanV2G(size_t vid, int t, double revenue) const { return evs[vid].CanV2G(t, revenue); }
	bool EV_CanV2GNow(size_t vid, double revenue) const { return evs[vid].CanV2G(getTime(), revenue); }
	bool EV_CanSlowCharge(size_t vid, int t, double cost) const { return evs[vid].CanSlowCharge(t, cost); }
	bool EV_CanSlowChargeNow(size_t vid, double cost) const { return evs[vid].CanSlowCharge(getTime(), cost); }

	const Trip& EV_CurrentTrip(size_t vid) const { return evs[vid].CurrentTrip(); }
	const Trip& EV_TripAt(size_t vid, int idx) const { return evs[vid].TripAt(idx); }
	size_t EV_TripsCount(size_t vid) const { return evs[vid].TripsCount(); }
	int EV_TripID(size_t vid) const { return evs[vid].TripID(); }
	int EV_NextTrip(size_t vid) { return evs[vid].NextTrip(); }
	double EV_MaxMileage(size_t vid) { return evs[vid].MaxMileage(); }
	bool EV_IsBattEnough(size_t vid, double dist) const { return evs[vid].IsBattEnough(dist); }
	const string& EV_brief(size_t vid) const { return evs[vid].brief(); }


	vector<string> FCSList_Names() const { return fcs.CSIDs(); }
	int FCSList_IndexOf(const string& csName) const { return fcs.IndexOf(csName); }
	bool FCSList_AddVeh(int vid, const string& csName) { return fcs.AddVeh(vid, csName); }
	bool FCSList_AddVeh(int vid, int cs_index) { return fcs.AddVeh(vid, cs_index); }
	bool FCSList_HasVeh(int vid) const { return fcs.HasVeh(vid); }
	bool FCSList_PopVeh(int vid) { return fcs.PopVeh(vid); }
	bool FCSList_IsCharging(int vid) { return fcs.IsCharging(vid); }
	size_t FCSList_size() const { return fcs.size(); }
	vector<size_t> FCSList_VehCounts() const { return fcs.VehCounts(); }

	const string& FCS_getID(size_t cs_index) const { return fcs[cs_index].ID; }
	const string& FCS_getEdge(size_t cs_index) const { return fcs[cs_index].Edge; }

	int FCS_getSlots(size_t cs_index) const { return fcs[cs_index].Slots; }
	void FCS_setSlots(size_t cs_index, int slots) { fcs[cs_index].Slots = slots; }

	const string& FCS_getBus(size_t cs_index) const { return fcs[cs_index].Bus; }
	double FCS_getX(size_t cs_index) const { return fcs[cs_index].X; }
	double FCS_getY(size_t cs_index) const { return fcs[cs_index].Y; }

	const vector<double>& FCS_getSinglePcLimit(size_t cs_index) const { return fcs[cs_index].SinglePcLimit; }
	double FCS_getSinglePcLimit(size_t cs_index, size_t slot_index) const {
		if (slot_index >= fcs[cs_index].SinglePcLimit.size()) {
			throw V2SimError(std::format("FCS_setSinglePcLimit: slot_index {} out of bound, size is {}.", slot_index, fcs[cs_index].SinglePcLimit.size()));
		}
		return fcs[cs_index].SinglePcLimit[slot_index]; 
	}
	void FCS_setSinglePcLimit(size_t cs_index, size_t slot_index, double value) { 
		if (slot_index >= fcs[cs_index].SinglePcLimit.size()) {
			throw V2SimError(std::format("FCS_setSinglePcLimit: slot_index {} out of bound, size is {}.", slot_index, fcs[cs_index].SinglePcLimit.size()));
		}
		fcs[cs_index].SinglePcLimit[slot_index] = value; 
	}

	double FCS_getTotalPcLimit(size_t cs_index) const { return fcs[cs_index].TotalPcLimit; }
	void FCS_setTotalPcLimit(size_t cs_index, double tot_pc) { fcs[cs_index].TotalPcLimit = tot_pc; }

	const vector<double>& FCS_getSinglePdActual(size_t cs_index) const { return fcs[cs_index].SinglePdActual; }
	double FCS_getSinglePdActual(size_t cs_index, size_t slot_index) const {
		if (slot_index >= fcs[cs_index].SinglePdActual.size()) {
			throw V2SimError(std::format("FCS_setSinglePdActual: slot_index {} out of bound, size is {}.", slot_index, fcs[cs_index].SinglePdActual.size()));
		}
		return fcs[cs_index].SinglePdActual[slot_index];
	}
	void FCS_setSinglePdActual(size_t cs_index, size_t slot_index, double value) {
		if (slot_index >= fcs[cs_index].SinglePdActual.size()) {
			throw V2SimError(std::format("FCS_setSinglePdActual: slot_index {} out of bound, size is {}.", slot_index, fcs[cs_index].SinglePdActual.size()));
		}
		fcs[cs_index].SinglePdActual[slot_index] = value;
	}
	double FCS_getTotalPdLimit(size_t cs_index) const { return fcs[cs_index].TotalPdLimit; }
	void FCS_setTotalPdLimit(size_t cs_index, double tot_pd) { fcs[cs_index].TotalPdLimit = tot_pd; }

	double FCS_PriceBuy(size_t cs_index, int t) const { return fcs[cs_index].PriceBuy(t); }
	double FCS_PriceBuyNow(size_t cs_index) const { return fcs[cs_index].PriceBuy(getTime()); }

	double FCS_PriceSell(size_t cs_index, int t) const { return fcs[cs_index].PriceSell(t); }
	double FCS_PriceSellNow(size_t cs_index) const { return fcs[cs_index].PriceSell(getTime()); }

	bool FCS_SupportV2G(size_t cs_index) const { return fcs[cs_index].SupportV2G(); }
	bool FCS_IsOnline(size_t cs_index, int t) const { return fcs[cs_index].IsOnline(t); }
	bool FCS_IsOnlineNow(size_t cs_index) const { return fcs[cs_index].IsOnline(getTime()); }

	void FCS_ForceShutdown(size_t cs_index) { fcs[cs_index].ForceShutdown(); }
	void FCS_ForceReopen(size_t cs_index) { fcs[cs_index].ForceReopen(); }
	void FCS_ClearForceOffline(size_t cs_index) { fcs[cs_index].ClearForceOffline(); }

	double FCS_Pc(size_t cs_index) const { return fcs[cs_index].Pc(); }
	double FCS_Pc_kW(size_t cs_index) const { return fcs[cs_index].Pc_kW(); }
	double FCS_Pc_MW(size_t cs_index) const { return fcs[cs_index].Pc_MW(); }

	double FCS_Pd(size_t cs_index) const { return fcs[cs_index].Pd(); }
	double FCS_Pd_kW(size_t cs_index) const { return fcs[cs_index].Pd_kW(); }
	double FCS_Pd_MW(size_t cs_index) const { return fcs[cs_index].Pd_MW(); }
	
	double FCS_Pv2g(size_t cs_index) const { return fcs[cs_index].Pv2g(); }
	double FCS_Pv2g_kW(size_t cs_index) const { return fcs[cs_index].Pv2g_kW(); }
	double FCS_Pv2g_MW(size_t cs_index) const { return fcs[cs_index].Pv2g_MW(); }

	bool FCS_AddVeh(int cs_index, int vid) { return fcs.AddVeh(vid, cs_index); }
	bool FCS_PopVeh(size_t cs_index, int vid) { return fcs[cs_index].PopVeh(vid); }
	bool FCS_HasVeh(size_t cs_index, int vid) const { return fcs[cs_index].HasVeh(vid); }
	bool FCS_IsCharging(size_t cs_index, int vid) const { return fcs[cs_index].IsCharging(vid); }

	size_t FCS_size(size_t cs_index) const { return fcs[cs_index].size(); }
	size_t FCS_VehCount(size_t cs_index, bool only_charging = false) const { return fcs[cs_index].VehCount(only_charging); }

	vector<int> FCS_Update(size_t cs_index, int sec, int ctime, double v2g_k) {
		return fcs[cs_index].Update(evs, sec, ctime, v2g_k);
	}
	vector<int> FCS_UpdateNow(size_t cs_index, int sec, double v2g_k) {
		return fcs[cs_index].Update(evs, sec, getTime(), v2g_k);
	}
	double FCS_V2GCapacity(size_t cs_index, int ctime) { return fcs[cs_index].V2GCapacity(evs, ctime); }
	double FCS_V2GCapacityNow(size_t cs_index) { return fcs[cs_index].V2GCapacity(evs, getTime()); }
	double FCS_V2GCapBuffer(size_t cs_index) const { return fcs[cs_index].V2GCapBuffer(); }


	vector<string> SCSList_Names() const { return scs.CSIDs(); }
	int SCSList_IndexOf(const string& csName) const { return scs.IndexOf(csName); }
	bool SCSList_AddVeh(int vid, const string& csName) { return scs.AddVeh(vid, csName); }
	bool SCSList_AddVeh(int vid, int cs_index) { return scs.AddVeh(vid, cs_index); }
	bool SCSList_HasVeh(int vid) const { return scs.HasVeh(vid); }
	bool SCSList_PopVeh(int vid) { return scs.PopVeh(vid); }
	bool SCSList_IsCharging(int vid) { return scs.IsCharging(vid); }
	size_t SCSList_size() const { return scs.size(); }
	vector<size_t> SCSList_VehCounts() const { return scs.VehCounts(); }

	const string& SCS_getID(size_t cs_index) const { return scs[cs_index].ID; }
	const string& SCS_getEdge(size_t cs_index) const { return scs[cs_index].Edge; }

	double SCS_getTotalPcLimit(size_t cs_index) const { return scs[cs_index].TotalPcLimit; }
	void SCS_setTotalPcLimit(size_t cs_index, double tot_pc) { scs[cs_index].TotalPcLimit = tot_pc; }

	const vector<double>& SCS_getSinglePdActual(size_t cs_index) const { return scs[cs_index].SinglePdActual; }
	double SCS_getSinglePdActual(size_t cs_index, size_t slot_index) const {
		if (slot_index >= scs[cs_index].SinglePdActual.size()) {
			throw V2SimError(std::format("SCS_setSinglePdActual: slot_index {} out of bound, size is {}.", slot_index, scs[cs_index].SinglePdActual.size()));
		}
		return scs[cs_index].SinglePdActual[slot_index];
	}
	void SCS_setSinglePdActual(size_t cs_index, size_t slot_index, double value) {
		if (slot_index >= scs[cs_index].SinglePdActual.size()) {
			throw V2SimError(std::format("SCS_setSinglePdActual: slot_index {} out of bound, size is {}.", slot_index, scs[cs_index].SinglePdActual.size()));
		}
		scs[cs_index].SinglePdActual[slot_index] = value;
	}
	double SCS_getTotalPdLimit(size_t cs_index) const { return scs[cs_index].TotalPdLimit; }
	void SCS_setTotalPdLimit(size_t cs_index, double tot_pd) { scs[cs_index].TotalPdLimit = tot_pd; }

	double SCS_PriceBuy(size_t cs_index, int t) const { return scs[cs_index].PriceBuy(t); }
	double SCS_PriceBuyNow(size_t cs_index) const { return scs[cs_index].PriceBuy(getTime()); }

	double SCS_PriceSell(size_t cs_index, int t) const { return scs[cs_index].PriceSell(t); }
	double SCS_PriceSellNow(size_t cs_index) const { return scs[cs_index].PriceSell(getTime()); }

	bool SCS_SupportV2G(size_t cs_index) const { return scs[cs_index].SupportV2G(); }
	bool SCS_IsOnline(size_t cs_index, int t) const { return scs[cs_index].IsOnline(t); }
	bool SCS_IsOnlineNow(size_t cs_index) const { return scs[cs_index].IsOnline(getTime()); }

	void SCS_ForceShutdown(size_t cs_index) { scs[cs_index].ForceShutdown(); }
	void SCS_ForceReopen(size_t cs_index) { scs[cs_index].ForceReopen(); }
	void SCS_ClearForceOffline(size_t cs_index) { scs[cs_index].ClearForceOffline(); }

	double SCS_Pc(size_t cs_index) const { return scs[cs_index].Pc(); }
	double SCS_Pc_kW(size_t cs_index) const { return scs[cs_index].Pc_kW(); }
	double SCS_Pc_MW(size_t cs_index) const { return scs[cs_index].Pc_MW(); }

	double SCS_Pd(size_t cs_index) const { return scs[cs_index].Pd(); }
	double SCS_Pd_kW(size_t cs_index) const { return scs[cs_index].Pd_kW(); }
	double SCS_Pd_MW(size_t cs_index) const { return scs[cs_index].Pd_MW(); }

	double SCS_Pv2g(size_t cs_index) const { return scs[cs_index].Pv2g(); }
	double SCS_Pv2g_kW(size_t cs_index) const { return scs[cs_index].Pv2g_kW(); }
	double SCS_Pv2g_MW(size_t cs_index) const { return scs[cs_index].Pv2g_MW(); }

	bool SCS_AddVeh(int cs_index, int vid) { return scs.AddVeh(vid, cs_index); }
	bool SCS_PopVeh(size_t cs_index, int vid) { return scs[cs_index].PopVeh(vid); }
	bool SCS_HasVeh(size_t cs_index, int vid) const { return scs[cs_index].HasVeh(vid); }
	bool SCS_IsCharging(size_t cs_index, int vid) const { return scs[cs_index].IsCharging(vid); }

	size_t SCS_size(size_t cs_index) const { return scs[cs_index].size(); }
	size_t SCS_VehCount(size_t cs_index, bool only_charging = false) const { return scs[cs_index].VehCount(only_charging); }

	vector<int> SCS_Update(size_t cs_index, int sec, int ctime, double v2g_k) {
		return scs[cs_index].Update(evs, sec, ctime, v2g_k);
	}
	vector<int> SCS_UpdateNow(size_t cs_index, int sec, double v2g_k) {
		return scs[cs_index].Update(evs, sec, getTime(), v2g_k);
	}
	double SCS_V2GCapacity(size_t cs_index, int ctime) { return scs[cs_index].V2GCapacity(evs, ctime); }
	double SCS_V2GCapacityNow(size_t cs_index) { return scs[cs_index].V2GCapacity(evs, getTime()); }
	double SCS_V2GCapBuffer(size_t cs_index) const { return scs[cs_index].V2GCapBuffer(); }
};