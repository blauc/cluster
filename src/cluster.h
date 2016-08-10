/*
 * cluster.h
 *  Created on: August 3, 2016
 *      Author: cblau@gwdg.de
 */
#ifndef CLUSTER_H_
#define CLUSTER_H_

#include <algorithm>
#include <cmath>
#include <list>
#include <memory>
#include <vector>

#include "binarytree.h"

/*! \brief Construct a binary tree by hierarchically merging list items.
 *
 * \tparam Clusterable The items to be clustered must provide the following:
 *  DistanceType, deleteDistance, merge, distances, size.
 * \tparam MergeFunction
 * \param[in] items_to_cluster List of items to cluster.
 * \param[in] merge_distance A function defining how to calculate the new distances to the merged cluster.
 *
 * Clusters are represented by binary trees.
 * Store only distances to the next clusters in the list to employ symmetry in the distance matrix.
 *
 */
template <typename Clusterable, typename MergeFunction>
std::unique_ptr < BinaryTree < Clusterable>> hierarchical_merge_into_tree(std::list<Clusterable> &items_to_cluster, MergeFunction merge_distance)
{
    /* Treat each item as single cluster, i.e. binary tree for hierarchical clustering */
    std::list < std::unique_ptr < BinaryTree < Clusterable>> > list_of_trees {};
    /* Store the size of the clusters in a vector. */
    std::vector<std::size_t> cluster_sizes {};
    for (auto &cluster : items_to_cluster)
    {
        cluster_sizes.push_back(cluster.size());
        list_of_trees.emplace_back( new BinaryTree<Clusterable>{std::move(cluster)} );
    }

    /* For brevity, define a function to give the distance to the next up neighbour */
    auto next_up_neighbour_distance = []( BinaryTree < Clusterable> &a){
            return *std::min_element(std::begin((*a).distances()), std::end((*a).distances()), [](const typename Clusterable::DistanceType &a, const typename Clusterable::DistanceType &b){ return a < b; });
        };

    /* For brevity, define a function to give the next up neighbour */
    auto next_up_neighbour = [] (BinaryTree<Clusterable> &a ) {
            return std::min_element(begin((*a).distances()), end((*a).distances()))-(*a).distances().begin()+1;
        };
    /* Merge trees until two independent trees are left (then selecting the two trees to be merged is trivial). */
    while (list_of_trees.size() > 2)
    {

        /* The new left element in the merged tree will be the element with the lowest distance to another element */
        auto left = std::min_element(std::begin(list_of_trees), --std::end(list_of_trees),
                                     [next_up_neighbour_distance] ( std::unique_ptr < BinaryTree < Clusterable>> &a, std::unique_ptr < BinaryTree < Clusterable>> &b )
                                     {
                                         return next_up_neighbour_distance(*a) < next_up_neighbour_distance(*b);
                                     }
                                     );
        /* Convenience abbrevivation */
        Clusterable   &left_cluster     = ***left;
        /* The new right element in the merged tree will be the closest element to the left element just picked */
        auto           minimum_distance = next_up_neighbour_distance(**left);

        auto           right            = left;
        std::advance(right, next_up_neighbour(**left));
        Clusterable   &right_cluster = ***right;

        /* Collect distances to the "to-be-merged" clusters into a new vector
         * and discard them in their old storage place.
         * Distances are stored in triangle shape, with the first cluster holding
         * all n-1 distances to following elements in the list,
         * the second cluster holding n-2 distances to following elements and
         * the last cluster holding no distance. */
        auto left_index    = std::distance(list_of_trees.begin(), left );
        auto right_index   = std::distance(list_of_trees.begin(), right );

        std::vector<typename Clusterable::DistanceType>  distances_to_left {};
        std::vector<typename Clusterable::DistanceType>  distances_to_right {};
        auto current = list_of_trees.begin();
        for (auto current_index = 0; current_index < left_index; ++current_index)
        {
            distances_to_left.push_back((***current).distances()[left_index-current_index]);
            distances_to_right.push_back((***current).distances()[right_index-current_index]);
            (***current).deleteDistance(left_index-current_index);
            (***current).deleteDistance(right_index-current_index-1); // already deleted distance at left_index, so go back one more
            ++current;
        }
        /* when at the first element to be merged, store all its distances */
        for (auto &distance : left_cluster.distances())
        {
            distances_to_left.push_back(distance);
        }
        distances_to_right.push_back((***current).distances()[right_index-left_index]);
        ++current;
        for (auto i = left_index+1; i < right_index; ++i)
        {
            distances_to_right.push_back((***current).distances()[right_index-i]);
            (***current).deleteDistance(right_index-i);
            ++current;
        }

        for (auto &distance : right_cluster.distances())
        {
            distances_to_right.push_back(distance);
        }

        /* end collect distances */

        /* Merge the two clusters and calculate new distances */
        auto d_right   = distances_to_right.begin();
        auto n_current = cluster_sizes.begin();
        std::vector<typename Clusterable::DistanceType> new_distances;
        for (auto d_left : distances_to_left)
        {
            new_distances.push_back(merge_distance(minimum_distance, *n_current, cluster_sizes[left_index], cluster_sizes[right_index], d_left, *d_right));
            d_right++;
            n_current++;
        }

        /* Put the new constructed cluster in place */
        list_of_trees.emplace_front(new BinaryTree<Clusterable>(std::move(*left), std::move(*right), *left_cluster.merger(right_cluster, std::move(new_distances), minimum_distance)));
        /* Update the array that holds the cluster sizes */
        cluster_sizes.insert(cluster_sizes.begin(), cluster_sizes[left_index]+cluster_sizes[right_index]);
        cluster_sizes.erase(cluster_sizes.begin()+left_index);
        cluster_sizes.erase(cluster_sizes.begin()+right_index);

        /* Remove the merged elements from the list of clusters */
        list_of_trees.erase(left);
        list_of_trees.erase(right);

        /* Reset the distance containers */
        distances_to_left.resize(0);
        distances_to_right.resize(0);
    }

    /* Merge the two left-over clusters into one */
    auto left_cluster  = **list_of_trees.front();
    auto right_cluster = **list_of_trees.back();
    std::unique_ptr<Clusterable> new_cluster {
        left_cluster.merger(right_cluster, std::move(std::vector<typename Clusterable::DistanceType>(left_cluster.distances().front())), left_cluster.distances().front())
    };
    list_of_trees.emplace_front(new BinaryTree<Clusterable>(std::move(list_of_trees.front()), std::move(list_of_trees.back()), *new_cluster));
    list_of_trees.pop_back();
    list_of_trees.pop_back();

    return std::move(list_of_trees.front());
}

