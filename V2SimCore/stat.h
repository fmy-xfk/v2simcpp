#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include "core.h"
using namespace std;

class StatItem {
private:
    bool compress;
    FILE* fh = nullptr;
    string fname;
    vector<string> items;
    vector<double> last_items;
    vector<string> b62;
    int last_t = -1;
protected:
    size_t _n;
    void load();
public:
    StatItem(const string& filename, const vector<string>& items, bool _compress) : fname(filename), items(items), compress(_compress) {
        load();
    }
    StatItem(const string& filename, const vector<string>&& items, bool _compress) : fname(filename), items(items), compress(_compress) {
        load();
    }
    virtual vector<double> getItems(const V2SimCore& vc) = 0;

    void recordItems(const V2SimCore& vc);

    void close() {
        if (fh) {
            fclose(fh);
            fh = nullptr;
        }
    }

    ~StatItem() {
        close();
    }
};


class StatFCS : public StatItem {  
public:  
    StatFCS(const string& filename, const vector<string>& csnames, bool _compress);
    vector<double> getItems(const V2SimCore& vc) override;
};


class StatSCS : public StatItem {
public:
    StatSCS(const string& filename, const vector<string>& csnames, bool _compress);
    vector<double> getItems(const V2SimCore& vc) override;
};


class StatEV : public StatItem {
public:
    StatEV(const string& filename, const vector<string>& evnames, bool _compress);
    vector<double> getItems(const V2SimCore& vc) override;
};