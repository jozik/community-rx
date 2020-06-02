/*
 * network_tests.cpp
 *
 *  Created on: Jul 20, 2016
 *      Author: nick
 */
#include <set>
#include "gtest/gtest.h"

#include "repast_hpc/RepastProcess.h"

#include "Network.h"

using namespace crx;

/*
 * Run with mpirun -n 4
 */

TEST(NetworkTests, add_node_edge) {
  if (repast::RepastProcess::instance()->rank() == 0) {
    unsigned int a = 1, b = 2, c = 3, d = 4;

    Network net;
    net.addNode(a, 1, false);
    net.addNode(b, 1, false);
    net.addNode(c, 3, false);
    net.addNode(d, 2, false);

    ASSERT_EQ(4u, net.size());
    ASSERT_EQ(1, net.getRank(a));
    ASSERT_EQ(1, net.getRank(b));
    ASSERT_EQ(3, net.getRank(c));

    net.addEdge(a, b, false);
    net.addEdge(a, c, false);
    net.addEdge(c, b, false);

    std::set<unsigned int> expected { b, c };
    for (auto iter = net.edgesBegin(a); iter != net.edgesEnd(a); ++iter) {
      expected.erase(net.edgeTarget(*iter).id);
      ASSERT_EQ(a, net.edgeSource(*iter).id);
    }
    ASSERT_EQ(0u, expected.size());

    int i = 0;
    for (auto iter = net.edgesBegin(c); iter != net.edgesEnd(c); ++iter) {
      if (net.edgeSource(*iter).id == a) {
        ASSERT_EQ(a, net.edgeSource(*iter).id);
        ASSERT_EQ(c, net.edgeTarget(*iter).id);
      } else {
        ASSERT_EQ(c, net.edgeSource(*iter).id);
        ASSERT_EQ(b, net.edgeTarget(*iter).id);
      }
      ++i;
    }
    ASSERT_EQ(2, i);

    ASSERT_TRUE(net.edgesBegin(d) == net.edgesEnd(d));

    net.removeNode(a, false);
    ASSERT_EQ(3u, net.size());

    unsigned int e = 5, f = 6;
    net.addNode(e, 0, false);
    net.addNode(f, 0, false);
    net.addEdge(e, f, false);

    i = 0;
    for (auto iter = net.edgesBegin(c); iter != net.edgesEnd(c); ++iter) {
      ASSERT_EQ(c, net.edgeSource(*iter).id);
      ASSERT_EQ(b, net.edgeTarget(*iter).id);
      ++i;
    }
    ASSERT_EQ(i, 1);

    i = 0;
    for (auto iter = net.edgesBegin(b); iter != net.edgesEnd(b); ++iter) {
      ASSERT_EQ(c, net.edgeSource(*iter).id);
      ASSERT_EQ(b, net.edgeTarget(*iter).id);
      ++i;
    }
    ASSERT_EQ(i, 1);

    net.removeNode(f, false);
    ASSERT_EQ(4u, net.size());
    ASSERT_TRUE(net.edgesBegin(e) == net.edgesEnd(e));

    net.addNode(f, 0, false);
    net.addEdge(e, f, false);
    net.addEdge(b, e, false);

    i = 0;
    for (auto iter = net.edgesBegin(f); iter != net.edgesEnd(f); ++iter) {
      ASSERT_EQ(e, net.edgeSource(*iter).id);
      ASSERT_EQ(f, net.edgeTarget(*iter).id);
      ++i;
    }
    ASSERT_EQ(i, 1);

    net.removeEdge(e, f, false);
    ASSERT_TRUE(net.edgesBegin(f) == net.edgesEnd(f));

    i = 0;
    for (auto iter = net.edgesBegin(e); iter != net.edgesEnd(e); ++iter) {
      ASSERT_EQ(b, net.edgeSource(*iter).id);
      ASSERT_EQ(e, net.edgeTarget(*iter).id);
      ++i;
    }
    ASSERT_EQ(i, 1);
  }
}

void queueChanges(Network& net, std::vector<unsigned int>& vec) {
  for (size_t i = 0; i < vec.size(); i += 2) {
    net.queueRankChange(vec[i], vec[i + 1]);
  }
}

