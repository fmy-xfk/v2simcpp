#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <format>
using namespace std;

class V2SimAppError : public exception {
private:
	string message;
public:
	V2SimAppError(const string& msg) : message(msg)
	{

	}
	V2SimAppError(const string&& msg) : message(msg)
	{

	}
	const char* what() const throw ()
	{
		return message.c_str();
	}
};

class ArgParser {
	const string progname;
	unordered_map<string, string> dict;
	unordered_set<string> opts;
public:
	ArgParser(int argc, char *argv[]) : progname(argv[0]) {
		for (int i = 1; i < argc; ++i) {
			string itm(argv[i]);
			auto eqpos = itm.find_first_of('=');
			auto keystart = itm.find_first_not_of('-');
			if (keystart == 0) {
				throw V2SimAppError("ArgParser: Key must start with at least one '-'.");
			}
			if (keystart >= eqpos) {
				throw V2SimAppError(std::format("ArgParser: Key name not found in {}", itm));
			}
			string key = itm.substr(keystart, eqpos - keystart);
			if (eqpos == string::npos) {
				opts.insert(std::move(key));
			} else {
				dict[key] = itm.substr(eqpos + 1);
			}
		}
	}
	string GetStr(const string& key) {
		if (dict.contains(key)) {
			return dict[key];
		}
		else {
			throw V2SimAppError(std::format("Argparser: Key {} not found", key));
		}
	}
	string GetStr(const string& key, const string& def) {
		return dict.contains(key) ? dict[key] : def;
	}
	int GetInt(const string& key) {
		return stoi(GetStr(key));
	}
	int GetInt(const string& key, int def) {
		return dict.contains(key) ? stoi(dict[key]) : def;
	}
	double GetDouble(const string& key) {
		return stod(GetStr(key));
	}
	double GetDouble(const string& key, double def) {
		return dict.contains(key) ? stod(dict[key]) : def;
	}
	bool HasOpt(const string& opt) { return opts.contains(opt); }
	void print() {
		cout << "Dict:" << endl;
		for (auto& kvp : dict) {
			cout << "  " << kvp.first << " = " << kvp.second << endl;
		}
		cout << "Options:" << endl;
		for (auto& s : opts) {
			cout << s << ",";
		}
	}
};