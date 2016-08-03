/*
 * cluster.h
 *  Created on: August 3, 2016
 *      Author: cblau@gwdg.de
 */

#include <list>
#include <algorithm>

/*! \brief Generic hierachical clustering.
 *
 * Hierachical clustering merges closest clusters from a list until
 * one cluster is left.
 */
template <typename T>
void hierarchical_clustering(std::list<T> &clusters)
{
    while (clusters.size() > 1)
    {
        auto left = std::min_element(begin(clusters), end(clusters),
                                     [] (const T &a, const T &b ){
                                         return nextRightNeighbourDistance(a) < nextRightNeighbourDistance(b);
                                     });
        auto minimum_distance = nextRightNeighbourDistance(*left);
        auto right(left);
        auto advance_right = nextRightNeighbour(*left);
        auto index_left=distance(begin(clusters),left);
        auto index_right=index_left+advance_right;
        do
        {
            ++right;
        } while (--advance_right >= 0);
        /* Distances to the next right */
        for_each(
                begin(clusters), right,
                [minimum_distance, index_left, index_right](T & cluster){
                    updateDistances(cluster, minimum_distance, index_left, index_right);
                });
        mergeInto(*left, *right);
        clusters.erase(right);
    }
}
