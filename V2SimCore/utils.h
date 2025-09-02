#pragma once

#include <iostream>
#include <unordered_set>
#include "tinyxml2.h"
#include "segfunc.h"
#include "kdtree.h"
#include "orderedset.h"
using namespace std;

vector<string> cross_list(const vector<string>& a, const vector<string>& b);
string to_base62(int num);