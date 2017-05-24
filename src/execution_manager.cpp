#include "tangible/execution_manager.h"

namespace tangible {

ExecutionManager::ExecutionManager(ros::NodeHandle& n)
{
	ROS_INFO("execution node instantiated...");

}

ExecutionManager::~ExecutionManager() {}

void ExecutionManager::mode_callback(const tangible::Mode::ConstPtr& mode_msg) 
{
	mode = *(mode_msg);
	switch(mode)
	{
		case *(mode_msg).IDLE:
			ROS_INFO("Idle Mode");

			// stop all movements
			stop_execution();

			break;

		case *(mode_msg).EDIT:
			ROS_INFO("Edit Mode");

			// clear the program. A new program should be obtained after the edit
			stop_execution();
			program.clear();

			break;

		case *(mode_msg).EXECUTE:
			ROS_INFO("Execution Mode");

			// if the program is not defined, obtain the program
			// else resume program execution from the first incompelete instruction.
			// NOTE: instructions are the atomic units of normal program execution.
			if(program.empty())
				get_program();

			start_execution();

			break;

		default:
			ROS_INFO("Invalid Mode");
	}
}

void ExecutionManager::get_program() 
{

}

void ExecutionManager::get_scene()
{

}

void ExecutionManager::start_execution()
{

}

void ExecutionManager::stop_execution()
{

}

}