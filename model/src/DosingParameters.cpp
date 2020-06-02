/*
 * DosingParameters.cpp
 *
 *  Created on: Dec 14{0}, 2017
 *      Author: nick
 */

#include "DosingParameters.h"

namespace crx {

DosingParameters::DosingParameters() : decay_{0}, doctor_{0}, nurse_{0}, psr_{0}, resource_{0},
        peer_{0}, new_place_{0} {
}

DosingParameters::Builder::Builder() : params{nullptr} {

}

DosingParameters::Builder& DosingParameters::Builder::decay(float val) {
    if (!params) {
       params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    params->decay_ = val;
    return *this;
}

DosingParameters::Builder& DosingParameters::Builder::doctor(float val) {
    if (!params) {
        params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    params->doctor_ = val;
    return *this;
}

DosingParameters::Builder& DosingParameters::Builder::nurse(float val) {
    if (!params) {
        params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    params->nurse_ = val;
    return *this;
}

DosingParameters::Builder& DosingParameters::Builder::psr(float val) {
    if (!params) {
        params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    params->psr_ = val;
    return *this;
}

DosingParameters::Builder& DosingParameters::Builder::resource(float val) {
    if (!params) {
        params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    params->resource_ = val;
    return *this;
}

DosingParameters::Builder& DosingParameters::Builder::peer(float val) {
    if (!params) {
        params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    params->peer_ = val;
    return *this;
}

DosingParameters::Builder& DosingParameters::Builder::newPlace(float val) {
    if (!params) {
        params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    params->new_place_ = val;
    return *this;
}

std::shared_ptr<DosingParameters>DosingParameters::Builder:: build() {
    if (!params) {
        params = std::shared_ptr<DosingParameters>(new DosingParameters());
    }
    std::shared_ptr<DosingParameters> p = params;
    params = std::shared_ptr<DosingParameters>(nullptr);
    return p;
}


} /* namespace crx */
