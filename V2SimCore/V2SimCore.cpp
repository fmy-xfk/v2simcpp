// V2SimCore.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <libsumo/libsumo.h>

using namespace libsumo;

int main(int argc, char* argv[]) {
    std::cout << Simulation::hasGUI();
    Simulation::start({ "sumo", "-n", "D:\\etst.net.xml" });
    for (int i = 0; i < 5; i++) {
        Simulation::step();
    }
    Simulation::close();
}