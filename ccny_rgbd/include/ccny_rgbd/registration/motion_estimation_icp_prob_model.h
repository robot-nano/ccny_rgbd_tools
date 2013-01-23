/*
 *  Copyright (C) 2013, City University of New York
 *  Ivan Dryanovski <ivan.dryanovski@gmail.com>
 *
 *  CCNY Robotics Lab
 *  http://robotics.ccny.cuny.edu
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CCNY_RGBD_MOTION_ESTIMATION_ICP_PROB_MODEL_H
#define CCNY_RGBD_MOTION_ESTIMATION_ICP_PROB_MODEL_H

#include <tf/transform_datatypes.h>
#include <pcl_ros/point_cloud.h>
#include <pcl_ros/transforms.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/registration/transformation_estimation_svd.h>
#include <visualization_msgs/Marker.h>

#include "ccny_rgbd/registration/motion_estimation.h"
#include "ccny_rgbd/Save.h"
#include "ccny_rgbd/Load.h"

namespace ccny_rgbd
{

class MotionEstimationICPProbModel: public MotionEstimation
{
  typedef pcl::KdTreeFLANN<PointFeature> KdTree;

  public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    MotionEstimationICPProbModel(ros::NodeHandle nh, ros::NodeHandle nh_private);
    virtual ~MotionEstimationICPProbModel();

    bool getMotionEstimationImpl(
      RGBDFrame& frame,
      const tf::Transform& prediction,
      tf::Transform& motion);
  
    int getModelSize() const { return model_size_; }

    bool saveSrvCallback(ccny_rgbd::Save::Request& request,
                         ccny_rgbd::Save::Response& response);
    bool loadSrvCallback(ccny_rgbd::Save::Request& request,
                         ccny_rgbd::Save::Response& response);

  private:

    // **** ros

    ros::Publisher model_publisher_;
    ros::Publisher covariances_publisher_;
    ros::ServiceServer save_service_;
    ros::ServiceServer load_service_;

    // **** params

    std::string fixed_frame_; 
    std::string base_frame_;

    int max_iterations_;
    int min_correspondences_;
    int n_nearest_neighbors_; // for searching for mah NN
    int max_model_size_;      // bound for how many features to store in the model

    double tf_epsilon_linear_;   
    double tf_epsilon_angular_;

    double max_assoc_dist_mah_;
    double max_corresp_dist_mah_;
    double max_corresp_dist_eucl_;
    
    bool publish_model_;      // for visualization
    bool publish_model_cov_;  // for visualization

    // derived
    double max_assoc_dist_mah_sq_;
    double max_corresp_dist_mah_sq_;
    double max_corresp_dist_eucl_sq_;

    // **** variables

    PointCloudFeature::Ptr model_ptr_;   
    int model_idx_;         // current intex in the ring buffer
    int model_size_;        // current model size
    Vector3fVector means_;
    Matrix3fVector covariances_;

    KdTree model_tree_;

    Matrix3f I_;
    
    tf::Transform f2b_; // Fixed frame to Base (moving) frame
    
    // ***** funtions
  
    bool alignICPEuclidean(
      const Vector3fVector& data_means,
      tf::Transform& correction);

    bool alignICPMahalanobis(
      const Vector3fVector& data_means_in,
      const Matrix3fVector& data_covariances_in,
      tf::Transform& correction);

    void getCorrespEuclidean(
      const PointCloudFeature& data_cloud,
      IntVector& data_indices,
      IntVector& model_indices);
    
    void getCorrespMahalanobis(
      const Vector3fVector& data_means,
      const Matrix3fVector& data_covariances,
      IntVector& data_indices,
      IntVector& model_indices);
 
    bool getNNEuclidean(
      const PointFeature& data_point,
      int& eucl_nn_idx, double& eucl_dist_sq);

    bool getNNMahalanobis(
      const Vector3f& data_mean, const Matrix3f& data_cov,
      int& mah_nn_idx, double& mah_dist_sq,
      IntVector& indices, FloatVector& dists_sq);

    void updateModelFromData(const Vector3fVector& data_means,
                             const Matrix3fVector& data_covariances);
  
    void initializeModelFromData(
      const Vector3fVector& data_means,
      const Matrix3fVector& data_covariances);

    void addToModel(
      const Vector3f& data_mean,
      const Matrix3f& data_cov);

    void publishCovariances();
    
    bool saveModel(const std::string& filename);
    bool loadModel(const std::string& filename);
};

} // namespace ccny_rgbd

#endif // CCNY_RGBD_MOTION_ESTIMATION_ICP_PROB_MODEL_H

