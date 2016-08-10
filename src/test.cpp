#include <list>
#include <vector>
#include <algorithm>
#include <cmath>

#include "binarytree.h"
#include "distancecluster.h"
#include "cluster.h"


int main ()
{
    // intitialize a triagonal distance matrix that we will cluster
    std::list < std::vector < float>> distance_matrix { { 1, 2, 3 }, { 1, 2 }, { 1 }, {}};

    std::list <DistanceCluster> clusters;

    // give the clusters some ids
    std::list < std::vector < std::size_t>> cluster_ids { { 1 }, { 2 }, { 3 }, { 4 }};
    auto cluster_id = cluster_ids.begin();

    // generate a list of elemental clusters with distances between them and ids
    for (auto &distance_matrix_row : distance_matrix)
    {
        clusters.emplace_back( DistanceCluster(std::move(distance_matrix_row), std::move(*cluster_id)));
        ++cluster_id;
    }

    // perform a hierarchical clustering
    auto result = hierarchical_merge_into_tree(clusters, LanceWilliamsUpdate<DistanceCluster::DistanceType>::simple_average);

    std::string out; //< keeps the output

    // Proof of principle: Cut away from all clusters whose list of ids starts with a "1". Note that cluster ids aren't sorted
    auto cut_predicate = [](DistanceCluster & a){return a.elements()[0]==1;};
    auto cut_branches = result->cut(cut_predicate);

    // Print the left-over clusters
    for (auto &branch : *result)
    {
        out += (*branch).print();
    }

    // Extract the bottom of the left-over binary tree
    auto clusterbottom = result->bottom();

    // print the bottom of the left-over binary tree
    for (const auto &branch : clusterbottom){
        out += (**branch).print();
    }
    fprintf(stderr, "%s\n", out.c_str());

    return 0;
}
