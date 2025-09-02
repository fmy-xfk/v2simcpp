#include <iostream>
#include "..\V2SimCore\v2sim.h"

int kdtree() {
    // Example usage
    std::vector<Point> points = {
        {2, 3, 1},
        {5, 4, 2},
        {9, 6, 3},
        {4, 7, 4},
        {8, 1, 5},
        {7, 2, 6}
    };

    // Build the KDTree
    KDTree tree(points);

    // Test nearest neighbor search
    Point query(6, 3, 0); // Label doesn't matter for the query
    Point nearest = tree.findNearestNeighbor(query);
    std::cout << "Nearest neighbor: (" << nearest.x << ", " << nearest.y
        << ") with label " << nearest.label << std::endl;

    // Test k nearest neighbors search
    int k = 3;
    std::vector<Point> kNearest = tree.findKNearestNeighbors(query, k);
    std::cout << k << " nearest neighbors:" << std::endl;
    for (const auto& p : kNearest) {
        std::cout << "(" << p.x << ", " << p.y << ") with label " << p.label << std::endl;
    }

    return 0;
}

// 示例用法
int ohs() {
    OrderedHashSet<int> oset;

    // 插入元素
    oset.insert(3);
    oset.insert(1);
    oset.insert(4);
    oset.insert(1); // 重复插入，不会成功

    // 检查存在性
    std::cout << "Contains 4: " << oset.contains(4) << std::endl;
    std::cout << "Contains 2: " << oset.contains(2) << std::endl;

    // 遍历元素 (按插入顺序)
    std::cout << "Elements before clear: ";
    for (const auto& elem : oset) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // 清空集合
    oset.clear();
    std::cout << "After clear, size: " << oset.size() << std::endl;

    // 再次插入新元素
    oset.insert(5);
    oset.insert(2);

    // 遍历清空后重新插入的元素
    std::cout << "Elements after clear and reinsert: ";
    for (const auto& elem : oset) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    return 0;
}

int rglist() {
    try {
        RangeList rl({ {2,3}, {1,1} });
        std::cout << rl.Contains(3) << std::endl;
        std::cout << rl.Contains(2) << std::endl;
    }
    catch (V2SimError& e) {
        std::cout << e.what() << endl;
    }
    return 0;
}

int segf() {
    try {
        SegFunc sf({ {1,0.8},{3,3.5},{5,3.6} });
        SegFunc sf2({ {0,0.6},{3,3.1},{4,3.9} });
        auto sf3 = QuickSum({ sf, sf2 });
        std::cout << sf(1) << std::endl;
        for (int i = 0; i < sf3.size(); ++i) {
            std::cout << sf3.TimeLine(i) << " " << sf3.Data(i) << std::endl;
        }
    }
    catch (V2SimError& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}


void inst() {
    Trip t("trip1", 1400, "TAZ1", "TAZ3", "Edge_29to30 Edge_10to11");
    std::cout << t.ID << " " << t.DepartTime << " " << t.FromTAZ << " " << t.ToTAZ << " " << t.FixedRoute << std::endl;
    Trip t2("trip2", 14000, "TAZ3", "TAZ1", "Edge_10to11 Edge_29to30");
    EV ev("v0", { t,t2 }, 0.9, 0.9, 100, 0.6, 300, 70.0, 8.0, 10.0, 8, 1.05, 0.25, 0.5, 0.7, "Equal", RangeList(true), 0.8, RangeList(true), 0.9, false);
    std::cout << ev.CanSlowCharge(0, 0.5) << std::endl;
    std::cout << ev.CanV2G(0, 1.2) << std::endl;
    std::cout << ev.PcFast_kW() << " " << ev.SoC() << " " << ev.Charge(60, 0.9, ev.PcFast) << " " << ev.SoC() << " " << ev.Pc_kW() << std::endl;
    EVMap evmp;
    evmp.Add(std::move(ev));
    std::cout << evmp.Get("v0").SoC() << std::endl;
    //cout << evmp.Get("v1").SoC() << endl;
    FastCS fcs("Edge_10to11", "Edge_10to11", 10, "B1", 100., 100., RangeList(false), 5000, SegFunc({{0, 1.0}}));
    FastCSMap fcsm({ std::move(fcs) });
    SlowCS scs("Edge_29to30", "Edge_29to30", 10, "B2", 105., 105., RangeList(false), 5000, 100, SegFunc({{0, 1.0}}), SegFunc({{0, 1.5}}), "");
    SlowCSMap scsm({ std::move(scs) });
    V2SimCore core(0, 172800, 10, "D:\\test\\roadnet_raw.net.xml", evmp, fcsm, scsm, nullptr);
    try {
        core.Start();
    }
    catch (libsumo::TraCIException e) {
        std::cout << e.what() << std::endl;
        return;
    }
    while (core.getTime() < core.getEndTime()) {
        core.Step();
        std::cout << core.getTime() << std::endl;
    }
    core.Stop();
}