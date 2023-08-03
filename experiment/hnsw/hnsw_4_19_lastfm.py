import h5py


import logging
logging.basicConfig(level=logging.NOTSET)

import numpy 
import sys 
import nmslib 
import time 
import math 
print(sys.version)
print("NMSLIB version:", nmslib.__version__)

with h5py.File('/home/ubuntu/lulingling/testnmslib/lastfm-64-dot.hdf5', 'r') as file:
    all_data_matrix  = file['train']#train test distances neighbors
    all_data_matrix_query = file['test']


    query_matrix = all_data_matrix_query[0:100]
    data_matrix = all_data_matrix[0:200000]  #200000   5000


M = 10
#M = 8
efC =100

num_threads = 1
index_time_params = {'M': M, 'indexThreadQty': num_threads, 'efConstruction': efC, 'post' : 2}
#没有用skip_optimized_index参数，应该就是默认使用优化方法。

print('Index-time parameters', index_time_params)

# Number of neighbors 
K=10
# Space name should correspond to the space name 
# used for brute-force search
space_name='cosinesimil'# l2 cosinesimil l2
# Intitialize the library, specify the space, the type of the vector and add data points 
index = nmslib.init(method='hnsw', space=space_name, data_type=nmslib.DataType.DENSE_VECTOR) 
index.addDataPointBatch(data_matrix) 



# Create an index
start = time.time()
index.createIndex(index_time_params) 
end = time.time() 
print('Index-time parameters', index_time_params)
print('Indexing time = %f' % (end-start))

# Setting query-time parameters
efS = 50


query_time_params = {'efSearch': efS}
print('Setting query-time parameters', query_time_params)
index.setQueryTimeParams(query_time_params)


# Querying 查询计时
query_qty = query_matrix.shape[0]
start = time.time() 
nbrs = index.knnQueryBatch(query_matrix, k = K, num_threads = num_threads)
end = time.time() 
print('kNN time total=%f (sec), per query=%f (sec), per query adjusted for thread number=%f (sec)' % 
      (end-start, float(end-start)/query_qty, num_threads*float(end-start)/query_qty)) 

# Save a meta index, but no data!
index.saveIndex('dense_index_lastfm_20_optim.bin', save_data=False)