TEST(NetworkTests, synchronizeRanks) {
  unsigned int a = 1, b = 2, c = 3, d = 4, e = 5;

  Network net;
  net.addNode(a, 1, false);
  net.addNode(b, 1, false);
  net.addNode(c, 3, false);

  net.addNode(d, 2, false);
  net.addNode(e, 3, false);

  int rank = repast::RepastProcess::instance()->rank();
  if (rank == 0) {
    // a and b moved to 2 and 3.
    std::vector<unsigned int> changes{a, 2, b, 3};
    queueChanges(net, changes);
  } else if (rank == 1) {
    // no changes
    std::vector<unsigned int> changes;
    queueChanges(net, changes);
  } else if (rank == 2) {
    std::vector<unsigned int> changes{e, 1};
    queueChanges(net, changes);
  } else {
    std::vector<unsigned int> changes{c, 2, d, 12};
    queueChanges(net, changes);
  }
  net.synchronize();

  ASSERT_EQ(2, net.getRank(a));
  ASSERT_EQ(3, net.getRank(b));
  ASSERT_EQ(2, net.getRank(c));
  ASSERT_EQ(12, net.getRank(d));
  ASSERT_EQ(1, net.getRank(e));
}

TEST(NetworkTests, synchronizeStructure) {
  unsigned int a = 1, b = 2, c = 3, d = 4, e = 5,
      f = 6, g = 7;

  Network net;
  net.addNode(a, 1, false);
  net.addNode(b, 1, false);
  net.addNode(c, 3, false);

  net.addNode(d, 2, false);
  net.addNode(e, 3, false);

  int rank = repast::RepastProcess::instance()->rank();
  if (rank == 0) {
    net.addNode(f, rank, true);
    net.addEdge(f, b, true);
    net.addEdge(a, b, true);
  } else if (rank == 1) {
    net.addNode(g, rank, true);
  } else if (rank == 2) {
    net.addEdge(d, e, true);
    net.addEdge(d, c, true);
  }
  net.synchronize();

  ASSERT_EQ(0, net.getRank(f));
  ASSERT_EQ(1, net.getRank(g));

  int i = 0;
  for (auto iter = net.edgesBegin(a); iter != net.edgesEnd(a); ++iter) {
    ASSERT_EQ(a, net.edgeSource(*iter).id);
    ASSERT_EQ(b, net.edgeTarget(*iter).id);
    ++i;
  }
  ASSERT_EQ(i, 1);

  i = 0;
  for (auto iter = net.edgesBegin(f); iter != net.edgesEnd(f); ++iter) {
    ASSERT_EQ(f, net.edgeSource(*iter).id);
    ASSERT_EQ(b, net.edgeTarget(*iter).id);
    ++i;
  }
  ASSERT_EQ(i, 1);

  std::set<unsigned int> expected { a, f };
  for (auto iter = net.edgesBegin(b); iter != net.edgesEnd(b); ++iter) {
    expected.erase(net.edgeSource(*iter).id);
    ASSERT_EQ(b, net.edgeTarget(*iter).id);
  }

  i = 0;
  for (auto iter = net.edgesBegin(e); iter != net.edgesEnd(e); ++iter) {
    ASSERT_EQ(d, net.edgeSource(*iter).id);
    ASSERT_EQ(e, net.edgeTarget(*iter).id);
    ++i;
  }
  ASSERT_EQ(i, 1);

  expected.insert({ e, c });
  for (auto iter = net.edgesBegin(d); iter != net.edgesEnd(d); ++iter) {
    expected.erase(net.edgeTarget(*iter).id);
  }
  ASSERT_EQ(0u, expected.size());

  ASSERT_TRUE(net.edgesBegin(b) != net.edgesEnd(b));

  if (rank == 0) {
    net.removeEdge(a, b, true);
    net.removeEdge(f, b, true);
  } else if (rank == 2) {
    net.removeNode(d, true);
  }

  net.synchronize();
  // b has no more edges
  ASSERT_TRUE(net.edgesBegin(b) == net.edgesEnd(b));
  // d removed
  ASSERT_EQ(6u, net.size());

  for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
    ASSERT_NE(d, net.edgeSource(*iter).id);
    ASSERT_NE(d, net.edgeTarget(*iter).id);
  }

}