template <typename DistanceType>
class LanceWilliamsUpdate
{
    public:
        /*!\brief Single linkage merge, also known as nearest neighbour.
         * Lance Williams parameters are:
         *  a_i =  1/2
         *  b   =  0
         *  g   = -1/2
         *
         *  \param[in] d_to_left Distance to "left" or first cluster to be merged.
         *  \param[in] d_to_right Distance to "right" or second cluster to be merged.
         */
        static DistanceType
        single_linkage(DistanceType /*d_left_right*/, std::size_t /*size_current*/, std::size_t /*size_left*/, std::size_t /*size_right*/, DistanceType d_to_left, DistanceType d_to_right )
        {
            return 0.5 * d_to_left + 0.5 * d_to_right - 0.5 * std::abs(d_to_left - d_to_right);
        }

        /*!\brief Complete linkage merge, also known as diameter or max dist merge.
         * Lance Williams parameters are:
         *  a_i =  1/2
         *  b   =  0
         *  g   =  1/2
         *
         *  \param[in] d_to_left Distance to "left" or first cluster to be merged.
         *  \param[in] d_to_right Distance to "right" or second cluster to be merged.
         */
        static DistanceType
        complete_linkage(DistanceType d_left_right, std::size_t size_current, std::size_t size_left, std::size_t size_right, DistanceType d_to_left, DistanceType d_to_right)
        {
            return 0.5 * d_to_left + 0.5 * d_to_right + 0.5 * std::abs(d_to_left - d_to_right);
        }

