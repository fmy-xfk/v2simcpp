#include "stat.h"

void StatItem::load() {
    if (fopen_s(&fh, fname.c_str(), "w") != 0) {
        throw runtime_error(std::format("Fail to open {}", fname));
    }
    _n = items.size();
    if (compress) {
        fputs("C\n", fh);
        for (size_t i = 0; i < _n; ++i) {
            if (i > 0) {
                fputc(',', fh);
            }
            fputs(items[i].c_str(), fh);
        }
        fputc('\n', fh);
        b62.reserve(_n);
        for (int i = 0; i < _n; ++i) {
            b62.emplace_back(to_base62(i));
        }
    }
    fprintf(fh, "Time,Item,Value\n");
}

void StatItem::recordItems(const V2SimCore& vc) {
    vector<double> this_items = getItems(vc);
    int t = vc.getTime();
    if (this_items.size() != _n) {
        throw runtime_error(format("Bad item length: get {}, but should be {}", this_items.size(), _n));
    }
    for (int i = 0; i < _n; ++i) {
        if (last_items.size() == 0 || fabs(this_items[i] - last_items[i]) > 0.5e-6) {
            if (t != last_t) { 
                fprintf_s(fh, "%d", t); 
                last_t = t;
            }
            if (compress) {
                fprintf_s(fh, ",%s,%.6f\n", b62[i].c_str(), this_items[i]);
            }
            else {
                fprintf_s(fh, ",%s,%.6f\n", items[i].c_str(), this_items[i]);
            }

        }
    }
    last_items = this_items;
}

static vector<string> SCS_ATTRS = { "cnt","c","d","v2g","pb","ps" };
static vector<string> FCS_ATTRS = { "cnt","c","pb" };
static vector<string> EV_ATTRS = { "soc", "status", "cost", "earn", "x", "y" };

StatFCS::StatFCS(const string& filename, const vector<string>& csnames, bool _compress)
    : StatItem(filename, cross_list(csnames, FCS_ATTRS), _compress) {
}

vector<double> StatFCS::getItems(const V2SimCore& vc) {
    vector<double> ret;
    ret.reserve(_n);
    int t = vc.getTime();
    for (auto& cs : vc.FCSs()) {
        ret.emplace_back((double)cs.VehCount());
        ret.emplace_back(cs.Pc_kW());
        ret.emplace_back(cs.PriceBuy(t));
    }
    return ret;
}

StatSCS::StatSCS(const string& filename, const vector<string>& csnames, bool _compress)
    : StatItem(filename, cross_list(csnames, SCS_ATTRS), _compress) {
}

vector<double> StatSCS::getItems(const V2SimCore& vc) {
    vector<double> ret;
    ret.reserve(_n);
    int t = vc.getTime();
    for (auto& cs : vc.SCSs()) {
        ret.emplace_back((double)cs.VehCount());
        ret.emplace_back(cs.Pc_kW());
        ret.emplace_back(cs.Pd_kW());
        ret.emplace_back(cs.V2GCapBuffer());
        ret.emplace_back(cs.PriceBuy(t));
        ret.emplace_back(cs.PriceSell(t));
    }
    return ret;
}

StatEV::StatEV(const string& filename, const vector<string>& csnames, bool _compress)
    : StatItem(filename, cross_list(csnames, EV_ATTRS), _compress) {
}

vector<double> StatEV::getItems(const V2SimCore& vc) {
    vector<double> ret;
    ret.reserve(_n);
    int t = vc.getTime();
    for (auto& v : vc.EVs()) {
        ret.emplace_back(v.SoC());
        ret.emplace_back((double)((int)v.Status));
        ret.emplace_back(v.Cost);
        ret.emplace_back(v.Revenue);
        if (v.Status == VehStatus::Driving) {
            auto pos = libsumo::Vehicle::getPosition(v.ID);
            ret.emplace_back(pos.x);
            ret.emplace_back(pos.y);
        }
    }
    return ret;
}