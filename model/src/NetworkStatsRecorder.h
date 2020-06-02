/*
 * NetworkStatsRecorder.h
 *
 *  Created on: May 17, 2018
 *      Author: nick
 */

#ifndef NETWORKSTATSRECORDER_H_
#define NETWORKSTATSRECORDER_H_

#include "FileOut.h"

namespace crx {


class NetworkStatsRecorder {

private:
    static NetworkStatsRecorder* instance_;

public:

    static void initialize(const std::string& fname);
    static NetworkStatsRecorder* instance();
    virtual ~NetworkStatsRecorder();

    virtual void initInfoNetworkRecording(double tick) = 0;
    virtual void initInfoNetworkRecordingFor(unsigned int pid) = 0;
    virtual void addInfoNetworkLink(unsigned int pid) = 0;
    virtual void finalizeInfoNetworkRecording() = 0;
    virtual void close() = 0;
};

class NSRecorder : public NetworkStatsRecorder {

private:

    friend class NetworkStatsRecorder;

    FileOut* out;
    unsigned int pid_;
    bool written;
    std::string fname_;

    NSRecorder(const std::string& fname);

public:

    virtual ~NSRecorder();

    virtual void initInfoNetworkRecording(double tick);
    virtual void initInfoNetworkRecordingFor(unsigned int pid);
    virtual void addInfoNetworkLink(unsigned int pid);
    virtual void finalizeInfoNetworkRecording();
    virtual void close();
};


class NullNetworkStatsRecorder : public NetworkStatsRecorder {

private:
    friend class NetworkStatsRecorder;
    NullNetworkStatsRecorder() {}

public:
    virtual ~NullNetworkStatsRecorder() {}
    void initInfoNetworkRecording(double tick) {}
    void initInfoNetworkRecordingFor(unsigned int pid) {}
    void addInfoNetworkLink(unsigned int pid) {}
    void finalizeInfoNetworkRecording() {}
    void close() {}

};


class PlaceEdgeRecorder {

private:
    static PlaceEdgeRecorder* instance_;

public:

    static void initialize(const std::string& fname);
    static PlaceEdgeRecorder* instance();
    virtual ~PlaceEdgeRecorder() {}

    virtual void recordEdge(int current_place, int next_place, unsigned person_id) = 0;
    virtual void close() = 0;
    virtual void initRecording(double tick) = 0;
};

class PERecorder : public PlaceEdgeRecorder {

private:

    friend class PlaceEdgeRecorder;

    FileOut* out;
    std::string fname_;

    PERecorder(const std::string& fname);

public:

    virtual ~PERecorder();

    void recordEdge(int current_place, int next_place, unsigned person_id) override;
    void close() override;
    void initRecording(double tick) override;
};


class NullPERecorder : public PlaceEdgeRecorder {

private:
    friend class PlaceEdgeRecorder;
    NullPERecorder() {}

public:
    virtual ~NullPERecorder() {}
    void recordEdge(int current_place, int next_place, unsigned person_id) {}
    void close() {}
    void initRecording(double tick) {}
};



} /* namespace crx */

#endif /* NETWORKSTATSRECORDER_H_ */
