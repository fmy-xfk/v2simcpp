// V2SimApp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <chrono>
#include <filesystem>
#include <iostream>
#include "utils.h"
#include "..\V2SimCore\v2sim.h"

namespace fs = std::filesystem;

inline int GetCurrentUnixTime() {
    return static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

int main(int argc, char *argv[])
{
    ArgParser args(argc, argv);
	string caseDir = args.GetStr("d");
	int start = args.GetInt("b", 0);
	int end = args.GetInt("e", 172800);
	int step = args.GetInt("s", 10);

	auto root = fs::absolute(caseDir);
    cout << "Case directory: " << root.string() << endl;
	
    // Find files
	string netfile, vehfile, fcsfile, scsfile;
    for (const auto & fn : fs::directory_iterator(root)) {
        const auto & fp = fn.path().string();
        if (fp.ends_with(".net.xml")) {
            netfile = fp;
            cout << "Roadnet file: " << fn.path().filename() << endl;
        }
        else if(fp.ends_with(".veh.xml")) {
            vehfile = fp;
            cout << "Vehicle file: " << fn.path().filename() << endl;
        }
        else if(fp.ends_with(".fcs.xml")) {
            fcsfile = fp;
			cout << "Fast charging station file: " << fn.path().filename() << endl;
        }
        else if(fp.ends_with(".scs.xml")) {
            scsfile = fp;
			cout << "Slow charging station file: " << fn.path().filename() << endl;
		}
    }

    if (netfile.empty()) {
        throw V2SimAppError("Roadnet file (*.net.xml) not found!");
	}
    if (vehfile.empty()) {
        throw V2SimAppError("Vehicle file (*.veh.xml) not found!");
	}
    if (fcsfile.empty()) {
        throw V2SimAppError("Fast charging station file (*.fcs.xml) not found!");
    }
    if (scsfile.empty()) {
        throw V2SimAppError("Slow charging station file (*.scs.xml) not found!");
	}

    // Create result directory
    auto resdir = root / "result";
    fs::create_directories(resdir);
    cout << "Result directory: " << resdir.string() << endl;


    V2SimInterface vc(start, end, step, 
        netfile,
        vehfile,
        fcsfile,
        scsfile,
		resdir.string()
    );
    vc.Start();
    int lastT = 0, tbeg = GetCurrentUnixTime();
    
    while (vc.getTime() < vc.getEndTime()) {
        int t = GetCurrentUnixTime();
        if (t - lastT >= 1) {
            cout << "\r" << vc.getTime() << "/" << vc.getEndTime() << "  " << (t - tbeg) << "s";
            lastT = t;
        }
        vc.Step();
    }
    cout << "\rFinished. " << GetCurrentUnixTime() - tbeg << "s            " << endl;
    vc.Stop();
    return 0;
}