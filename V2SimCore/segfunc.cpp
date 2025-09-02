#include "segfunc.h"

void SegFunc::check() {
	if (loop_period < 0) {
		throw V2SimError(std::format("Invalid loop period: {}", loop_period));
	}
	if (loop_times < -1 || loop_times == 0) {
		throw V2SimError(std::format("Invalid loop times: {}", loop_times));
	}
	if (loop_period > 0 && tl.back() > loop_period) {
		throw V2SimError(std::format(
			"SegFunc's last time ({}) exceeds loop period ({}).", tl.back(), loop_period));
	}
	for (auto& e : tl) {
		if (e < 0) {
			throw V2SimError(std::format(
				"Time ({}) must be non-negative for SegFunc.", e));
		}
	}
	size_t n = d.size();
	for (size_t i = 1; i < n; ++i) {
		if (tl[i - 1] >= tl[i]) {
			throw V2SimError(std::format("Time ({} and {}) must be increasing.", tl[i - 1], tl[i]));
		}
	}
}

SegFunc::SegFunc(tinyxml2::XMLElement* e, bool allow_null, 
	const char* tag, const char* time_attr, const char* val_attr) : 
	loop_period(0), loop_times(1)
{
	if (!e) {
		if (allow_null) {
			return;
		}
		throw V2SimError("SegFunc element is NULL");
	}
	for (auto* i = e->FirstChildElement(tag); i != NULL; i = i->NextSiblingElement(tag)) {
		int t = i->IntAttribute(time_attr, -1);
		if (t < 0) {
			throw V2SimError(std::format("Invalid SegFunc item on line {}: Time attribute '{}' not found or invalid.", i->GetLineNum(), time_attr));
		}
		double v = 0;
		if (i->QueryDoubleAttribute(val_attr, &v) != tinyxml2::XML_SUCCESS) {
			throw V2SimError(std::format("Invalid SegFunc item on line {}: Value attribute '{}' not found or invalid.", i->GetLineNum(), val_attr));
		}
		tl.emplace_back(t);
		d.emplace_back(v);
	}
	check();
}

double SegFunc::Get(int time) const {
	if (loop_period > 0) {
		if (loop_times > 0 && time > loop_period * loop_times) {
			return false;
		}
		time %= loop_period;
	}
	if (tl.size() == 0) return 0;
	if (time < tl[0]) {
		//throw V2SimError(std::format("The beginning time of the SegFunc is {}, but time {} is queried", tl[0], time));
		return 0;
	}
	return d[distance(tl.begin(), upper_bound(tl.begin(), tl.end(), time)) - 1];
}

SegFunc QuickSum(const vector<SegFunc>& funcs) {
	if (funcs.empty()) {
		return SegFunc();
	}

	// ������к��������ں�ѭ�������Ƿ�һ��
	int period = funcs[0].GetPeriod();
	int times = funcs[0].GetRepeatTimes();
	for (const auto& f : funcs) {
		if (f.GetPeriod() != period || f.GetRepeatTimes() != times) {
			throw V2SimError("All SegFuncs must have the same loop_period and loop_times in QuickSum.");
		}
	}

	// ʹ�����ȼ����д����¼���
	using Event = pair<int, pair<size_t, size_t>>; // (time, (func_index, point_index))
	priority_queue<Event, vector<Event>, greater<Event>> pq;

	// ��ʼ�����ȼ����У����ÿ�������ĵ�һ����Чʱ���
	for (size_t i = 0; i < funcs.size(); ++i) {
		if (funcs[i].size() > 0) {
			pq.emplace(funcs[i].TimeLine(0), make_pair(i, 0));
		}
	}

	if (pq.empty()) {
		return SegFunc();
	}

	vector<int> result_tl;
	vector<double> result_data;

	// ��ǰʱ��͵�ǰ�ܺ�
	int current_time = 0;
	double current_sum = double();

	// ����0ʱ�̵���һ�������֮���ֵ��Ϊ0�����
	int first_time = pq.top().first;
	if (first_time > 0) {
		result_tl.push_back(0);
		result_data.push_back(double());
		current_time = 0;
	}

	while (!pq.empty()) {
		Event event = pq.top();
		pq.pop();

		int event_time = event.first;
		size_t func_idx = event.second.first;
		size_t point_idx = event.second.second;

		// ���ʱ���仯����¼�µĶ�
		if (event_time > current_time) {
			result_tl.push_back(current_time);
			result_data.push_back(current_sum);
			current_time = event_time;
		}

		// ���µ�ǰ�ܺͣ���ȥ��ֵ��������ֵ
		if (point_idx > 0) {
			current_sum -= funcs[func_idx].Data((int)point_idx - 1);
		}
		current_sum += funcs[func_idx].Data(point_idx);

		// ����������������һ���㣬�������
		if (point_idx + 1 < funcs[func_idx].size()) {
			pq.emplace(funcs[func_idx].TimeLine(point_idx + 1),
				make_pair(func_idx, point_idx + 1));
		}
	}

	// ������һ����
	if (!result_tl.empty() || !result_data.empty()) {
		result_tl.push_back(current_time);
		result_data.push_back(current_sum);
	}

	return SegFunc(result_tl, std::move(result_data), period, times);
}