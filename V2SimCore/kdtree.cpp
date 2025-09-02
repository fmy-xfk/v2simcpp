#include "kdtree.h"
#include <algorithm>

// Comparator for sorting points based on axis
bool compareX(const Point& a, const Point& b) { return a.x < b.x; }
bool compareY(const Point& a, const Point& b) { return a.y < b.y; }

KDNode* KDTree::buildTree(std::vector<Point>& points, int depth) {
    if (points.empty()) return nullptr;

    // Alternate between x and y axis
    int axis = depth % 2;

    // Sort points based on current axis
    if (axis == 0) {
        std::sort(points.begin(), points.end(), compareX);
    }
    else {
        std::sort(points.begin(), points.end(), compareY);
    }

    // Find median
    size_t median = points.size() / 2;

    // Create node
    KDNode* node = new KDNode(points[median]);

    // Recursively build left and right subtrees
    std::vector<Point> leftPoints(points.begin(), points.begin() + median);
    std::vector<Point> rightPoints(points.begin() + median + 1, points.end());

    node->left = buildTree(leftPoints, depth + 1);
    node->right = buildTree(rightPoints, depth + 1);

    return node;
}

void KDTree::nearestNeighbor(KDNode* node, const Point& target, int depth, KDNode*& best, double& bestDist) {
    if (node == nullptr) return;

    double dist = node->point.dist_to(target);
    if (dist < bestDist) {
        bestDist = dist;
        best = node;
    }

    int axis = depth % 2;
    KDNode* nextBranch = nullptr;
    KDNode* otherBranch = nullptr;

    if ((axis == 0 && target.x < node->point.x) || (axis == 1 && target.y < node->point.y)) {
        nextBranch = node->left;
        otherBranch = node->right;
    }
    else {
        nextBranch = node->right;
        otherBranch = node->left;
    }

    nearestNeighbor(nextBranch, target, depth + 1, best, bestDist);

    // Check if we need to search the other branch
    double axisDist;
    if (axis == 0) {
        axisDist = target.x - node->point.x;
    }
    else {
        axisDist = target.y - node->point.y;
    }

    if (axisDist * axisDist < bestDist) {
        nearestNeighbor(otherBranch, target, depth + 1, best, bestDist);
    }
}

void KDTree::kNearestNeighbors(KDNode* node, const Point& target, int depth,
    std::priority_queue<std::pair<double, KDNode*>>& pq, int k) {
    if (node == nullptr) return;

    double dist = node->point.dist_to(target);
    pq.push({ dist, node });

    // Maintain only k elements in the priority queue
    if (pq.size() > k) {
        pq.pop();
    }

    int axis = depth % 2;
    KDNode* nextBranch = nullptr;
    KDNode* otherBranch = nullptr;

    if ((axis == 0 && target.x < node->point.x) || (axis == 1 && target.y < node->point.y)) {
        nextBranch = node->left;
        otherBranch = node->right;
    }
    else {
        nextBranch = node->right;
        otherBranch = node->left;
    }

    kNearestNeighbors(nextBranch, target, depth + 1, pq, k);

    // Check if we need to search the other branch
    double axisDist;
    if (axis == 0) {
        axisDist = target.x - node->point.x;
    }
    else {
        axisDist = target.y - node->point.y;
    }

    if (pq.size() < k || axisDist * axisDist < pq.top().first) {
        kNearestNeighbors(otherBranch, target, depth + 1, pq, k);
    }
}

Point KDTree::findNearestNeighbor(const Point& target) {
    if (root == nullptr) {
        throw std::runtime_error("Tree is empty");
    }

    KDNode* best = nullptr;
    double bestDist = std::numeric_limits<double>::max();
    nearestNeighbor(root, target, 0, best, bestDist);

    if (best == nullptr) {
        throw std::runtime_error("No nearest neighbor found");
    }

    return best->point;
}

std::vector<Point> KDTree::findKNearestNeighbors(const Point& target, int k) {
    if (root == nullptr) {
        throw std::runtime_error("Tree is empty");
    }

    // Use a max-heap to keep track of the k smallest distances
    std::priority_queue<std::pair<double, KDNode*>> pq;
    kNearestNeighbors(root, target, 0, pq, k);

    std::vector<Point> result;
    while (!pq.empty()) {
        result.push_back(pq.top().second->point);
        pq.pop();
    }

    // The results come out in reverse order (max to min), so reverse them
    std::reverse(result.begin(), result.end());

    return result;
}