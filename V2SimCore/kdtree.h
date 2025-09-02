#pragma once

#include "utilbase.h"

// Structure to represent a 2D point with a label
struct Point {
    double x, y;
    int label;

    Point(double x = 0, double y = 0, int label = 0) : x(x), y(y), label(label) {}

    double dist_to(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return dx * dx + dy * dy;
    }
};

// Structure to represent a node in the KDTree
struct KDNode {
    Point point;
    KDNode* left;
    KDNode* right;

    KDNode(const Point& point) : point(point), left(nullptr), right(nullptr) {}
};


// KDTree class
class KDTree {
private:
    KDNode* root;

    // Helper function to build the tree recursively
    KDNode* buildTree(std::vector<Point>& points, int depth = 0);

    // Helper function for nearest neighbor search
    void nearestNeighbor(KDNode* node, const Point& target, 
        int depth, KDNode*& best, double& bestDist);
        
    // Helper function for k nearest neighbors search
    void kNearestNeighbors(KDNode* node, const Point& target, int depth,
        std::priority_queue<std::pair<double, KDNode*>>& pq, int k);

    // Helper function to delete the tree recursively
    void deleteTree(KDNode* node) {
        if (node == nullptr) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

    KDTree(KDTree&) = delete;
    KDTree& operator=(KDTree&) = delete;

public:
	bool Initialized() const { return root != nullptr; }

    KDTree() {
        root = nullptr;
    }
    // Constructor builds the tree from a vector of points
    KDTree(const std::vector<Point>& points) {
        if (points.empty()) {
            root = nullptr;
            return;
        }

        std::vector<Point> pointsCopy = points;
        root = buildTree(pointsCopy);
    }

    // Constructor builds the tree from a vector of points
    void Init(std::vector<Point>&& points) {
        if (points.empty()) {
            root = nullptr;
            return;
        }

        root = buildTree(points);
    }

    // Destructor to clean up memory
    ~KDTree() {
        deleteTree(root);
    }

    // Function to find the nearest neighbor to a given point
    Point findNearestNeighbor(const Point& target);

    // Function to find the k nearest neighbors to a given point
    std::vector<Point> findKNearestNeighbors(const Point& target, int k);
};