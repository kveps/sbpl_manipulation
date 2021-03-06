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

#include <smpl/heuristic/bfs_heuristic.h>

// system includes
#include <leatherman/viz.h>

// project includes
#include <smpl/bfs3d/bfs3d.h>
#include <smpl/intrusive_heap.h>
#include <smpl/grid.h>

namespace sbpl {
namespace motion {

BfsHeuristic::BfsHeuristic(
    const RobotPlanningSpacePtr& ps,
    const OccupancyGrid* grid)
:
    RobotHeuristic(ps, grid),
    m_bfs(),
    m_goal_x(-1),
    m_goal_y(-1),
    m_goal_z(-1)
{
    m_pp = ps->getExtension<PointProjectionExtension>();
    if (m_pp) {
        ROS_INFO_NAMED(params()->heuristic_log, "Got Point Projection Extension!");
    }
    syncGridAndBfs();
}

BfsHeuristic::~BfsHeuristic()
{
    // empty to allow forward declaration of BFS_3D
}

void BfsHeuristic::updateGoal(const GoalConstraint& goal)
{
    int gx, gy, gz;
    grid()->worldToGrid(
            goal.tgt_off_pose[0], goal.tgt_off_pose[1], goal.tgt_off_pose[2],
            gx, gy, gz);

    ROS_DEBUG_NAMED(params()->heuristic_log, "Setting the BFS heuristic goal (%d, %d, %d)", gx, gy, gz);

    if (!m_bfs->inBounds(gx, gy, gz)) {
        ROS_ERROR_NAMED(params()->heuristic_log, "Heuristic goal is out of BFS bounds");
    }

    m_goal_x = gx;
    m_goal_y = gy;
    m_goal_z = gz;

    m_bfs->run(gx, gy, gz);
}

double BfsHeuristic::getMetricStartDistance(double x, double y, double z)
{
    int start_id = planningSpace()->getStartStateID();

    if (!m_pp) {
        return 0.0;
    }

    Eigen::Vector3d p;
    if (!m_pp->projectToPoint(planningSpace()->getStartStateID(), p)) {
        return 0.0;
    }

    int sx, sy, sz;
    grid()->worldToGrid(p.x(), p.y(), p.z(), sx, sy, sz);

    int gx, gy, gz;
    grid()->worldToGrid(x, y, z, gx, gy, gz);

    // compute the manhattan distance to the start cell
    const int dx = sx - gx;
    const int dy = sy - gy;
    const int dz = sz - gz;
    return grid()->resolution() * (abs(dx) + abs(dy) + abs(dz));
}

double BfsHeuristic::getMetricGoalDistance(double x, double y, double z)
{
    int gx, gy, gz;
    grid()->worldToGrid(x, y, z, gx, gy, gz);
    if (!m_bfs->inBounds(gx, gy, gz)) {
        return (double)BFS_3D::WALL * grid()->resolution();
    } else {
        return (double)m_bfs->getDistance(gx, gy, gz) * grid()->resolution();
    }
}

Extension* BfsHeuristic::getExtension(size_t class_code)
{
    if (class_code == GetClassCode<RobotHeuristic>()) {
        return this;
    }
    return nullptr;
}

int BfsHeuristic::GetGoalHeuristic(int state_id)
{
    if (!m_pp) {
        return 0;
    }

    Eigen::Vector3d p;
    if (!m_pp->projectToPoint(state_id, p)) {
        return 0;
    }

    Eigen::Vector3i dp;
    grid()->worldToGrid(p.x(), p.y(), p.z(), dp.x(), dp.y(), dp.z());

    return getBfsCostToGoal(*m_bfs, dp.x(), dp.y(), dp.z());
}

int BfsHeuristic::GetStartHeuristic(int state_id)
{
    ROS_WARN_ONCE("BfsHeuristic::GetStartHeuristic unimplemented");
    return 0;
}

int BfsHeuristic::GetFromToHeuristic(int from_id, int to_id)
{
    if (to_id == planningSpace()->getGoalStateID()) {
        return GetGoalHeuristic(from_id);
    }
    else {
        ROS_WARN_ONCE("BfsHeuristic::GetFromToHeuristic unimplemented for arbitrary state pair");
        return 0;
    }
}

visualization_msgs::MarkerArray BfsHeuristic::getWallsVisualization() const
{
    std::vector<geometry_msgs::Point> points;
    int dimX = grid()->numCellsX();
    int dimY = grid()->numCellsY();
    int dimZ = grid()->numCellsZ();
    for (int z = 0; z < dimZ; z++) {
        for (int y = 0; y < dimY; y++) {
            for (int x = 0; x < dimX; x++) {
                if (m_bfs->isWall(x, y, z)) {
                    geometry_msgs::Point p;
                    grid()->gridToWorld(x, y, z, p.x, p.y, p.z);
                    points.push_back(p);
                }
            }
        }
    }

    ROS_DEBUG_NAMED(params()->heuristic_log, "BFS Visualization contains %zu points", points.size());

    std_msgs::ColorRGBA color;
    color.r = 100.0f / 255.0f;
    color.g = 149.0f / 255.0f;
    color.b = 238.0f / 255.0f;
    color.a = 1.0f;

    visualization_msgs::Marker cubes_marker = viz::getCubesMarker(
            points,
            grid()->resolution(),
            color,
            grid()->getReferenceFrame(),
            "bfs_walls",
            0);

    visualization_msgs::MarkerArray ma;
    ma.markers.push_back(std::move(cubes_marker));
    return ma;
}

visualization_msgs::MarkerArray BfsHeuristic::getValuesVisualization()
{
    visualization_msgs::MarkerArray ma;

    if (m_goal_x < 0 || m_goal_y < 0 || m_goal_z < 0) {
        return ma;
    }

    if (m_bfs->isWall(m_goal_x, m_goal_y, m_goal_z)) {
        return ma;
    }

    // hopefully this doesn't screw anything up too badly...this will flush the
    // bfs to a little past the start, but this would be done by the search
    // hereafter anyway
    int start_heur = GetGoalHeuristic(planningSpace()->getStartStateID());
    if (start_heur == Infinity) {
        return ma;
    }

    ROS_INFO("Start cell heuristic: %d", start_heur);

    const int max_cost = (int)(1.1 * start_heur);

    ROS_INFO("Get visualization of cells up to cost %d", max_cost);

    // ...and this will also flush the bfs...

    const size_t max_points = 4 * 4096;

    std::vector<geometry_msgs::Point> points;
    std::vector<std_msgs::ColorRGBA> colors;

    struct CostCell
    {
        int x, y, z, g;
    };
    std::queue<CostCell> cells;
    Grid3<bool> visited(grid()->numCellsX(), grid()->numCellsY(), grid()->numCellsZ(), false);
    visited(m_goal_x, m_goal_y, m_goal_z) = true;
    cells.push({m_goal_x, m_goal_y, m_goal_z, 0});
    while (!cells.empty()) {
        CostCell c = cells.front();
        cells.pop();

        if (c.g > max_cost || points.size() >= max_points) {
            break;
        }

        {
            double cost_pct = (double)c.g / (double)max_cost;

            double hue = 300.0 - 300.0 * cost_pct;
            double sat = 1.0;
            double val = 1.0;
            double r, g, b;
            leatherman::HSVtoRGB(&r, &g, &b, hue, sat, val);

            std_msgs::ColorRGBA color;
            color.r = (float)r;
            color.g = (float)g;
            color.b = (float)b;
            color.a = 1.0f;

            auto clamp = [](double d, double lo, double hi) {
                if (d < lo) {
                    return lo;
                } else if (d > hi) {
                    return hi;
                } else {
                    return d;
                }
            };

            color.r = clamp(color.r, 0.0f, 1.0f);
            color.g = clamp(color.g, 0.0f, 1.0f);
            color.b = clamp(color.b, 0.0f, 1.0f);

            geometry_msgs::Point p;
            grid()->gridToWorld(c.x, c.y, c.z, p.x, p.y, p.z);
            points.push_back(p);

            colors.push_back(color);
        }

//        visited(c.x, c.y, c.z) = true;

        const int d = params()->cost_per_cell * m_bfs->getDistance(c.x, c.y, c.z);

        for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
        for (int dz = -1; dz <= 1; ++dz) {
            if (!(dx | dy | dz)) {
                continue;
            }

            int sx = c.x + dx;
            int sy = c.y + dy;
            int sz = c.z + dz;

            // check if neighbor is valid
            if (!m_bfs->inBounds(sx, sy, sz) || m_bfs->isWall(sx, sy, sz)) {
                continue;
            }

            // check if cost can be improved
            if (visited(sx, sy, sz)) {
                continue;
            }

            visited(sx, sy, sz) = true;

            int dd = params()->cost_per_cell * m_bfs->getDistance(sx, sy, sz);
            cells.push({sx, sy, sz, dd});
        }
        }
        }
    }

