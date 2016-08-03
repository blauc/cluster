#ifndef DISTANCE_CLUSTER_DATA_H_
#define DISTANCE_CLUSTER_DATA_H_

#include <string>
class DistanceClusterData
{

	public:
		float nextRightNeighbourDistance() const
		{
			return 0;
		};
		std::size_t nextRightNeighbour() const
		{
			return 1;
		};

		std::string print() const;

};

#endif /* end of include guard: DISTANCE_CLUSTER_DATA_H_ */
