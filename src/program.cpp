#include "tangible/program.h"

#include <algorithm>
#include <math.h>

#include "Eigen/Geometry"
#include "tangible/utils.h"

//testing
#include <iostream>

namespace tangible {

Program::Program(std::vector<Tag>& tgs) { 
	tags = tgs;
	groupTags();
}
Program::~Program() {}

bool Program::refreshTags(std::vector<Tag>& tgs) {
	tags = tags;
	return groupTags();
}

bool Program::groupTags() {
//grouping:
//  - sort tags ascendingly based on id
//  - find the range for selection, action, and number tags
//  - for each pair of numbers and actions/secodndary selections tags
//      - find the vector connecting the centers of the two tags and normalize it
//      - consider the two as grouped if
//          - the center to center distance is ~TAG_EDGE
//          - the dot product of the normalized center to center and the x-axis is ~1
//            alternativly, the dot product of x-axes of the two tags is ~1
//      ! the grouping is 1-1 => visited (i.e. already grouped) can be removed from the pool
//  - for each pair of actions and selections tags
//      similar to number-action/secondary selection but should find the minimum 
//      center-2-center distance and should check alignment with y-axis
//  ! repeated numbers ---> selection and secondary selection that should be grouped but this
//    can wait to be done during instruction generation
	
	int tagNum = tags.size();
	if(tagNum == 0) {
		return false;
		//TO-DO provide error message: zero tags to group
	}

	std::sort(tags.begin(), tags.end());

	int selectionInd = tagNum; // index where selection tags start
	int selection2ndInd = tagNum; // index where secondary selection tags start
	int actionInd = tagNum; // index where action tags start
	int numberInd = tagNum; // index where number tags start
	int otherInd  = tagNum; //YSS: to handle additional tags (e.g. loop)
	int grouped[tagNum];
	for(int i = 0; i < tagNum; i++) {
		if(inRange(Tag::SELECTION_ID_MIN, Tag::SELECTION_ID_MAX, tags.at(i).getID()) 
		    && i < selectionInd) selectionInd = i;
		else if(tags.at(i).getID() == Tag::SELECTION_2ND_ID
			&& i < selection2ndInd) selection2ndInd = i;
		else if(inRange(Tag::ACTION_ID_MIN, Tag::ACTION_ID_MAX, tags.at(i).getID())
			&& i < actionInd) actionInd = i;
		else if(inRange(Tag::NUMBER_ID_MIN, Tag::NUMBER_ID_MAX, tags.at(i).getID())
			&& i < numberInd) numberInd = i;
		else if(tags.at(i).getID() > Tag::NUMBER_ID_MAX
			&& i < otherInd) otherInd = i;
		grouped[i] = -1;
	}

	//TO-DO return false for the following error cases
	//   - only primary selection tags
	//   - only primary and secondary tas
	//   - only selection and action but no number tags
	//   - number of number tags ~= number of actions + number of region selection tags
	//   - number of action tags is odd
	//   - number of pick actions ~= number of place actions

	//std::cout << "selection tags @ " << selectionInd << ", " 
	//          << "2ndary selection tags @ " << selection2ndInd << ", " 
	//          << "ation tags @ " << actionInd << ", " 
	//          << "number tags @ " << numberInd << "\n";

	for(int i = 0; i < tagNum; i++) {
		if(i == selectionInd || i == selection2ndInd || i == actionInd || i == numberInd)
			std::cout << "| ";
		else
			std::cout << ", ";
		std::cout << tags.at(i).getID();
	}
	std::cout << " |\n";

	for(int i = numberInd; i < otherInd; i++) {
		Tag number = tags.at(i);
		//std::cout << "n" << number.getID() << "(" 
		//                 << number.getCenter().x << ", " 
		//                 << number.getCenter().y << ", " 
		//                 << number.getCenter().z << ") at " << i << " to\n";
		for(int j = selection2ndInd; j < numberInd; j++) {
			if(grouped[j] > -1) // action or secondary selection tag is already grouped
				continue;
			Tag actionOr2ndarySelection = tags.at(j);
			//std::cout << "\ta" << actionOr2ndarySelection.getID() << "("
			//                    << actionOr2ndarySelection.getCenter().x << ", " 
		    //                    << actionOr2ndarySelection.getCenter().y << ", " 
		    //                    << actionOr2ndarySelection.getCenter().z << ") at " << j << "\n";
			double distance = number.dist(actionOr2ndarySelection);
			//std::cout << "\tdistance: " << distance << "\n";
			if(!inRange(Tag::EDGE_SIZE - DIST_ERR_MARGIN,
				        Tag::EDGE_SIZE + DIST_ERR_MARGIN,
				        distance)) // action or secondary selection tag is too close/far
				continue;
			// normalized center-to-center vector
			Eigen::Vector3d n2a = number.vect(actionOr2ndarySelection) / distance;
			Eigen::Vector3d ux = number.getXvect();
			//std::cout << "\tx axis: " << ux.transpose() << "\n";
			double innerProduct = ux.dot(n2a);
			//std::cout << "\tinnerProduct: " << innerProduct << "\n";
			if(!inRange(1 - ROTATE_ERR_MARGIN,
			            1 + ROTATE_ERR_MARGIN,
			            innerProduct)) // action or secondary selection tag is not along x-axis
				continue;
			// number tag is grouped with action or secondary selection tag
			grouped[i] = j; 
			grouped[j] = i;
			//std::cout << "\tnumber at " << i << " grouped with action at " << j << "\n";
			break;
		}
	}

	//TO-DO return false for the following error cases
	//   - there is a number tag not yet grouped
	//   - there is a secondary selection tag not yet grouped
	//   - there is an action tag not yet grouped
	//   - id of more that two successive number tags is equal
	//   - of two successive tags with the same id, both are grouped with an action or 
	//     secondary selection tool
	
	for(int i = actionInd; i < numberInd; i++) {
		Tag action = tags.at(i);
		double minDist = MAX_WORKSPACE_DIST; int tempGrouped = -1;
		for(int j = selectionInd; j < selection2ndInd; j++) {
			Tag selection = tags.at(j);
			double distance = action.dist(selection);
			// normalized the center-to-center vector
			Eigen::Vector3d a2s = action.vect(selection) / distance;
			Eigen::Vector3d uy = action.getYvect();
			double innerProduct = uy.dot(a2s);
			if(!inRange(1 - ROTATE_ERR_MARGIN,
				        1 + ROTATE_ERR_MARGIN,
				        innerProduct)) // selection tag is not along the y-axis
				continue;
			if(distance < minDist) {
				minDist = distance;
				tempGrouped = j;
			}
		}
		double quantizedDist = round(minDist/Tag::EDGE_SIZE);
		if(!inRange(quantizedDist*Tag::EDGE_SIZE - DIST_ERR_MARGIN,
			        quantizedDist*Tag::EDGE_SIZE + DIST_ERR_MARGIN,
			        minDist)) // selection tag is not located in multiples of EDGE_SIZE
			continue;
		grouped[i] = tempGrouped;
		if(grouped[tempGrouped] == -1)
			grouped[tempGrouped] = i;
	}

	for(int i = numberInd; i < otherInd-1; i++) {
		Tag num1 = tags.at(i);
		Tag num2 = tags.at(i+1);
		if(num1.getID() == num2.getID()) {
			// of two successive tags with the same id, one is grouped with an action and
	        // another with a secondary selection tool
			if(tags.at(grouped[i]).getID() == Tag::SELECTION_2ND_ID &&
			   tags.at(grouped[i+1]).getID() == Tag::SELECTION_2ND_ID)
				return false;

			if(tags.at(grouped[i]).getID() != Tag::SELECTION_2ND_ID &&
			   tags.at(grouped[i+1]).getID() != Tag::SELECTION_2ND_ID)
				return false;			

			//YSS don't need to check the above condition if error handling is in place

			if(tags.at(grouped[i]).getID() == Tag::SELECTION_2ND_ID)
				grouped[grouped[i]] = grouped[grouped[i+1]];
			else
				grouped[grouped[i+1]] = grouped[grouped[i]];
		}
	}

	return true;
}

}