    visualization_msgs::Marker marker;
    marker.header.stamp = ros::Time(0);
    marker.header.frame_id = grid()->getReferenceFrame();
    marker.ns = "bfs_values";
    marker.id = 0;
    marker.type = visualization_msgs::Marker::CUBE_LIST;
    marker.action = visualization_msgs::Marker::ADD;
    marker.pose.orientation.w = 1.0;
    marker.scale.x = 0.5 * grid()->resolution();
    marker.scale.y = 0.5 * grid()->resolution();
    marker.scale.z = 0.5 * grid()->resolution();
//    marker.color;
    marker.lifetime = ros::Duration(0.0);
    marker.frame_locked = false;
    marker.points = std::move(points);
    marker.colors = std::move(colors);
    marker.text = "";
    marker.mesh_use_embedded_materials = false;

    ma.markers.push_back(std::move(marker));
    return ma;
}

void BfsHeuristic::syncGridAndBfs()
{
    const int xc = grid()->numCellsX();
    const int yc = grid()->numCellsY();
    const int zc = grid()->numCellsZ();
//    ROS_DEBUG_NAMED(params()->heuristic_log_, "Initializing BFS of size %d x %d x %d = %d", xc, yc, zc, xc * yc * zc);
    m_bfs.reset(new BFS_3D(xc, yc, zc));
    const int cell_count = xc * yc * zc;
    int wall_count = 0;
    for (int z = 0; z < zc; ++z) {
        for (int y = 0; y < yc; ++y) {
            for (int x = 0; x < xc; ++x) {
                const double radius = params()->planning_link_sphere_radius;
                if (grid()->getDistance(x, y, z) <= radius) {
                    m_bfs->setWall(x, y, z);
                    ++wall_count;
                }
            }
        }
    }

    ROS_DEBUG_NAMED(params()->heuristic_log, "%d/%d (%0.3f%%) walls in the bfs heuristic", wall_count, cell_count, 100.0 * (double)wall_count / cell_count);
}

int BfsHeuristic::getBfsCostToGoal(const BFS_3D& bfs, int x, int y, int z) const
{
    if (!bfs.inBounds(x, y, z)) {
        return Infinity;
    }
    else if (bfs.getDistance(x, y, z) == BFS_3D::WALL) {
        return Infinity;
    }
    else {
        return params()->cost_per_cell * bfs.getDistance(x, y, z);
    }
}

} // namespace motion
} // namespace sbpl
