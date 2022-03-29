#include "AABBBuilder.h"




void AABBBuilder::startBuild() {
	vector<uvec4> relevantTriangles = {};
	vector<vec3> relevantPoints = {};
	for (int i = 0; i < scene->numOfMeshes(); i++) {
		shared_ptr<Mesh> myMesh = scene->mesh(i);
		const vector<uvec3>& myTriangleIndices = myMesh->triangleIndices();
		for (uvec3 index : myTriangleIndices) {
			relevantTriangles.push_back(uvec4(index, i));
		}
	}


	this->firstRoot->inflate(0.2f);
	//build(relevantTriangles, this->firstRoot, 0);
	build2(relevantTriangles, this->firstRoot, 0);
	std::cout << glm::to_string(firstRoot->getMax()) << std::endl;
	std::cout << glm::to_string(firstRoot->getMin()) << std::endl;
	std::cout << "sum of triangles before building " << relevantTriangles.size() << std::endl;
	std::cout << "sum of triangles after building " << sum << std::endl;
}

bool testDimension(int prev, vector<uvec4>& left, vector<uvec4>& right, int& d, AABB* root, bool debug = false) {
	if (debug) {
		std::cout << "right is " << right.size() << std::endl;
		std::cout << "left is " << left.size() << std::endl;
		std::cout << prev << std::endl;
	}


	//if (d > 2) {
	//	root->inflate(5.f);
	//	d = 0;
	//	return true;
	//}

	if ((prev == left.size()) || (prev == right.size())) {
		left.clear();
		right.clear();
		return true;
	}

	return false;
}


void AABBBuilder::build(std::vector<uvec4> in, AABB* root, int s) {

	if (in.size() <= 50) {
		sum += in.size();
		root->setData(in);
		return;
	}

	if (in.size() == 0) {
		return;
	}

	vec3 rootMax = root->getMax();
	vec3 rootMin = root->getMin();
	vec3 middle = (rootMax + rootMin) / 2.f;

	vector<uvec4> left = {}, right = {};
	int d = 0;
	int max = 0;
	do {

		for (uvec4 index : in) {
			vec3 barycenter = vec3(0.0);
			for (int i = 0; i < 3; i++) {
				barycenter += scene->mesh(index[3])->vertexPositions()[index[i]];

			}

			barycenter /= 3.f;

			if (barycenter[d] >= middle[d]) {
				left.push_back(index);

			}
			else {
				right.push_back(index);
			}
		}
		d++;

	} while (testDimension(in.size(), left, right, d, root));

	if (left.size() != 0) {
		AABB* child1 = new AABB(left, scene);
		child1->inflate(0.2);
		root->setLeft(child1);
		build(left, child1, (s + 1));
	}
	if (right.size() != 0) {
		AABB* child2 = new AABB(right, scene);
		child2->inflate(0.2);
		root->setRight(child2);
		build(right, child2, (s + 1));
	}

}

bool AABBBuilder::rayTrace(const Ray& ray, AABB* root, vector<uvec4>& result) {
	if (!(root->rayIntersect(ray))) {
		return false;
	}
	if (root->isReady()) {
		vector<uvec4>& data = root->getData();
		result.reserve(data.size() + result.size()); 
		result.insert(result.end(), data.begin(), data.end());
		return true;
	}
	else {
		AABBBuilder::rayTrace(ray, root->getLeft(), result);
		AABBBuilder::rayTrace(ray, root->getRight(), result);
	}
}

