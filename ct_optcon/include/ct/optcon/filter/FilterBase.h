/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

#pragma once

namespace ct {
namespace optcon {

template <size_t OBS_DIM, size_t STATE_DIM, typename SCALAR = double>
class FilterBase
{
public:
    using state_vector_t  = ct::core::StateVector<STATE_DIM, SCALAR>;
    using output_vector_t = ct::core::OutputVector<OBS_DIM, SCALAR>;
    using Time_t          = ct::core::Time;

    virtual ~FilterBase() {}
    virtual state_vector_t filter(const output_vector_t& y, const Time_t& t) = 0;
};

}  // optcon
}  // ct
