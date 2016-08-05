/*
 * cluster.h
 *  Created on: August 3, 2016
 *      Author: cblau@gwdg.de
 */
#ifndef CLUSTER_H_
#define CLUSTER_H_

#include <list>
#include <vector>
#include <algorithm>
#include <memory>

#include "binarytree.h"
/*! \brief Generic hierachical clustering.
 *
 * Hierachical clustering merges closest clusters from a list until
 * one cluster is left.
 */
template <typename T, typename MergeFunction>
std::unique_ptr < BinaryTree < T>> hierarchical_clustering(std::list<T> &clusters, MergeFunction mergeDistance)
{
    typedef std::unique_ptr < BinaryTree < T>> ClusterHandle;
    std::list < ClusterHandle > result {};
    for (auto &cluster : clusters)
    {
        result.emplace_back(
                new BinaryTree<T>(std::move(std::unique_ptr<T>(&cluster)))
                );
    }

    while (result.size() > 1)
    {
        auto left = std::min_element(begin(result), end(result),
                                     [] ( ClusterHandle &a, ClusterHandle &b ){
                                         return (**a).nextRightNeighbourDistance() < (**b).nextRightNeighbourDistance();
                                     });
        T   &left_cluster     = ***left;
        auto minimum_distance = left_cluster.nextRightNeighbourDistance();
        auto right            = left;
        std::advance(right, left_cluster.nextRightNeighbour());
        T   &right_cluster = ***right;
        auto left_index    = std::distance(result.begin(), left );
        auto right_index   = std::distance(result.begin(), right );

        decltype(left_cluster.distances()) distances_to_left {};
        decltype(distances_to_left) distances_to_right;
        auto current = result.begin();
        for (auto i = 0; i < left_index; ++i)
        {
            distances_to_left.push_back((***current).distance(left_index-i));
            (***current).deleteDistance(left_index-i);
            distances_to_right.push_back((***current).distance(right_index-i));
            (***current).deleteDistance(right_index-i);
            ++current;
        }
        distances_to_left.insert(end(distances_to_left), left_cluster.distances().begin(), left_cluster.distances().end());
        ++current;
        for (auto i = left_index+1; i < right_index; ++i)
        {
            distances_to_right.push_back((***current).distance(right_index-i));
            (***current).deleteDistance(right_index-i);
            ++current;
        }
        distances_to_right.insert(end(distances_to_right), right_cluster.distances().begin(), right_cluster.distances().end());

        auto d_right = distances_to_right.begin();
        auto n_left  = left_cluster.size();
        auto n_right = right_cluster.size();
        decltype(distances_to_left) new_distances;
        for (auto d_left : distances_to_left)
        {
            new_distances.push_back(mergeDistance(minimum_distance, n_left, n_right, d_left, *d_right));
            d_right++;
        }

        distances_to_right.resize(0);
        distances_to_left.resize(0);
        std::unique_ptr<T> new_cluster {
            left_cluster.merge(right_cluster, std::move(new_distances), minimum_distance)
        };
        result.emplace_front(new BinaryTree<T>(std::move(*left), std::move(*right), std::move(new_cluster)));
        result.erase(left);
        result.erase(right);
    }
}
#endif /* end of include guard: CLUSTER_H_ */
