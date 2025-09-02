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

	// ����������ɾ�������ڵ�
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

	// ��ռ����е�����Ԫ��
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

	// ������������������Ԫ�أ��������Ϊ���򷵻�std::nullopt��
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

	// ����Ԫ�� (����Ѵ����򷵻�false)
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

	// ɾ��Ԫ�� (����������򷵻�false)
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

	// ���Ԫ���Ƿ����
	bool contains(const T& value) const {
		return hash_map.find(value) != hash_map.end();
	}

	// ��ȡ��ǰԪ������
	size_t size() const {
		return hash_map.size();
	}

	// �ж��Ƿ�Ϊ��
	bool empty() const {
		return hash_map.empty();
	}

	// ��ȡ������˳�����е�����Ԫ��
	std::vector<T> getOrderedElements() const {
		std::vector<T> elements;
		Node* current = head;
		while (current) {
			elements.push_back(current->value);
			current = current->next;
		}
		return elements;
	}

	// ������֧��
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