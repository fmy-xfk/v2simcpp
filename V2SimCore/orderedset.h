#pragma once

#include "utilbase.h"

template <typename T>
class OrderedHashSet {
private:
	struct Node {
		T value;
		Node* prev;
		Node* next;
		Node(const T& val) : value(val), prev(nullptr), next(nullptr) {}
	};

	std::unordered_map<T, Node*> hash_map;
	Node* head;
	Node* tail;

	// 辅助函数：删除单个节点
	void deleteNode(Node* node) {
		if (node->prev) {
			node->prev->next = node->next;
		}
		else {
			head = node->next;
		}

		if (node->next) {
			node->next->prev = node->prev;
		}
		else {
			tail = node->prev;
		}

		delete node;
	}

public:
	OrderedHashSet() : head(nullptr), tail(nullptr) {}

	~OrderedHashSet() {
		clear();
	}

	// 清空集合中的所有元素
	void clear() {
		Node* current = head;
		while (current) {
			Node* next = current->next;
			delete current;
			current = next;
		}
		hash_map.clear();
		head = tail = nullptr;
	}

	// 弹出并返回最早插入的元素（如果集合为空则返回std::nullopt）
	std::optional<T> pop() {
		if (!head) {
			return std::nullopt;
		}

		T value = head->value;
		Node* toDelete = head;
		hash_map.erase(value);
		deleteNode(toDelete);
		return value;
	}

	// 插入元素 (如果已存在则返回false)
	bool insert(const T& value) {
		if (hash_map.find(value) != hash_map.end()) {
			return false;
		}

		Node* new_node = new Node(value);
		hash_map[value] = new_node;

		if (!head) {
			head = tail = new_node;
		}
		else {
			tail->next = new_node;
			new_node->prev = tail;
			tail = new_node;
		}

		return true;
	}

	// 删除元素 (如果不存在则返回false)
	bool erase(const T& value) {
		auto it = hash_map.find(value);
		if (it == hash_map.end()) {
			return false;
		}

		Node* node = it->second;
		deleteNode(node);
		hash_map.erase(it);
		return true;
	}

	// 检查元素是否存在
	bool contains(const T& value) const {
		return hash_map.find(value) != hash_map.end();
	}

	// 获取当前元素数量
	size_t size() const {
		return hash_map.size();
	}

	// 判断是否为空
	bool empty() const {
		return hash_map.empty();
	}

	// 获取按插入顺序排列的所有元素
	std::vector<T> getOrderedElements() const {
		std::vector<T> elements;
		Node* current = head;
		while (current) {
			elements.push_back(current->value);
			current = current->next;
		}
		return elements;
	}

	// 迭代器支持
	class iterator {
	private:
		Node* current;
	public:
		iterator(Node* node) : current(node) {}

		T& operator*() { return current->value; }
		iterator& operator++() { current = current->next; return *this; }
		bool operator!=(const iterator& other) { return current != other.current; }
	};

	iterator begin() { return iterator(head); }
	iterator end() { return iterator(nullptr); }
};