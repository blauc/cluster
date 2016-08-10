#ifndef DISTANCE_CLUSTER_DATA_H_
#define DISTANCE_CLUSTER_DATA_H_

#include <string>
#include <vector>
#include <algorithm>
#include <memory>

class DistanceCluster
{

	public:
		/*!\brief Type that describes the mutual distance between clusters.*/
		typedef float DistanceType;

		/*!\brief Constructor that copies the elements for clustering.
		 * \param[in] dist_to_following provides distances to the following clusters to be copied.
		 * \param[in] elements provides cluster elements to be copied.
		 * \param[in] merge_d If the cluster is created from a merger, the merge distance at which the cluster was created.
		 */
		DistanceCluster(const std::vector<DistanceType> & dist_to_following, const std::vector<size_t> & elements, DistanceType merge_d=0);

		/*!\brief Constructor that copies the elements for clustering.
		 * \param[in] dist_to_following provides distances to the following clusters to be copied.
		 * \param[in] elements provides cluster elements to be copied.
		 * \param[in] merge_d If the cluster is created from a merger, the merge distance at which the cluster was created.
		 */
		DistanceCluster(std::vector<DistanceType> && dist_to_following, std::vector<size_t> && elements, DistanceType merge_d=0);

		/*!\brief The number of elements in the cluster. */
		std::size_t size() const;
		/*!\brief Return the merger of this cluster with another as unique_ptr.
		 * The original clusters stay intact.
		 * For memory efficiency with large clusters consider not merging elements, but back-calculating them from binary tree representation.
		 * \param[in] other The other cluster to merge into.
		 * \param[in] dist_to_following A distance vector for the created cluster.
		 * \param[in] merge_d the distance at which this cluster was merged.
		 *
		 * \result The caller owns the merged clusters via a unique_ptr.
		 */
		std::unique_ptr<DistanceCluster> merger(const DistanceCluster & other, std::vector<DistanceType> dist_to_following, DistanceType merge_d);
		/*!\brief Observe the distances to following clusters via a const ref. */
		const std::vector<DistanceType> & distances() const;
		/*!\brief Delete a distances to a following cluster */
		void deleteDistance(size_t index);
		/*!\brief Prints cluster information into a string. */
		std::string print() const;
		/*!\brief Indizes to the what the cluster represents
		 * \returns A const ref to a list of indices to the cluster elemetns
		 */
		const std::vector<size_t> & elements() const;

	private:
		std::vector<size_t> elements_; //< indexed elements of this cluster.
		std::vector<DistanceType> dist_to_following_; //< distance to the follow up elements in the cluster
		DistanceType merge_d_; //< distance at which this cluster was created
};
#endif /* end of include guard: DISTANCE_CLUSTER_DATA_H_ */
