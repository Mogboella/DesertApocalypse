#ifndef HIERARCHY_UTILS
#define HIERARCHY_UTILS

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>


#include "mesh_loader.h"

using namespace glm;
using namespace std;

void rebuildNodePointers(HierarchicalModel &hmodel,
                         const vector<pair<string, string>> &parentChildPairs,
                         const string &rootName);
void collectHierarchy(HierarchicalNode *node, vector<pair<string, string>> &pairs, string &rootName);
bool isValidNodePointer(HierarchicalNode *node, const vector<HierarchicalNode> &allNodes);
void printNode(HierarchicalNode *node);

#endif
