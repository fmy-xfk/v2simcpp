#pragma once

#include "cs.h"
#include "triplogger.h"
using namespace std;

template<typename T, typename = typename enable_if_t<is_base_of_v<EVCS, T>>>
class CSMap {
protected:
	vector<T> cs;
	vector<string> cs_names;
	unordered_map<string, size_t> csmp; // CS ID -> CS index at the vector
	unordered_map<int, size_t> vmp; // Vehicle index -> CS index
	KDTree tr;
	CSMap(CSMap<T>&) = delete;
	CSMap<T>& operator=(CSMap<T>&) = delete;
	
	void create_map() {
		int i = 0;
		for (auto& c : cs) {
			csmp[c.ID] = i;
			cs_names.push_back(c.ID);
			++i;
		}
	}
public:
	bool TreeInitialized() const {
		return tr.Initialized();
	}
	void UpdateTree() {
		int i = 0;
		vector<Point> pts;
		pts.reserve(cs.size());
		for (auto& c : cs) {
			pts.emplace_back(Point(c.X, c.Y, i));
			++i;
		}
		tr.Init(std::move(pts));
	}
	auto begin() { return cs.begin(); }
	auto end() { return cs.end(); }

	auto begin() const { return cs.begin(); }
	auto end() const { return cs.end(); }

	CSMap(const char* filename, const char* tag) {
		bool has_inf = false;
		tinyxml2::XMLDocument doc;
		if (doc.LoadFile(filename) != tinyxml2::XML_SUCCESS) {
			throw V2SimError(std::format("Fail to load CS xml {}.", filename));
		}
		auto* root = doc.RootElement();
		for (tinyxml2::XMLElement* e = root->FirstChildElement(tag); e; e = e->NextSiblingElement(tag)) {
			T c0 = T(e);
			if (isinf(c0.X) || isinf(c0.Y)) {
				has_inf = true;
			}
			cs.emplace_back(std::move(c0));
		}
		create_map();
		if(!has_inf) UpdateTree();
	}
	CSMap(vector<T>&& cs_list):cs(cs_list) {
		bool has_inf = false;
		for (auto& c : cs) {
			if (isinf(c.X) || isinf(c.Y)) {
				has_inf = true;
			}
		}
		create_map();
		if (!has_inf) UpdateTree();
	}
	T& operator[](size_t idx) {
		try {
			return cs.at(idx);
		} catch (exception e) {
			throw V2SimError(std::format("Out of bound: {} >= {}", idx, cs.size()));
		}
	}
	const T& operator[](size_t idx) const {
		try {
			return cs.at(idx);
		}
		catch (exception e) {
			throw V2SimError(std::format("Out of bound: {} >= {}", idx, cs.size()));
		}
	}
	const vector<string>& CSIDs() const {
		return cs_names;
	}	
	std::optional<vector<Point>> SelectNear(double x, double y, int n = -1) {
		if (n == -1 || n >= cs.size() || !tr.Initialized()) {
			return std::nullopt;
		}
		return tr.findKNearestNeighbors(Point(x, y, 0), n);
	}
	int IndexOf(const string& ID) const {
		auto it = csmp.find(ID);
		if (it == csmp.end()) {
			return -1;
		}
		return (int)it->second;
	}
	T& Get(const string& ID) {
		int i = IndexOf(ID);
		if (i < 0) {
			throw V2SimError(std::format("EVCS {} not found.", ID));
		}
		return cs.at(i);
	}
	virtual bool AddVeh(int vid, const string& csID) {
		int i = IndexOf(csID);
		if (i < 0) return false;
		return AddVeh(vid, i);
	}
	virtual bool AddVeh(int vid, int csID) {
		T& cs = this->cs.at(csID);
		if (cs.AddVeh(vid)) {
			vmp[vid] = csID;
			return true;
		}
		return false;
	}
	bool HasVeh(int vid) const {
		return vmp.contains(vid);
	}
	virtual bool PopVeh(int vid) {
		auto pos = vmp.find(vid);
		if (pos != vmp.end()) {
			cs[pos->second].PopVeh(vid);
			vmp.erase(pos);
			return true;
		}
		return false;
	}
	virtual bool IsCharging(int vid) {
		if (vmp.contains(vid)) {
			return cs[vmp[vid]].IsCharging(vid);
		}
		return false;
	}
	size_t size() const noexcept{
		return cs.size();
	}
	Point FindNearestCS(double x, double y) {
		if (!tr.Initialized()) {
			throw V2SimError("CSMap not initialized.");
		}
		return tr.findNearestNeighbor({ x,y,0 }).label;
	}
	vector<size_t> VehCounts() const {
		vector<size_t> ret;
		ret.reserve(cs.size());
		for (auto& c : cs) {
			ret.push_back(c.VehCount());
		}
		return ret;
	}
};

class FastCSMap : public CSMap<FastCS> {
public:
	FastCSMap(vector<FastCS>&& cs_list) : 
		CSMap<FastCS>(std::move(cs_list)){
	}
	FastCSMap(const char* filename, const char* tag) :
		CSMap<FastCS>(filename, tag) {}
	void Update(EVMap& mp, int sec, int ctime, TripsLogger* tlog);
};

class SlowCSMap :public CSMap<SlowCS>{
	int v2g_cap_res_time = -1;
	vector<double> v2g_cap_res;
	vector<double> v2g_demand;
	vector<double> v2g_k;
	void init() {
		const size_t n = this->cs.size();
		v2g_cap_res.assign(n, 0);
		v2g_demand.assign(n, 0);
		v2g_k.assign(n, 0);
	}
public:
	SlowCSMap(vector<SlowCS>&& cs_list) : 
		CSMap<SlowCS>(std::move(cs_list)){
		init();
	}
	SlowCSMap(const char* filename, const char* tag) :
		CSMap<SlowCS>(filename, tag) {
		init();
	}
	void UpdateV2GCapacities(EVMap& mp, int t);
	vector<double>& V2GCapacities(EVMap& mp, int t) {
		UpdateV2GCapacities(mp, t);
		return v2g_cap_res;
	}
	void SetV2GDemand(int cs_idx, double demand) {
		v2g_demand[cs_idx] = demand;
	}
	void ClearV2GDemand();
	void Update(EVMap& mp, int sec, int ctime, TripsLogger* tlog);
};