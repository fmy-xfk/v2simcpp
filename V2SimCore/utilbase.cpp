#include "utilbase.h"

void AddVehToSUMO(const string& name, const string& from_edge, const string& to_edge) {
	try {
		libsumo::Vehicle::add(name, "");
		libsumo::Vehicle::setRoute(name, { from_edge });
		libsumo::Vehicle::setRoutingMode(name, libsumo::ROUTING_MODE_AGGREGATED);
		libsumo::Vehicle::changeTarget(name, to_edge);
	}
	catch (libsumo::TraCIException e) {
		cerr << e.what() << endl;
		throw e;
	}
	
}
