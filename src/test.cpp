#include <list>
#include <vector>
#include <algorithm>
#include <cmath>

#include "binarytree.h"
#include "distancecluster.h"
#include "cluster.h"


int main ()
{
    std::list < std::vector < float>> distance_matrix { { 1, 2, 3 }, { 1, 2 }, { 1 }, {}};
    std::list <DistanceCluster> clusters;
    std::list < std::vector < std::size_t>> cluster_ids { { 1 }, { 2 }, { 3 }, { 4 }};
    auto cluster_id = cluster_ids.begin();
    for (auto &distance_matrix_row : distance_matrix)
    {
        clusters.emplace_back( DistanceCluster(std::move(distance_matrix_row), std::move(*cluster_id)));
        ++cluster_id;
    }
    auto result = hierarchical_merge_into_tree(clusters, LanceWilliamsUpdate<DistanceCluster::DistanceType>::simple_average);

    std::string out;
    auto cut_predicate = [](DistanceCluster & a){return a.elements()[0]==1;};
    auto cut_branches = result->cut(cut_predicate);
    for (auto &branch : *result)
    {
        out += (*branch).print();
    }
    auto clusterbottom = result->bottom();
    for (const auto &branch : clusterbottom){
        out += (**branch).print();
    }
    fprintf(stderr, "%s\n", out.c_str());
    return 0;
}
