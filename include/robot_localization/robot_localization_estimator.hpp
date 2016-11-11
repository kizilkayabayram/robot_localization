/*
 * Copyright (c) 2014, 2015, 2016, Charles River Analytics, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ROBOT_LOCALIZATION_LISTENER_H
#define ROBOT_LOCALIZATION_LISTENER_H

#include <Eigen/Dense>
#include <boost/circular_buffer.hpp>
#include <robot_localization/filter_common.hpp>
#include <robot_localization/filter_utilities.hpp>

#include <iostream>

namespace robot_localization
{

//! @brief Robot Localization Estimator State
//!
//! The Estimator State data structure bundles the state information of the
//! estimator.
//!
struct EstimatorState
{
    EstimatorState():
        time_stamp(0.0),
        state(STATE_SIZE),
        covariance(STATE_SIZE,STATE_SIZE)
    {
        state.setZero();
        covariance.setZero();
    }

    //! @brief Time at which this state is/was achieved
    double time_stamp;

    //! @brief System state at time = time_stamp
    Eigen::VectorXd state;

    //! @brief System state covariance at time = time_stamp
    Eigen::MatrixXd covariance;

    friend std::ostream& operator<<(std::ostream &os, const EstimatorState& state)
    {
        return os << "state:\n - time_stamp: " << state.time_stamp <<
                     "\n - state: \n" << state.state <<
                     " - covariance: \n" << state.covariance;
    }
};

//! @brief Robot Localization Listener class
//!
//! The Robot Localization Estimator class buffers states of and inputs to a
//! system and can interpolate and extrapolate based on a given system model.
//!
class RobotLocalizationEstimator
{
  public:
    //! @brief Constructor for the robot_localizationListener class
    //!
    //! @param[in] args - Generic argument container (not used here, but
    //! needed so that the ROS filters can pass arbitrary arguments to
    //! templated filter types).
    //!
    RobotLocalizationEstimator(unsigned int buffer_capacity);

    //! @brief Destructor for the robot_localizationListener class
    //!
    ~RobotLocalizationEstimator();

    //! @brief Sets the current internal state of the listener.
    //!
    //! @param[in] state - The new state vector to set the internal state to
    //!
    void setState(const EstimatorState& state);

    //! @brief Returns the most recent state
    //!
    //! Projects the current state and error matrices forward using a model of
    //! the vehicle's motion.
    //!
    //! @param[out] state - The returned (most recent) state
    //!
    bool getState(EstimatorState &state) const;

    //! @brief Returns the state at a given time
    //!
    //! Projects the current state and error matrices forward using a model of
    //! the vehicle's motion.
    //!
    //! @param[in] time - The time at which the prediction is being made
    //! @param[out] state - The returned state at the given time
    //!
    bool getState(const double time, EstimatorState &state) const;

    //! @brief Clears the internal state buffer
    //!
    void clearBuffer();

    //! @brief Sets the buffer capacity
    //!
    //! @param[in] capacity - The new buffer capacity
    void setBufferCapacity(const int capacity);

    //! @brief Returns the buffer capacity
    //!
    unsigned int capacity() const;

    //! @brief Returns the current buffer size
    //!
    unsigned int size() const;

  private:

    boost::circular_buffer<EstimatorState> state_buffer_;

    friend std::ostream& operator<<(std::ostream &os, const RobotLocalizationEstimator& rle)
    {
      for ( boost::circular_buffer<EstimatorState>::const_iterator it = rle.state_buffer_.begin(); it != rle.state_buffer_.end(); ++it )
      {
        os << *it << "\n";
      }
      return os;
    }

    void extrapolate(const EstimatorState& boundary_state, const double requested_time, EstimatorState& state_at_req_time) const;

    void interpolate(const EstimatorState& given_state_1, const EstimatorState& given_state_2, const double requested_time, EstimatorState& state_at_req_time) const;

    //! @brief As we move through the world, we follow a predict/update
    //! cycle. If one were to imagine a scenario where all we did was make
    //! predictions without correcting, the error in our position estimate
    //! would grow without bound. This error is stored in the
    //! stateEstimateCovariance_ matrix. However, this matrix doesn't answer
    //! the question of *how much* our error should grow for each time step.
    //! That's where the processNoiseCovariance matrix comes in. When we
    //! make a prediction using the transfer function, we add this matrix
    //! (times deltaT) to the state estimate covariance matrix.
    //!
    Eigen::MatrixXd processNoiseCovariance_;

    void wrapStateAngles(EstimatorState state) const
    {
      state.state(StateMemberRoll)  = filter_utilities::clampRotation(state.state(StateMemberRoll));
      state.state(StateMemberPitch) = filter_utilities::clampRotation(state.state(StateMemberPitch));
      state.state(StateMemberYaw)   = filter_utilities::clampRotation(state.state(StateMemberYaw));
    }

};

}  // namespace robot_localization

#endif  // ROBOT_LOCALIZATION_LISTENER_H
