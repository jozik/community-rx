/*
 * DosingParameters.h
 *
 *  Created on: Dec 14, 2017
 *      Author: nick
 */

#ifndef DOSINGPARAMETERS_H_
#define DOSINGPARAMETERS_H_

#include <memory>

namespace crx {

class DosingParameters {

private:
    float decay_, doctor_, nurse_, psr_, resource_, peer_, new_place_;
    DosingParameters();

public:

    class Builder;

    const float decay() const {
        return decay_;
    }

    const float doctor() const {
        return doctor_;
    }

    const float nurse() const {
        return nurse_;
    }

    const float psr() const {
        return psr_;
    }

    const float resource() const {
        return resource_;
    }

    const float peer() const {
        return peer_;
    }

    const float newPlace() const {
        return new_place_;
    }

};

class DosingParameters::Builder {

private:
    std::shared_ptr<DosingParameters> params;

public:
    Builder();

    Builder& decay(float val);

    Builder& doctor(float val);

    Builder& nurse(float val);

    Builder& psr(float val);

    Builder& resource(float val);

    Builder& peer(float val);

    Builder& newPlace(float val);

    std::shared_ptr<DosingParameters> build();
};

} /* namespace crx */

#endif /* DOSINGPARAMETERS_H_ */
