#ifndef DISTANCE_CLUSTER_DATA_H_
#define DISTANCE_CLUSTER_DATA_H_

#include <string>
#include <vector>
class DistanceClusterData
{

	public:
		explicit DistanceClusterData(std::vector<float> distances_to_right_neighbours){};
		float nextRightNeighbourDistance() const
		{
			return 0;
		};
		std::size_t nextRightNeighbour() const
		{
			return 1;
		};

		std::string print() const {return std::string("");};

};

#endif /* end of include guard: DISTANCE_CLUSTER_DATA_H_ */
