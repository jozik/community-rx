/*
 * OutputWriter
 *
 *  Created on: Dec 15, 2017
 *      Author: nick
 */

#ifndef CONCATENATINGDATARECORDER_H_
#define CONCATENATINGDATARECORDER_H_

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>

#include "mpi.h"

#include "repast_hpc/RepastProcess.h"

#include "FileOut.h"
#include "Stats.h"

namespace crx {


template<typename T>
class ConcatenatingDataRecorder {

private:
    int rank_;
    int world_size;
    MPI_Datatype data_type;
    std::shared_ptr<FileOut> out;
    std::vector<T> items;

public:

    template <typename MakeMPIDataType>
    ConcatenatingDataRecorder(const std::string& file_name, const std::string& header, int rank, MakeMPIDataType op);
    ~ConcatenatingDataRecorder();

    template <typename F>
    void write(F fwrite);
    void record(const T& item);
    void close();
    void flush();
};

template<typename T>
template<typename MakeMPIDataType>
ConcatenatingDataRecorder<T>::ConcatenatingDataRecorder(const std::string& file_name, const std::string& header, int rank, MakeMPIDataType op) : rank_(rank),
world_size(0), data_type(), out(nullptr), items{} {

    op(&data_type);
    boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();
    MPI_Comm_size(*comm, &world_size);
    if (rank_ == 0) {
        out = std::make_shared<FileOut>(file_name);
        (*out) << header << "\n";
    }
}

template<typename T>
template <typename F>
void ConcatenatingDataRecorder<T>::write(F fwrite) {
    int size = items.size();
    boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();
    if (rank_ == 0) {
        std::vector<int> counts(world_size, 0);
        MPI_Gather(&size, 1, MPI_INT, &counts[0], 1, MPI_INT, 0, *comm);

        int* displacements = new int[world_size];
        displacements[0] = 0;
        for (size_t i = 1; i < counts.size(); ++i) {
            displacements[i] = displacements[i - 1] + counts[i - 1];
        }

        int recv_count = std::accumulate(counts.begin(), counts.end(), 0);
        std::vector<T> recv_buf;
        recv_buf.resize(recv_count);
        MPI_Gatherv(&items[0], size, data_type, &recv_buf[0],  &counts[0], displacements, data_type, 0, *comm);
        delete[] displacements;

        for (auto& item : recv_buf) {
            fwrite(*out, item);
        }

    } else {
        MPI_Gather(&size, 1, MPI_INT, nullptr, 1, MPI_INT, 0, *comm);
        MPI_Gatherv(&items[0], size, data_type, nullptr, nullptr, nullptr, data_type, 0, *comm);
    }
    items.clear();
}

template<typename T>
void ConcatenatingDataRecorder<T>::record(const T& item) {
    items.push_back(item);
}


template<typename T>
void ConcatenatingDataRecorder<T>::flush() {
    if (out) {
        out->flush();
    }
}

template<typename T>
void ConcatenatingDataRecorder<T>::close() {
    if (out) {
        out->close();
        MPI_Type_free(&data_type);
    }
}

template<typename T>
ConcatenatingDataRecorder<T>::~ConcatenatingDataRecorder() {

}

} /* namespace crx */

#endif /* CONCATENATINGDATARECORDER_H_ */