        /*!\brief Simple average, also known as McQuitty's or Weighted Pair Group Method with Arithmetic Mean (WPGMA).
         * Lance Williams parameters are:
         *  a_i =  1/2
         *  b   =  0
         *  g   =  0
         *
         *  \param[in] d_to_left Distance to "left" or first cluster to be merged.
         *  \param[in] d_to_right Distance to "right" or second cluster to be merged.
         */
        static DistanceType
        simple_average(DistanceType /*d_left_right*/, std::size_t /*size_current*/, std::size_t /*size_left*/, std::size_t /*size_right*/, DistanceType d_to_left, DistanceType d_to_right)
        {
            return 0.5 * d_to_left + 0.5 * d_to_right;
        }
        /*!\brief Centroid merge, or Unweighted Pair Group Method with Centroid Mean (UPGMC).
         * Lance Williams parameters are:
         *  a_i =  n_i / (n_i+n_j)
         *  b   =  0
         *  g   =  a_i * a_j
         *
         *  \param[in] d_left_right Distance between the two clusters to be merged.
         *  \param[in] size_left Size of the "left" or first cluster to be merged.
         *  \param[in] size_right Size of the "right" or second cluster to be merged.
         *  \param[in] d_to_left Distance to "left" or first cluster to be merged.
         *  \param[in] d_to_right Distance to "right" or second cluster to be merged.
         */
        static DistanceType
        centroid(DistanceType d_left_right, std::size_t /*size_current*/, std::size_t size_left, std::size_t size_right, DistanceType d_to_left, DistanceType d_to_right)
        {
            auto a_left  = size_left/(size_left+size_right);
            auto a_right = size_right/(size_left+size_right);
            return a_left * d_to_left +  a_right  * d_to_right - a_left * a_right * d_left_right;
        }
        /*!\brief Median merge, also known as Gowers or Weighted Pair Group Method with Centroid Mean (WPGMC).
         * Lance Williams parameters are:
         *  a_i =  1/2
         *  b   = -1/4
         *  g   =  0
         *
         *  \param[in] d_to_left Distance to "left" or first cluster to be merged.
         *  \param[in] d_to_right Distance to "right" or second cluster to be merged.
         */
        static DistanceType
        median(DistanceType d_left_right, std::size_t /*size_current*/, std::size_t /*size_left*/, std::size_t /*size_right*/, DistanceType d_to_left, DistanceType d_to_right)
        {
            return 0.5 * d_to_left + 0.5 * d_to_right - 0.25 * d_left_right;
        }

        /*!\brief Group average merge, also known as average link or Unweighted Pair Group Method with Average Mean (UPGMA)
         * Lance Williams parameters are:
         *  a_i =  n_i / (n_i+n_j)
         *  b   =  0
         *  g   =  0
         *
         *  \param[in] size_left Size of the "left" or first cluster to be merged.
         *  \param[in] size_right Size of the "right" or second cluster to be merged.
         *  \param[in] d_to_left Distance to "left" or first cluster to be merged.
         *  \param[in] d_to_right Distance to "right" or second cluster to be merged.
         */
        static DistanceType
        group_average(DistanceType /*d_left_right*/, std::size_t /*size_current*/, std::size_t size_left, std::size_t size_right, DistanceType d_to_left, DistanceType d_to_right)
        {
            DistanceType a_left  = size_left/(DistanceType)(size_left+size_right);
            DistanceType a_right = size_right/(DistanceType)(size_left+size_right);
            return a_left * d_to_left +  a_right  * d_to_right;
        }

        /*!\brief Ward minimum distance merge, also known as minimum variance or error sum if squares clustering.
         * Lance Williams parameters are:
         *  a_i =  (n_k + n_i) / (n_k + n_i + n_j)
         *  b   =  - n_k / (n_k + n_i + n_j)
         *  g   =  0
         *
         *  \param[in] d_left_right Distance between the two clusters to be merged.
         *  \param[in] size_current Size of the current cluster.
         *  \param[in] size_left Size of the "left" or first cluster to be merged.
         *  \param[in] size_right Size of the "right" or second cluster to be merged.
         *  \param[in] d_to_left Distance to "left" or first cluster to be merged.
         *  \param[in] d_to_right Distance to "right" or second cluster to be merged.
         */
        static DistanceType
        ward_minimum_distance(DistanceType d_left_right, std::size_t size_current, std::size_t size_left, std::size_t size_right, DistanceType d_to_left, DistanceType d_to_right)
        {
            DistanceType total_size = (size_current + size_left + size_right);
            auto         a_left     = (size_current + size_left)/total_size;
            auto         a_right    = (size_current + size_right)/total_size;
            auto         b          =  size_current / (DistanceType)total_size;
            return a_left * d_to_left + a_right * d_to_right - b * d_left_right;
        }
};


#endif /* end of include guard: CLUSTER_H_ */
