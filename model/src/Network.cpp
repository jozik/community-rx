/*
 * Network.cpp
 *
 *  Created on: Jul 19, 2016
 *      Author: nick
 */

#include <stdexcept>
#include <string>
#include <algorithm>

#include "mpi.h"

#include "repast_hpc/RepastProcess.h"

#include "Network.h"

using namespace std;

namespace crx {

enum class UpdateType {NODE_ADD, NODE_REMOVE, EDGE_ADD, EDGE_REMOVE};

NodeData::NodeData(unsigned int id, int rank) : id_rank{id, rank}, edge_list{} {

}

NodeData::~NodeData() {
}

Network::Network() :
		node_map { }, edge_map { }, rank_updates{}, structure_updates{}, edge_idx{0} {
}

Network::~Network() {
	for (auto entry : node_map) {
		delete entry.second;
	}
}

NodeData* Network::findData(unsigned int pid) {
	auto iter = node_map.find(pid);
	if (iter == node_map.end()) {
		throw std::invalid_argument("Network::node_map does not contain " + std::to_string(pid));
	}
	return iter->second;
}

void Network::addNode(unsigned int pid, int rank, bool queue) {
  if (node_map.find(pid) == node_map.end()) {
    node_map.emplace(pid, new NodeData(pid, rank));
  } else {
    throw std::invalid_argument("Network::node_map already contains " + std::to_string(pid));
  }
  if (queue) {
    structure_updates.insert(structure_updates.end(), {static_cast<unsigned int>(UpdateType::NODE_ADD), pid, rank });
  }
}

int Network::getRank(unsigned int pid) {
	return findData(pid)->id_rank.rank;
}

void Network::addEdge(unsigned int source_pid, unsigned int target_pid, bool queue) {
  NodeData* source = findData(source_pid);
  NodeData* target = findData(target_pid);

  bool not_exists = find_if(source->edge_list.begin(), source->edge_list.end(),
      [target_pid, this](unsigned int idx)->bool {return edge_map[idx].second == target_pid;})
      == source->edge_list.end();
  if (not_exists) {
    source->edge_list.push_back(edge_idx);
    if (source_pid != target_pid)
      target->edge_list.push_back(edge_idx);
    edge_map.emplace(edge_idx, make_pair(source_pid, target_pid));
    ++edge_idx;
  }

  if (queue) {
    structure_updates.insert(structure_updates.end(), {
        static_cast<unsigned int>(UpdateType::EDGE_ADD), source_pid, target_pid });
  }
}

void Network::doRemoveEdge(unsigned int edge_idx, unsigned int pid) {
  NodeData* data = findData(pid);
  data->edge_list.erase(remove_if(data->edge_list.begin(), data->edge_list.end(),
      [edge_idx](unsigned int idx) { return edge_idx == idx;}));
}

void Network::removeEdge(unsigned int source_pid, unsigned int target_pid, bool queue) {
  NodeData* data = findData(source_pid);
  for (auto iter = data->edge_list.begin(); iter != data->edge_list.end(); ++iter) {
    if (edge_map.at(*iter).second == target_pid) {
      if (source_pid != target_pid)
        doRemoveEdge(*iter, target_pid);
      data->edge_list.erase(iter);
      break;
    }
  }

  if (queue) {
    structure_updates.insert(structure_updates.end(), {static_cast<unsigned int>(UpdateType::EDGE_REMOVE), source_pid,
      target_pid});
  }
}

void Network::removeNode(unsigned int pid, bool queue) {
  auto iter = node_map.find(pid);
  if (iter != node_map.end()) {
    NodeData* data = iter->second;
    for (unsigned int idx : data->edge_list) {
      auto edge = edge_map.at(idx);
      unsigned int other = edge.first == pid ? edge.second : edge.first;
      doRemoveEdge(idx, other);
      edge_map.erase(idx);
    }

    delete iter->second;
    node_map.erase(iter);
  }

  if (queue) {
    structure_updates.insert(structure_updates.end(), {static_cast<unsigned int>(UpdateType::NODE_REMOVE), pid});
  }
}

void Network::updateRank(unsigned int pid, int rank) {
	findData(pid)->id_rank.rank = rank;
}

ConstPEdgeIdxIter Network::edgesBegin(unsigned int pid) {
	return findData(pid)->edge_list.begin();
}

ConstPEdgeIdxIter Network::edgesEnd(unsigned int pid) {
    return findData(pid)->edge_list.end();
}

const IdRank& Network::edgeSource(unsigned int edge_idx) {
  auto iter = edge_map.find(edge_idx);
  if (iter == edge_map.end()) {
    throw std::invalid_argument("Network::edge_map does not contain edge index" + std::to_string(edge_idx));
  }
  NodeData* data = findData(iter->second.first);
  return data->id_rank;
}

const IdRank& Network::edgeTarget(unsigned int edge_idx) {
  auto iter = edge_map.find(edge_idx);
  if (iter == edge_map.end()) {
    throw std::invalid_argument(
        "Network::edge_map does not contain edge index" + std::to_string(edge_idx));
  }
  NodeData* data = findData(iter->second.second);
  return data->id_rank;
}

ConstAEdgeIdxIter Network::edgesBegin() {
  return boost::make_transform_iterator(edge_map.begin(), KeyExtractor());
}

ConstAEdgeIdxIter Network::edgesEnd() {
  return boost::make_transform_iterator(edge_map.end(), KeyExtractor());
}

size_t init_synchronize(int* counts, int* disps, boost::mpi::communicator* comm, int world_size, int count) {
  MPI_Allgather(&count, 1, MPI_INT, counts, 1, MPI_INT, *comm);

  disps[0] = 0;
  size_t counts_sum = 0;
  for (int i = 1; i < world_size; ++i) {
    int count = counts[i - 1];
    counts_sum += count;
    disps[i] = disps[i - 1] + count;
  }
  counts_sum += counts[world_size - 1];
  return counts_sum;
}

void Network::processStructureBuffer(unsigned int* recv_buf, size_t &i) {
  UpdateType ut = static_cast<UpdateType>(recv_buf[i]);
  if (ut == UpdateType::NODE_ADD) {
    addNode(recv_buf[i + 1], recv_buf[i + 2], false);
  } else if (ut == UpdateType::NODE_REMOVE) {
    removeNode(recv_buf[i + 1], false);
    // need to rewind i because we only need to jump
    // by 2 next time rather than 3
    --i;
  } else if (ut == UpdateType::EDGE_ADD) {
    addEdge(recv_buf[i + 1], recv_buf[i + 2], false);
  } else {
    removeEdge(recv_buf[i + 1], recv_buf[i + 2], false);
  }

}

void Network::synchronizeStructure(int* counts, int* disps, boost::mpi::communicator* comm) {
  size_t counts_sum = init_synchronize(counts, disps, comm, comm->size(), structure_updates.size());
  if (counts_sum > 0) {
    unsigned int* recv_buf = new unsigned int[counts_sum];
    MPI_Allgatherv(&structure_updates[0], structure_updates.size(), MPI_UNSIGNED, recv_buf, counts,
        disps, MPI_UNSIGNED, *comm);

    int rank = comm->rank();
    // the node adds from a process should be
    // before the edge adds. So we can do this
    // sequentially rather than breaking out the edge
    // adds for later
    // skip the updates from this rank
    if (rank == 0) {
      for (size_t i = disps[1]; i < counts_sum; i += 3) {
        processStructureBuffer(recv_buf, i);
      }
    } else if (rank == comm->size() - 1) {
      for (size_t i = 0; i < disps[comm->size() - 1]; i += 3) {
        processStructureBuffer(recv_buf, i);
      }
    } else {

      for (size_t i = 0; i < disps[rank]; i += 3) {
        processStructureBuffer(recv_buf, i);
      }

      for (size_t i = disps[rank + 1]; i < counts_sum; i += 3) {
        processStructureBuffer(recv_buf, i);
      }
    }

    delete[] recv_buf;
    structure_updates.clear();
  }
}

void Network::synchronizeRanks(int* counts, int* disps, boost::mpi::communicator* comm) {
  size_t counts_sum = init_synchronize(counts, disps, comm, comm->size(), rank_updates.size());
  if (counts_sum > 0) {
    unsigned int* recv_buf = new unsigned int[counts_sum];
    MPI_Allgatherv(&rank_updates[0], rank_updates.size(), MPI_UNSIGNED, recv_buf, counts, disps,
    MPI_UNSIGNED, *comm);

    for (size_t i = 0; i < counts_sum; i += 2) {
      updateRank(recv_buf[i], (int) recv_buf[i + 1]);
    }

    delete[] recv_buf;
    rank_updates.clear();
  }
}

void Network::synchronize() {
  boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();
  int world_size = repast::RepastProcess::instance()->worldSize();
  int* counts = new int[world_size];
  int* disps = new int[world_size];

  synchronizeStructure(counts, disps, comm);
  synchronizeRanks(counts, disps, comm);

  delete[] counts;
  delete[] disps;
}

void Network::queueRankChange(unsigned int pid, int rank) {
  rank_updates.push_back(pid);
  rank_updates.push_back(rank);

}

} /* namespace crx */
