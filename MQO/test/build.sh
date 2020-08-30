rm *cpp *.h* *.o* 
cp ../MQO/MQO/*.cpp ../MQO/MQO/*.h* ../MQO/Graph/*.cpp ../MQO/Graph/*.h* ../MQO/CommonUtility/*.cpp ../MQO/CommonUtility/*.h* .
g++ -std=c++14 *.cpp -lfaiss
