#ifndef TANGIBLE_TAG_EXTRACTOR
#define TANGIBLE_TAG_EXTRACTOR

#include <vector>

#include "ros/ros.h"
#include "ar_track_alvar_msgs/AlvarMarkers.h"

#include "tangible_msgs/GetBlocks.h"
#include "tangible_msgs/Block.h"
#include "tangible_msgs/Mode.h"
#include "Eigen/Geometry"
#include "geometry_msgs/PoseStamped.h"
#include <tf/transform_broadcaster.h>

namespace tangible {

class TagExtractor {
private:
	ros::NodeHandle node_handle;

	std::vector<tangible_msgs::Block> blocks;
	void initBlock(geometry_msgs::PoseStamped& p, int _id, tangible_msgs::Block* block);
	void setAxes(geometry_msgs::PoseStamped& p, tangible_msgs::Block* block);
	ros::Publisher mode_pub;
	int edit_id;
	int idle_id;
	int run_id;
	bool edit_state;
	bool idle_state;
	bool run_state;

public:
	TagExtractor(ros::NodeHandle& n, int i_id, int r_id, int e_id, std::string mode_change_topic);
	~TagExtractor();
	void tagCallback(const ar_track_alvar_msgs::AlvarMarkers::ConstPtr msg);
	bool parseCallback(tangible_msgs::GetBlocks::Request& req,
                 tangible_msgs::GetBlocks::Response& res);
};

}

#endif

//TO-DO publisher to publish tag information. Would be helpful for debugging