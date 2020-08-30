#include"GlobalConstant.h"



int GlobalConstant::G_GOURP_QUERY_CLIQUE_MINI_SIZE = 3;

int GlobalConstant::G_PCM_MINIMUM_CHILDREN_NUMBER = 3;

double GlobalConstant::G_COMMON_TLS_RATIO = 0.5;

int GlobalConstant::G_MINIMUM_NUMBER_MCS_VERTEX = 3;
int GlobalConstant::G_enoughRecursiveCalls = -1;



GlobalConstant::RUN_OPTION GlobalConstant::G_RUNNING_OPTION_INDEX = RUN_OPTION::RUN_OP_TURBOISO;

/*
 * G_MAXIMUM_WIDTH_COMBO can maximum be 3, if need more. Need to refine the cache intermediate results part
 */
int GlobalConstant::G_MAXIMUM_WIDTH_COMBO = 3;

int GlobalConstant::embeddingDimension = 128;
int GlobalConstant::numberOfCandidates = 50;
float GlobalConstant::distThreshold = 0.1;
float GlobalConstant::timeOut = 0.1;
float GlobalConstant::mcsTimeOut = 100;
int GlobalConstant::G_SUFFICIENT_NUMBER_EMBEDDINGS = 100;
bool GlobalConstant::useEmbedding = false;

int GlobalConstant::batchSize = 0;