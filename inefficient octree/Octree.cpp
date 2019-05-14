//  Kevin M. Smith - Basic Octree Class - CS134/235 4/18/18
//


#include "Octree.h"


// draw Octree (recursively)
//
void Octree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;

	ofSetColor(levelColors[level % sizeof(levelColors)]);

	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// draw only leaf Nodes
//
void Octree::drawLeafNodes(TreeNode & node) {

	ofSetColor(ofColor::tan);
	if (node.children.size() > 0)
		for (int i = 0; i < node.children.size(); i++)
			drawLeafNodes(node.children[i]);
	else if (node.points.size() > 0)
		drawBox(node.box);
}


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
	//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}



//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & geo, int numLevels) {
	// initialize octree structure

	float startTime = ofGetElapsedTimeMillis();
	float timeElapsed;
	mesh = geo;
	root = TreeNode();
	root.box = meshBounds(geo);

	// initialize root's points
	for (int i = 0; i < geo.getNumIndices(); i++)
		root.points.push_back(geo.getIndex(i));

	//cout << "entered" << endl;
	subdivide(geo, root, numLevels, 0);
	//cout << "done" << endl;

	timeElapsed = (ofGetElapsedTimeMillis() - startTime) / 1000;
	cout << timeElapsed << " seconds to create the octree with " << numLevels << " levels" << endl;
}

void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {

	if (level >= numLevels)
		return;

	// Create a node for the box containing:
	// List of point indices
	// List of child boxes
	vector<Box> childBoxes;

	//Subdivide the box into 8 equally sized boxes
	subDivideBox8(node.box, childBoxes);

	for (int i = 0; i < childBoxes.size(); i++)
	{
		// Create a node for each of the 8 child boxes
		TreeNode child;
		child.box = childBoxes[i];

		//For all indices in the box, add to an index list in each child node if the point is in the child box.
		int countOfPoints = getMeshPointsInBox(mesh, node.points, child.box, child.points);

		// If there is more than one vertex stored in a child:
		// Call Subdivide(recursively)
		if (countOfPoints > 1)
		{
			subdivide(mesh, child, numLevels, level + 1);
		}

		//If a child box is not empty, add to the parent list of child boxes;
		if (child.points.size() > 0)
			node.children.push_back(child);
	}


}

bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
	if (node.box.intersect(ray, -INTERSECT, INTERSECT))
	{
		// Repeat until we find a box that contains only one point (this is the selection)
		if (node.children.size() == 0)
		{
			nodeRtn = node;
			selectedPoint = node;
			return true;
		}

		//Recursively traverse the Octree testing for intersection with a box. If the box is hit go to the next level down
		for (int i = 0; i < node.children.size(); i++)
		{
			if (intersect(ray, node.children[i], nodeRtn))
				return true;
		}
	}

	return false;
}