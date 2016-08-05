#include <list>
#include <vector>
#include <algorithm>
#include <cmath>

#include "binarytree.h"
#include "distancecluster.h"
#include "cluster.h"

float
mergeDistance(float minimum_distance, std::size_t size_left, std::size_t size_right, float d_left, float d_right )
{
    float ai = 1;
    float aj = 1;
    float b  = 1;
    float g  = 1;
    return ai * d_left + aj * d_right + b * minimum_distance + g * std::fabs(d_left - d_right);
}

int main ()
{
    std::list < std::vector < float>> distance_matrix { {
                                                            1, 2, 3, 4
                                                        }, {
                                                            1, 2, 3
                                                        }, {
                                                            1, 2
                                                        }, {
                                                            1
                                                        }};
    std::list <DistanceCluster> clusters;
    std::size_t i = 0;
    for (auto &distance_matrix_row : distance_matrix)
    {
        clusters.emplace_back( DistanceCluster(std::move(distance_matrix_row), std::move(std::vector<std::size_t>{++i}), 0));
    }
    auto result = hierarchical_clustering(clusters, mergeDistance);
    for (const auto &branch : *result)
    {
        branch.print();
    }
    return 0;
}
