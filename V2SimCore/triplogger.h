#pragma once
#include <cstdio>
#include <initializer_list>
#include <string>
#include "ev.h"

constexpr int ARRIVAL_NO_CHARGE = 0;
constexpr int ARRIVAL_CHARGE_SUCCESSFULLY = 1;
constexpr int ARRIVAL_CHARGE_FAILED = 2;

class TripsLogger {
private:
    FILE* fh;
public:
    enum ArrivalStatus {
        ARRIVAL_NO_CHARGE = 0,
        ARRIVAL_CHARGE_SUCCESSFULLY = 1,
        ARRIVAL_CHARGE_FAILED = 2
    };

    TripsLogger(const char* file_name) {
        if (fopen_s(&fh, file_name, "w") != 0) {
            throw std::runtime_error(std::format("Failed to open log file: {}", file_name));
        }
    }

    ~TripsLogger() {
        if (fh) {
            fclose(fh);
            fh = NULL;
        }
    }

    void pr(std::initializer_list<std::string> args) {
        bool first = true;
        for (const auto& arg : args) {
            if (!first) {
                fputc('|', fh);
            }
            fputs(arg.c_str(), fh);
            first = false;
        }
        fputc('\n', fh);
    }

    void arrive(int simT, const EV& veh, ArrivalStatus status);
    void arrive_FCS(int simT, const EV& veh, const std::string& cs);
    void depart(int simT, const EV& veh, int delay = 0, const std::optional<std::string>& cs = std::nullopt);
    void depart_delay(int simT, const EV& veh, double batt_req, int delay);
    void depart_FCS(int simT, const EV& veh, const std::string& cs);
    void depart_failed(int simT, const EV& veh, double batt_req, const std::string& cs, int trT);
    void fault_deplete(int simT, const EV& veh, const std::string& cs, int trT);
    void fault_nocharge(int simT, const EV& veh, const std::string& cs);
    void fault_redirect(int simT, const EV& veh, const std::string& cs_old, const std::string& cs_new);
    void warn_smallcap(int simT, const EV& veh, double batt_req);
	void join_SCS(int simT, const EV& veh, const std::string& cs);
	void leave_SCS(int simT, const EV& veh, const std::string& cs);
    void close() { fclose(fh); }
};