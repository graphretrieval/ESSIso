//
// This file encapsutaltes all the global parameters
//
#pragma once

class GlobalConstant {

public:
	/*
	* The cliques whose size less then the G_GOURP_QUERY_CLIQUE_MINI_SIZE won't be reported
	* The number of vertices in the clique is the number of query graphs in this group
	*/
	static int G_GOURP_QUERY_CLIQUE_MINI_SIZE;

	/*
	 * when we format the pcm, we also restrict that a query at least having more than 3 children. otherwise, there is no meaning
	 * to cache the results. the value of G_PCM_MINIMUM_CHILDREN_NUMBER normally are same as G_GOURP_QUERY_CLIQUE_MINI_SIZE.
	 */
	static int G_PCM_MINIMUM_CHILDREN_NUMBER;

	/*
	* The ratio is the threshold when to compute MCS between to graphs
	*/
	static double G_COMMON_TLS_RATIO;



	static int G_MINIMUM_NUMBER_MCS_VERTEX;
	static int G_enoughRecursiveCalls;

	enum RUN_OPTION {
		RUN_OP_TURBOISO,
		RUN_OP_TURBO_ISO_BOOSTED,
		RUN_OP_EMBOOST
	};

	static RUN_OPTION G_RUNNING_OPTION_INDEX;

	static int G_SUFFICIENT_NUMBER_EMBEDDINGS;
	static int G_MAXIMUM_WIDTH_COMBO;

	static int embeddingDimension;
	static int numberOfCandidates;
	static float distThreshold;
	static float timeOut;
	static float mcsTimeOut;
	static bool useEmbedding;

	static int batchSize;

};

