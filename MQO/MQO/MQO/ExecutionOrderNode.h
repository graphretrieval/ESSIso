#pragma once

#include<vector>

class ExecutionOrderNode {
public:
	int pcmId;
	std::vector<int> releaseParentsPcmId;

	ExecutionOrderNode() {
		pcmId = -1;
	}
};