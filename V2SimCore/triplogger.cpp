#include "triplogger.h"

void TripsLogger::arrive(int simT, const EV& veh, ArrivalStatus status) {
    int tid = veh.TripID();
    string nt("None");
    if (tid < veh.TripsCount() - 1) {
        nt = veh.TripAt(tid + 1).__repr__();
    }
    fprintf_s(fh, "%d|A|%s|%d|%s|%s\n",
        simT, veh.brief().c_str(), static_cast<int>(status), veh.CurrentTrip().ToEdge().c_str(), nt.c_str());
}

void TripsLogger::arrive_FCS(int simT, const EV& veh, const std::string& cs) {
	fprintf_s(fh, "%d|AC|%s|%s\n", simT, veh.brief().c_str(), cs.c_str());
}

void TripsLogger::depart(int simT, const EV& veh, int delay, const std::optional<std::string>& cs) {
    fprintf_s(fh, "%d|D|%s|%s|%d|%s|cpp_not_support\n",
        simT, veh.brief().c_str(), veh.CurrentTrip().__repr__().c_str(),
		delay, cs.value_or("None").c_str());
}

void TripsLogger::depart_delay(int simT, const EV& veh, double batt_req, int delay)
{
    fprintf_s(fh, "%d|DD|%s|%lf|%lf|%d\n",
		simT, veh.brief().c_str(), veh.BattElec, batt_req, delay);
}

void TripsLogger::depart_FCS(int simT, const EV& veh, const std::string& cs)
{
	fprintf_s(fh, "%d|DC|%s|%s|%s\n", simT, veh.brief().c_str(), cs.c_str(), veh.CurrentTrip().ToEdge().c_str());
}

void TripsLogger::depart_failed(int simT, const EV& veh, double batt_req, const std::string& cs, int trT) {
    fprintf_s(fh, "%d|DF|%s|%lf|%lf|%s|%d\n",
		simT, veh.brief().c_str(), veh.BattElec, batt_req, cs.c_str(), trT);
}

void TripsLogger::fault_deplete(int simT, const EV& veh, const std::string& cs, int trT) {
	fprintf_s(fh, "%d|FD|%s|%s|%d\n", simT, veh.brief().c_str(), cs.c_str(), trT);
}

void TripsLogger::fault_nocharge(int simT, const EV& veh, const std::string& cs) {
    fprintf_s(fh, "%d|FN|%s|%lf|%s\n",
		simT, veh.brief().c_str(), veh.BattElec, cs.c_str());
}

void TripsLogger::fault_redirect(int simT, const EV& veh, const std::string& cs_old, const std::string& cs_new) {
	fprintf_s(fh, "%d|FR|%s|%lf|%s|%s\n", simT, veh.brief().c_str(), veh.BattElec, cs_old.c_str(), cs_new.c_str());
}

void TripsLogger::warn_smallcap(int simT, const EV& veh, double batt_req) {
    fprintf_s(fh, "%d|WC|%s|%lf|%lf\n", simT, veh.brief().c_str(), veh.BattElec, batt_req);
}

void TripsLogger::join_SCS(int simT, const EV& veh, const std::string& cs) {
    fprintf_s(fh, "%d|SC|%s|%s\n", simT, veh.brief().c_str(), cs.c_str());
}

void TripsLogger::leave_SCS(int simT, const EV& veh, const std::string& cs) {
    fprintf_s(fh, "%d|SC|%s|%s\n", simT, veh.brief().c_str(), cs.c_str());
}