void AABBBuilder::build2(std::vector<uvec4> in, AABB* root, int s) {

	if (in.size() <= 11) {
		sum += in.size();
		root->setData(in);
		return;
	}



	vec3 rootMax = root->getMax();
	vec3 rootMin = root->getMin();
	vec3 middle = (rootMax + rootMin) / 2.f;

	float height = glm::abs(rootMax.z - rootMin.z);
	float width = glm::abs(rootMax.x - rootMin.x);
	float depth = glm::abs(rootMax.y - rootMin.y);
	int d = 0;
	
	if ((height >= width) && (height >= depth)) {
		d = 2;
	}
	else if ((width >= height) && (width >= depth)) {
		d = 0;
	}
	else if ((depth >= height) && (depth >= width)) {
		d = 1;
	}

	vector<float> list = {};

	

	for (uvec4 index : in) {
		const vector<vec3>& myVertexPositions = scene->mesh(index[3])->vertexPositionsModel();
		float barycenter = 0;
		for (int i = 0; i < 3; i++) {
			barycenter += myVertexPositions[index[i]][d];	
		}

		barycenter /= 3.f;
		list.push_back(barycenter);
	}


	std::sort(list.begin(), list.end());
	
	vector<uvec4> left = {}, right = {};


	for (uvec4 index : in) {
		
		const vector<vec3>& myVertexPositions = scene->mesh(index[3])->vertexPositionsModel();
		
		vec3 barycenter = vec3(0.0);
		int lm = 0;
		for (int i = 0; i < 3; i++) {
			barycenter += myVertexPositions[index[i]];
			/*
			float v = myVertexPositions[index[i]][d];
			if (v > list[list.size() / 2]) {
				lm++;
			}
			*/
		}
		
		barycenter /= 3.f;
		
		if (barycenter[d] >= list[list.size() / 2]) {
			left.push_back(index);

		}
		else {
			right.push_back(index);
		}
		
		//
		/*
		if (lm == 3) {
			left.push_back(index);
		}
		else if (lm == 0) {
			right.push_back(index);
		}
		else {
			right.push_back(index);
			left.push_back(index);
		}
		*/

	}

	if (left.size() != 0) {
		AABB* child1 = new AABB(left, scene);
		root->setLeft(child1);
		build2(left, child1, (s + 1));
	}
	if (right.size() != 0) {
		AABB* child2 = new AABB(right, scene);
		root->setRight(child2);
		build2(right, child2, (s + 1));
	}

}


/*
if ((debug == true) && (s==0)){
	std::cout << "Debug§§§§§§§§§§§§§§" << std::endl;
	std::cout << "prev " << previous_size << " " << in.size() << std::endl;
	std::cout << "right " << right.size() << std::endl;
	std::cout << "left " << left.size() << std::endl;
	std::cout << "dimension in debug is" << d-1 << std::endl;

	std::cout << "middle is " << to_string(middle) << std::endl;

	for (uvec4 index : left) {
		vector<vec3>& vertices = scene->mesh(index[3])->vertexPositions();
		std::cout << to_string(vertices[index[0]]) << std::endl;
		std::cout << to_string(vertices[index[1]]) << std::endl;
		std::cout << to_string(vertices[index[3]]) << std::endl;
		vec3 barycenter = vertices[index[0]] + vertices[index[1]] + vertices[index[2]];
		std::cout << "barycenter is " << to_string(barycenter) << std::endl;
	}
	std::cout << "Debug§§§§§§§§§§§§§§" << std::endl;
}*/


/*
std::cout << "prev " << previous_size << " " << in.size() << std::endl;
std::cout << "right " << right.size() << std::endl;
std::cout << "left " << left.size() << std::endl;
std::cout << "dimension is" << d - 1 << std::endl;
*/

//std::cout << "in size " << in.size() << std::endl;
/*
if ((in.size() == 117) && (debug==false) && (left.size()!=0)) {
	AABB* zall = new AABB(left, scene);

	std::cout << "prev " << previous_size << " " << in.size() << std::endl;
	std::cout << "right " << right.size() << std::endl;
	std::cout << "left " << left.size() << std::endl;
	std::cout << "dimension is" << d - 1 << std::endl;

	build(left, zall, 0, left.size(),true);
}*/




/*

void AABBBuilder::build(std::vector<uvec4> in, AABB* root, int s) {

	if (in.size() <= 10) {
		sum += in.size();
		root->setData(in);
		return;
	}

	if (in.size() == 0) {
		return;
	}

	vec3 rootMax = root->getMax();
	vec3 rootMin = root->getMin();
	vec3 middle = (rootMax + rootMin) / 2.f;

	vector<uvec4> left = {}, right = {};
	int d = 0;
	int max = 0;
	do {

		for (uvec4 index : in) {
			int lm = 0;
			vec3 barycenter = vec3(0.0);
			for (int i = 0; i < 3; i++) {
				barycenter += scene->mesh(index[3])->vertexPositions()[index[i]];

			}

			barycenter /= 3.f;

			if (barycenter[d] >= middle[d]) {
				left.push_back(index);

			}
			else {
				right.push_back(index);
			}
		}
		d++;




		} while (testDimension(in.size(), left, right, d, root));



bool testDimension(int prev, vector<uvec4>& left, vector<uvec4>& right,int& d,AABB * root,bool debug = false) {
	if (debug) {
		std::cout << "right is " <<right.size()<< std::endl;
		std::cout << "left is " << left.size() << std::endl;
		std::cout << prev << std::endl;
	}


	//if (d > 2) {
	//	root->inflate(5.f);
	//	d = 0;
	//	return true;
	//}

if ((prev == left.size()) || (prev == right.size())) {
	left.clear();
	right.clear();
	return true;
}

return false;
}
*/