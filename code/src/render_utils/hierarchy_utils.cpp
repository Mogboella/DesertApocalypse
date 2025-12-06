#include "hierarchy_utils.h"

using namespace glm;
using namespace std;

void rebuildNodePointers(HierarchicalModel &hmodel,
                         const vector<pair<string, string>> &parentChildPairs,
                         const string &rootName)
{
    if (hmodel.nodes.empty())
        return;

    for (size_t i = 0; i < hmodel.nodes.size(); i++)
    {
        hmodel.nodes[i].parent = nullptr;
        hmodel.nodes[i].children.clear();
    }

    map<string, HierarchicalNode *> nameToNode;
    for (size_t i = 0; i < hmodel.nodes.size(); i++)
    {
        nameToNode[hmodel.nodes[i].name] = &hmodel.nodes[i];
    }

    auto rootIt = nameToNode.find(rootName);
    hmodel.rootNode = (rootIt != nameToNode.end()) ? rootIt->second : &hmodel.nodes[0];

    for (const auto &pair : parentChildPairs)
    {
        auto parentIt = nameToNode.find(pair.first);
        auto childIt = nameToNode.find(pair.second);

        if (parentIt != nameToNode.end() && childIt != nameToNode.end())
        {
            HierarchicalNode *parent = parentIt->second;
            HierarchicalNode *child = childIt->second;

            parent->children.push_back(child);
            child->parent = parent;
        }
    }
}

void collectHierarchy(HierarchicalNode *node, vector<pair<string, string>> &pairs, string &rootName)
{
    if (!node)
        return;

    if (node->parent == nullptr)
    {
        rootName = node->name;
    }
    else
    {
        pairs.push_back({node->parent->name, node->name});
    }

    for (HierarchicalNode *child : node->children)
    {
        if (child)
        {
            collectHierarchy(child, pairs, rootName);
        }
    }
}

bool isValidNodePointer(HierarchicalNode *node, const vector<HierarchicalNode> &allNodes)
{
    if (!node || allNodes.empty())
        return false;

    const HierarchicalNode *first = &allNodes[0];
    const HierarchicalNode *last = &allNodes[allNodes.size() - 1];

    return (node >= first && node <= last);
}

// TONOTE: for debugging purposes only
void printNode(HierarchicalNode *node)
{
    if (!node)
        return;
}
