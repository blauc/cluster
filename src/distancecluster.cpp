#include "distancecluster.h"
#include <numeric>
DistanceCluster::DistanceCluster(const std::vector<DistanceCluster::DistanceType> &dist_to_following, const std::vector<size_t> &elements, DistanceCluster::DistanceType merge_d)
    : elements_(elements), dist_to_following_(dist_to_following), merge_d_(merge_d)
{}

DistanceCluster::
    DistanceCluster(std::vector<DistanceType> &&dist_to_following, std::vector<size_t> &&elements, DistanceType merge_d)
    : elements_ {std::move(elements)}, dist_to_following_ {
    std::move(dist_to_following)
}, merge_d_(merge_d)
{
}

std::size_t
DistanceCluster::size() const
{
    return elements_.size();
}

std::unique_ptr<DistanceCluster>
DistanceCluster::merger(const DistanceCluster &other, std::vector<DistanceCluster::DistanceType> dist_to_following, DistanceCluster::DistanceType merge_d)
{
    std::vector<size_t> elements(elements_);
    elements.insert(elements.end(), other.elements_.begin(), other.elements_.end());
    return std::unique_ptr<DistanceCluster>(new DistanceCluster(dist_to_following, elements, merge_d));
}

const std::vector<DistanceCluster::DistanceType> &DistanceCluster::distances() const
{
    return dist_to_following_;
}

void DistanceCluster::deleteDistance(size_t index)
{
    dist_to_following_.erase(dist_to_following_.begin()+index);
}

const std::vector<size_t> &
DistanceCluster::elements() const
{
    return elements_;
}

std::string DistanceCluster::print() const
{
    std::string result;
    result = "{\n\t \"merge-distance\": " + std::to_string(merge_d_) + ",\n"
        "\t \"size\" : "+
        std::to_string(elements_.size())+ ",\n" +
        "\t \"elements\" :[" +
        std::accumulate(begin(elements_), --end(elements_), std::string(""), [](std::string &b, std::size_t a ) {return b + std::to_string(a) + ","; } ) +
        std::to_string(elements_.back()) + "]\n}\n";
    return result;
}
