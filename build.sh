# 单生产者模型
g++ -pthread -std=c++11 oneProducers.cpp ThreadPool.cpp -o oneProducers
echo "running oneProducers ThreadPool"
./oneProducers
rm ./oneProducers
echo "finish check oneProducers ThreadPool"
echo "====================================================================="


# 多生产者模型
g++ -pthread -std=c++11 multiProducers.cpp ThreadPool.cpp -o multiProducers
echo "running multiProducers ThreadPool"
./multiProducers
rm ./multiProducers
echo "finish check multiProducers ThreadPool"
echo "====================================================================="


# benchmark
g++ -pthread -std=c++11 benchmark.cpp ThreadPool.cpp -o benchmark
echo "running benchmark"
./benchmark
rm ./benchmark
echo "finish benchmark"
echo "====================================================================="