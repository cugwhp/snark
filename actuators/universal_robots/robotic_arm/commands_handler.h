// This file is part of snark, a generic and flexible library for robotics research
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SNARK_ACTUATORS_UNIVERISAL_ROBOTS_COMMANDS_HANDLER_H
#define SNARK_ACTUATORS_UNIVERISAL_ROBOTS_COMMANDS_HANDLER_H
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <comma/base/types.h>
#include <comma/dispatch/dispatched.h>
#include <comma/io/stream.h>
#include <comma/io/select.h>
#include <comma/base/exception.h>
#include <comma/application/signal_flag.h>
#include <boost/optional.hpp>
#include "data.h"
#include "commands.h"
#include "auto_initialization.h"
#include "camera_sweep.h"
#include "output.h"
#include <boost/filesystem.hpp>

namespace snark { namespace ur { namespace robotic_arm { namespace handlers {

namespace arm = robotic_arm;
namespace fs = boost::filesystem;


class commands_handler : public comma::dispatch::handler_of< power >,
                                  public comma::dispatch::handler_of< brakes >,
                                  public comma::dispatch::handler_of< set_home >,
                                  public comma::dispatch::handler_of< auto_init >,
                                  public comma::dispatch::handler_of< move_cam >,
                                  public comma::dispatch::handler_of< move_joints >,
                                  public comma::dispatch::handler_of< joint_move >,
                                  public comma::dispatch::handler_of< set_position >,
                                  public comma::dispatch::handler_of< auto_init_force >,
                                  public comma::dispatch::handler_of< sweep_cam >,
                                  public comma::dispatch::handler_of< move_effector >
{
public:
    void handle( power& p );
    void handle( brakes& b );
    void handle( auto_init& a );
    void handle( move_cam& c );
    void handle( move_effector& e );
    void handle( move_joints& js );
    void handle( set_home& h );
    void handle( set_position& p );
    void handle( auto_init_force& p );
    void handle( joint_move& j );
    void handle( sweep_cam& s );
    
    commands_handler( ExtU_Arm_Controller_T& simulink_inputs, const arm_output& output,
                      arm::status_t& status, std::ostream& robot, 
                      auto_initialization& init, camera_sweep& sweep ) : 
        inputs_(simulink_inputs), output_(output), 
        status_( status ), os( robot ), 
        init_(init), sweep_( sweep ),
        home_filepath_( init_.home_filepath() ), verbose_(false) {}
        
    bool is_initialising() const; 
    
    result ret;  /// Indicate if command succeed
    boost::optional< length_t > move_cam_height_;
    plane_angle_degrees_t move_cam_pan_;
//     length_t height_;   /// Last height set by move_cam
private:
    ExtU_Arm_Controller_T& inputs_; /// inputs into simulink engine 
    const arm_output& output_;
    status_t& status_;
    std::ostream& os;
    auto_initialization& init_;
    camera_sweep sweep_;
    fs::path home_filepath_;
    bool verbose_;
    
    void inputs_reset() { inputs_.motion_primitive = input_primitive::no_action; }
    /// Run the command on the controller if possible
    bool execute();
    
};

} } } } // namespace snark { namespace ur { namespace robotic_arm { namespace handlers {


#endif // SNARK_ACTUATORS_UNIVERISAL_ROBOTS_COMMANDS_HANDLER_H