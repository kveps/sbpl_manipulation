////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Andrew Dornbush
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     1. Redistributions of source code must retain the above copyright notice
//        this list of conditions and the following disclaimer.
//     2. Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//     3. Neither the name of the copyright holder nor the names of its
//        contributors may be used to endorse or promote products derived from
//        this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////

/// \author Andrew Dornbush

#ifndef SMPL_VISUALIZE_H
#define SMPL_VISUALIZE_H

// standard includes
#include <chrono>

// system includes
#include <ros/console.h>
#include <visualization_msgs/MarkerArray.h>

// project includes
#include <smpl/time.h>

namespace sbpl {
namespace viz {

namespace levels {
enum Level {
    Invalid = -1,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,

    NumLevels
};
} // namespace levels

class VisualizerBase
{
public:

    virtual ~VisualizerBase() { }

    virtual void visualize(
        levels::Level level,
        const visualization_msgs::MarkerArray& markers) = 0;
};

void set_visualizer(VisualizerBase* visualizer);
void unset_visualizer();
VisualizerBase* visualizer();

void visualize(
    levels::Level level,
    const visualization_msgs::MarkerArray& markers);

} // namespace viz
} // namespace sbpl

#define SBPL_VISUALIZE_SEVERITY_DEBUG   0
#define SBPL_VISUALIZE_SEVERITY_INFO    1
#define SBPL_VISUALIZE_SEVERITY_WARN    2
#define SBPL_VISUALIZE_SEVERITY_ERROR   3
#define SBPL_VISUALIZE_SEVERITY_FATAL   4
#define SBPL_VISUALIZE_SEVERITY_NONE    5

#ifndef SBPL_VISUALIZE_MIN_SEVERITY
#define SBPL_VISUALIZE_MIN_SEVERITY SBPL_VISUALIZE_SEVERITY_DEBUG
#endif

#define SV_SHOW_COND(cond, level, markers) \
    do { \
        if ((cond)) { \
            ::sbpl::viz::visualize(level, markers); \
        } \
    } \
    while (0)

#define SV_SHOW_ONCE(level, markers) \
    do { \
        static bool hit = false; \
        if (!hit) { \
            hit = true; \
            ::sbpl::viz::visualize(level, markers); \
        } \
    } while (0)

#define SV_SHOW_THROTTLE(rate, level, markers) \
    do { \
        static ::sbpl::clock::time_point last_hit; \
        static auto rate_dur = \
                ::std::chrono::duration_cast<::sbpl::clock::duration>( \
                        ::std::chrono::duration<double>(1.0 / (double)rate)); \
        auto now = ::sbpl::clock::now(); \
        if (last_hit + rate_dur <= now) { \
            last_hit = now; \
            ::sbpl::viz::visualize(level, markers); \
        } \
    } while (0)

#define SV_SHOW(level, markers) SV_SHOW_COND(true, level, markers)

#if (SBPL_VISUALIZE_MIN_SEVERITY > SBPL_VISUALIZE_SEVERITY_DEBUG)
#define SV_SHOW_DEBUG(markers)
#define SV_SHOW_DEBUG_COND(cond, markers)
#define SV_SHOW_DEBUG_THROTTLE(rate, markers)
#define SV_SHOW_DEBUG_ONCE(markers)
#else
#define SV_SHOW_DEBUG(markers) SV_SHOW(::sbpl::viz::levels::Debug, markers)
#define SV_SHOW_DEBUG_COND(cond, markers) SV_SHOW_COND(cond, ::sbpl::viz::levels::Debug, markers)
#define SV_SHOW_DEBUG_THROTTLE(rate, markers) SV_SHOW_THROTTLE(rate, ::sbpl::viz::levels::Debug, markers)
#define SV_SHOW_DEBUG_ONCE(markers) SV_SHOW_ONCE(::sbpl::viz::levels::Debug, markers)
#endif

#if (SBPL_VISUALIZE_MIN_SEVERITY > SBPL_VISUALIZE_SEVERITY_INFO)
#define SV_SHOW_INFO(markers)
#define SV_SHOW_INFO_COND(cond, markers)
#define SV_SHOW_INFO_THROTTLE(rate, markers)
#define SV_SHOW_INFO_ONCE(markers)
#else
#define SV_SHOW_INFO(markers) SV_SHOW(::sbpl::viz::levels::Info, markers)
#define SV_SHOW_INFO_COND(cond, markers) SV_SHOW_COND(cond, ::sbpl::viz::levels::Info, markers)
#define SV_SHOW_INFO_THROTTLE(rate, markers) SV_SHOW_THROTTLE(rate, ::sbpl::viz::levels::Info, markers)
#define SV_SHOW_INFO_ONCE(markers) SV_SHOW_ONCE(::sbpl::viz::levels::Info, markers)
#endif

#if (SBPL_VISUALIZE_MIN_SEVERITY > SBPL_VISUALIZE_SEVERITY_WARN)
#define SV_SHOW_WARN(markers)
#define SV_SHOW_WARN_COND(cond, markers)
#define SV_SHOW_WARN_THROTTLE(rate, markers)
#define SV_SHOW_WARN_ONCE(markers)
#else
#define SV_SHOW_WARN(markers) SV_SHOW(::sbpl::viz::levels::Warn, markers)
#define SV_SHOW_WARN_COND(cond, markers) SV_SHOW_COND(cond, ::sbpl::viz::levels::Warn, markers)
#define SV_SHOW_WARN_THROTTLE(rate, markers) SV_SHOW_THROTTLE(rate, ::sbpl::viz::levels::Warn, markers)
#define SV_SHOW_WARN_ONCE(markers) SV_SHOW_ONCE(::sbpl::viz::levels::Warn, markers)
#endif

#if (SBPL_VISUALIZE_MIN_SEVERITY > SBPL_VISUALIZE_SEVERITY_ERROR)
#define SV_SHOW_ERROR(markers)
#define SV_SHOW_ERROR_COND(cond, markers)
#define SV_SHOW_ERROR_THROTTLE(rate, markers)
#define SV_SHOW_ERROR_ONCE(markers)
#else
#define SV_SHOW_ERROR(markers) SV_SHOW(::sbpl::viz::levels::Error, markers)
#define SV_SHOW_ERROR_COND(cond, markers) SV_SHOW_COND(cond, ::sbpl::viz::levels::Error, markers)
#define SV_SHOW_ERROR_THROTTLE(rate, markers) SV_SHOW_THROTTLE(rate, ::sbpl::viz::levels::Error, markers)
#define SV_SHOW_ERROR_ONCE(markers) SV_SHOW_ONCE(::sbpl::viz::levels::Error, markers)
#endif

#if (SBPL_VISUALIZE_MIN_SEVERITY > SBPL_VISUALIZE_SEVERITY_FATAL)
#define SV_SHOW_FATAL(markers)
#define SV_SHOW_FATAL_COND(cond, markers)
#define SV_SHOW_FATAL_THROTTLE(rate, markers)
#define SV_SHOW_FATAL_ONCE(markers)
#else
#define SV_SHOW_FATAL(markers) SV_SHOW(::sbpl::viz::levels::Fatal, markers)
#define SV_SHOW_FATAL_COND(cond, markers) SV_SHOW_COND(cond, ::sbpl::viz::levels::Fatal, markers)
#define SV_SHOW_FATAL_THROTTLE(rate, markers) SV_SHOW_THROTTLE(rate, ::sbpl::viz::levels::Fatal, markers)
#define SV_SHOW_FATAL_ONCE(markers) SV_SHOW_ONCE(::sbpl::viz::levels::Fatal, markers)
#endif

#endif
