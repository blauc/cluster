#include <list>
#include <vector>
#include <algorithm>

#include "cluster.h"
#include "binarytree.h"
#include "distanceclusterdata.h"


float
nextRightNeighbourDistance(const BinaryTree<DistanceClusterData> & branch)
{
	return branch.value().nextRightNeighbourDistance();
};

size_t
nextRightNeighbour(const BinaryTree<DistanceClusterData> & branch)
{
	return branch.value().nextRightNeighbour();
};



int main ()
{
    std::list < std::vector < float>> distance_matrix { { 1, 2, 3, 4 }, { 1, 2, 3 }, { 1, 2 }, { 1 }};
	std::list<BinaryTree<DistanceClusterData>> clusters;
	for (const auto & distance_matrix_row : distance_matrix){
	    clusters.emplace_back(distance_matrix_row);
	}
    hierarchical_clustering(clusters);
	for (const auto & branch : clusters.front()){
	    branch.print();
	}
    return 0;
}
