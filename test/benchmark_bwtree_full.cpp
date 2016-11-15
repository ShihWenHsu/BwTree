
/*
 * benchmark_bwtree_full.cpp - This file contains test suites for command
 *                             benchmark-bwtree-full
 */

#include "test_suite.h"

/*
 * BenchmarkBwTreeSeqInsert() - As name suggests
 */
void BenchmarkBwTreeSeqInsert(TreeType *t, 
                              int key_num, 
                              int thread_num) {
  const int num_thread = thread_num;

  // This is used to record time taken for each individual thread
  double thread_time[num_thread];
  for(int i = 0;i < num_thread;i++) {
    thread_time[i] = 0.0;
  }

  auto func = [key_num, 
               &thread_time, 
               num_thread](uint64_t thread_id, TreeType *t) {
    long int start_key = key_num / num_thread * (long)thread_id;
    long int end_key = start_key + key_num / num_thread;

    // Declare timer and start it immediately
    Timer timer{true};
    CacheMeter cache{true};

    for(int i = start_key;i < end_key;i++) {
      t->Insert(i, i);
    }

    cache.Stop();
    double duration = timer.Stop();

    std::cout << "[Thread " << thread_id << " Done] @ " \
              << (key_num / num_thread) / (1024.0 * 1024.0) / duration \
              << " million insert/sec" << "\n";
    
    // Return L3 total accesses and cache misses
    auto l3_util = cache.GetL3CacheUtilization();
    
    std::cout << "    L3 total = " << l3_util.first << "; miss = " \
              << l3_util.second << "; hit ratio = " \
              << static_cast<double>(l3_util.second) / \
                 static_cast<double>(l3_util.second - l3_util.first) \
              << std::endl;

    thread_time[thread_id] = duration;

    return;
  };

  LaunchParallelTestID(num_thread, func, t);

  double elapsed_seconds = 0.0;
  for(int i = 0;i < num_thread;i++) {
    elapsed_seconds += thread_time[i];
  }

  std::cout << num_thread << " Threads BwTree: overall "
            << (key_num / (1024.0 * 1024.0) * num_thread) / elapsed_seconds
            << " million insert/sec" << "\n";
            
  return;
}

/*
 * BenchmarkBwTreeSeqRead() - As name suggests
 */
void BenchmarkBwTreeSeqRead(TreeType *t, 
                            int key_num,
                            int thread_num) {
  const int num_thread = thread_num;
  int iter = 1;
  
  // This is used to record time taken for each individual thread
  double thread_time[num_thread];
  for(int i = 0;i < num_thread;i++) {
    thread_time[i] = 0.0;
  }
  
  auto func = [key_num, 
               iter, 
               &thread_time, 
               num_thread](uint64_t thread_id, TreeType *t) {
    std::vector<long> v{};

    v.reserve(1);

    Timer timer{true};

    for(int j = 0;j < iter;j++) {
      for(int i = 0;i < key_num;i++) {
        t->GetValue(i, v);

        v.clear();
      }
    }

    double duration = timer.Stop();

    std::cout << "[Thread " << thread_id << " Done] @ " \
              << (iter * key_num / (1024.0 * 1024.0)) / duration \
              << " million read/sec" << "\n";
              
    thread_time[thread_id] = duration;

    return;
  };

  LaunchParallelTestID(num_thread, func, t);
  
  double elapsed_seconds = 0.0;
  for(int i = 0;i < num_thread;i++) {
    elapsed_seconds += thread_time[i];
  }

  std::cout << num_thread << " Threads BwTree: overall "
            << (iter * key_num / (1024.0 * 1024.0) * num_thread * num_thread) / elapsed_seconds
            << " million read/sec" << "\n";

  return;
}

/*
 * BenchmarkBwTreeRandRead() - As name suggests
 */
void BenchmarkBwTreeRandRead(TreeType *t, 
                             int key_num,
                             int thread_num) {
  const int num_thread = thread_num;
  int iter = 1;
  
  // This is used to record time taken for each individual thread
  double thread_time[num_thread];
  for(int i = 0;i < num_thread;i++) {
    thread_time[i] = 0.0;
  }
  
  auto func2 = [key_num, 
                iter, 
                &thread_time,
                num_thread](uint64_t thread_id, TreeType *t) {
    std::vector<long> v{};

    v.reserve(1);
    
    // This is the random number generator we use
    SimpleInt64Random<0, 30 * 1024 * 1024> h{};

    Timer timer{true};

    for(int j = 0;j < iter;j++) {
      for(int i = 0;i < key_num;i++) {
        //int key = uniform_dist(e1);
        long int key = (long int)h((uint64_t)i, thread_id);

        t->GetValue(key, v);

        v.clear();
      }
    }

    double duration = timer.Stop();

    std::cout << "[Thread " << thread_id << " Done] @ " \
              << (iter * key_num / (1024.0 * 1024.0)) / duration \
              << " million read (random)/sec" << "\n";
              
    thread_time[thread_id] = duration;

    return;
  };

  LaunchParallelTestID(num_thread, func2, t);

  double elapsed_seconds = 0.0;
  for(int i = 0;i < num_thread;i++) {
    elapsed_seconds += thread_time[i];
  }

  std::cout << num_thread << " Threads BwTree: overall "
            << (iter * key_num / (1024.0 * 1024.0) * num_thread * num_thread) / elapsed_seconds
            << " million read (random)/sec" << "\n";

  return;
}


/*
 * BenchmarkBwTreeZipfRead() - As name suggests
 */
void BenchmarkBwTreeZipfRead(TreeType *t, 
                             int key_num,
                             int thread_num) {
  const int num_thread = thread_num;
  int iter = 1;
  
  // This is used to record time taken for each individual thread
  double thread_time[num_thread];
  for(int i = 0;i < num_thread;i++) {
    thread_time[i] = 0.0;
  }
  
  // Generate zipfian distribution into this list
  std::vector<long> zipfian_key_list{};
  zipfian_key_list.reserve(key_num);
  
  // Initialize it with time() as the random seed
  Zipfian zipf{(uint64_t)key_num, 0.99, (uint64_t)time(NULL)};
  
  // Populate the array with random numbers 
  for(int i = 0;i < key_num;i++) {
    zipfian_key_list.push_back(zipf.Get()); 
  }
  
  auto func2 = [key_num, 
                iter, 
                &thread_time,
                &zipfian_key_list,
                num_thread](uint64_t thread_id, TreeType *t) {
    // This is the start and end index we read into the zipfian array
    long int start_index = key_num / num_thread * (long)thread_id;
    long int end_index = start_index + key_num / num_thread;
    
    std::vector<long> v{};

    v.reserve(1);

    Timer timer{true};

    for(int j = 0;j < iter;j++) {
      for(long i = start_index;i < end_index;i++) {
        long int key = zipfian_key_list[i];

        t->GetValue(key, v);

        v.clear();
      }
    }

    double duration = timer.Stop();

    std::cout << "[Thread " << thread_id << " Done] @ " \
              << (iter * (end_index - start_index) / (1024.0 * 1024.0)) / duration \
              << " million read (zipfian)/sec" << "\n";
              
    thread_time[thread_id] = duration;

    return;
  };

  LaunchParallelTestID(num_thread, func2, t);

  double elapsed_seconds = 0.0;
  for(int i = 0;i < num_thread;i++) {
    elapsed_seconds += thread_time[i];
  }

  std::cout << num_thread << " Threads BwTree: overall "
            << (iter * key_num / (1024.0 * 1024.0)) / (elapsed_seconds / num_thread)
            << " million read (zipfian)/sec" << "\n";

  return;
}
