/*
 * Network.h
 *
 *  Created on: Jul 19, 2016
 *      Author: nick
 */

#ifndef SRC_NETWORK_H_
#define SRC_NETWORK_H_

#include <utility>
#include <map>
#include <vector>

#include <boost/mpi/communicator.hpp>
#include "boost/iterator/transform_iterator.hpp"

namespace crx {

using Edge = std::pair<unsigned int, unsigned int>;

struct KeyExtractor {
    const unsigned int& operator()(const std::pair<unsigned int, Edge>& p) const {
      return p.first;
    }
};

struct IdRank {
    unsigned int id;
    int rank;
};

struct NodeData {

  IdRank id_rank;
  // keys in edge_map
  std::vector<unsigned int> edge_list;

  NodeData(unsigned int id, int rank);
  virtual ~NodeData();

};

using ConstPEdgeIdxIter = std::vector<unsigned int>::const_iterator;
using ConstAEdgeIdxIter = boost::transform_iterator<KeyExtractor, std::map<unsigned int, Edge>::const_iterator>;

class Network {

private:
    // id -> NodeData
	std::map<unsigned int, NodeData*> node_map;
	std::map<unsigned int, std::pair<unsigned int, unsigned int>> edge_map;

	std::vector<unsigned int> rank_updates;
	std::vector<unsigned int> structure_updates;

	unsigned int edge_idx;

	NodeData* findData(unsigned int pid);

	void updateRank(unsigned int pid, int rank);

	void synchronizeRanks(int* counts, int* disps, boost::mpi::communicator* comm);
	void synchronizeStructure(int* counts, int* disps, boost::mpi::communicator* comm);
	void doRemoveEdge(unsigned int edge_idx, unsigned int pid);
	void processStructureBuffer(unsigned int* recv_buf, size_t &i);

public:
	Network();
	virtual ~Network();

	void addNode(unsigned int pid, int rank, bool queue);

	void removeNode(unsigned int pid, bool queue);

	void addEdge(unsigned int source_pid, unsigned int target_pid, bool queue);

	void removeEdge(unsigned int source_pid, unsigned int target_pid, bool queue);

	ConstAEdgeIdxIter edgesBegin();
	ConstAEdgeIdxIter edgesEnd();

	int getRank(unsigned int pid);

	ConstPEdgeIdxIter edgesBegin(unsigned int pid);

	ConstPEdgeIdxIter edgesEnd(unsigned int pid);

	const IdRank& edgeSource(unsigned int edge_idx);

	const IdRank& edgeTarget(unsigned int edge_idx);

	void synchronize();
	void queueRankChange(unsigned int pid, int rank);

	size_t size() const {
		return node_map.size();
	}
};

} /* namespace crx */

#endif /* SRC_NETWORK_H_ */
