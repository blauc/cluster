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
        auto right(left);
        auto advance_right = nextRightNeighbour(*left);
        do
        {
            ++right;
        }
        while (--advance_right >= 0);
        /* Distances to the next right */
        for_each(
                begin(clusters), right,
                [left, right](T cluster){
                    updateDistances(cluster, *left, *right);
                });
        *left = merge(*left, *right);
        clusters.erase(right);
    }
}
