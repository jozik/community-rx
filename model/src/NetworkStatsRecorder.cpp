/*
 * NetworkStatsRecorder.cpp
 *
 *  Created on: May 17, 2018
 *      Author: nick
 */

#include <stdexcept>
#include <iostream>

#include "chi_sim/file_utils.h"

#include "NetworkStatsRecorder.h"

namespace crx {

NetworkStatsRecorder* NetworkStatsRecorder::instance_ = nullptr;

NSRecorder::NSRecorder(const std::string& fname) :
        out(nullptr), pid_(0), written(false), fname_(fname) {

}

NSRecorder::~NSRecorder() {
    close();

}

void NSRecorder::close() {
    if (out) {
        out->flush();
        out->close();
        delete out;
        out = nullptr;
    }
}

NetworkStatsRecorder::~NetworkStatsRecorder() {}

void NetworkStatsRecorder::initialize(const std::string& fname) {
    //std::cout << instance_ << std::endl;
    if (instance_) {
        delete instance_;
    }

    //std::cout << "fname: " << fname << std::endl;
    if (fname.size() > 0) {
        //std::cout << "using ns" << std::endl;
        instance_ = new NSRecorder(fname);
    } else {
        //std::cout << "using null" << std::endl;
        instance_ = new NullNetworkStatsRecorder();
    }
}

NetworkStatsRecorder* NetworkStatsRecorder::instance() {
    if (!instance_) {
        throw std::domain_error("NetworkStatsRecorder must be initialized before use");
    }
    return instance_;
}

void NSRecorder::initInfoNetworkRecording(double tick) {
    if (out) {
        close();
    }

    std::string fname = chi_sim::insert_in_file_name(fname_, (int) tick);
    out = new FileOut(fname);
    (*out) << "T" << tick << "\n";
}

void NSRecorder::initInfoNetworkRecordingFor(unsigned int pid) {
    pid_ = pid;

}

void NSRecorder::addInfoNetworkLink(unsigned int other) {
    if (written) {
        (*out) << "," << other;
    } else {
        (*out) << pid_ << ":" << other;
    }
    written = true;
}

void NSRecorder::finalizeInfoNetworkRecording() {
    if (written) {
        (*out) << "\n";
    }
    written = false;
}


PlaceEdgeRecorder* PlaceEdgeRecorder::instance_ = nullptr;

void PlaceEdgeRecorder::initialize(const std::string& fname) {
    if (instance_) {
        delete instance_;
    }

    if (fname.size() > 0) {
        instance_ = new PERecorder(fname);
    } else {
        instance_ = new NullPERecorder();
    }
}

PlaceEdgeRecorder* PlaceEdgeRecorder::instance() {
    if (!instance_) {
        throw std::domain_error("PlaceEdgeRecorder must be initialized before use");
    }
    return instance_;
}


PERecorder::PERecorder(const std::string& fname) :
        out(nullptr), fname_(fname) {
}

PERecorder::~PERecorder() {
    close();
}

void PERecorder::initRecording(double tick) {
    if (out) {
        close();
    }

    std::string fname = chi_sim::insert_in_file_name(fname_, (int) tick);
    out = new FileOut(fname);
}

void PERecorder::recordEdge(int current_place, int next_place, unsigned person_id) {
    (*out) << current_place << "," << next_place << "," << person_id << "\n";
}

void PERecorder::close() {
    if (out) {
        out->flush();
        out->close();
        delete out;
        out = nullptr;
    }
}

} /* namespace crx */
