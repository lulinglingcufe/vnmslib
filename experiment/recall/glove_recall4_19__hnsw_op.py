import h5py
import logging
logging.basicConfig(level=logging.NOTSET)

import numpy 
import sys 
import nmslib 
import time 
import math 
from sklearn.neighbors import NearestNeighbors
from sklearn.model_selection import train_test_split
print(sys.version)
print("NMSLIB version:", nmslib.__version__)


# 打开HDF5文件，以只读模式打开
with h5py.File('/home/ubuntu/Lulingling/testnmslib/glove-200-angular.hdf5', 'r') as file:
    # 获取文件中的一个数据集，假设数据集名称为'dataset_name'
    all_data_matrix  = file['train']#train test distances neighbors
    all_data_matrix_query = file['test']

    # 读取数据集里的数据
    #data = dataset[0:10]
    query_matrix = all_data_matrix_query[0:1000]
    data_matrix = all_data_matrix[0:200000]
    
    
M = 8
efC = 100


num_threads = 1
index_time_params = {'M': M, 'indexThreadQty': num_threads, 'efConstruction': efC, 'post' : 2
                    }
print('Index-time parameters', index_time_params)

# Number of neighbors 
K=10
# Space name should correspond to the space name 
# used for brute-force search
space_name='cosinesimil'  #cosinesimil l2
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
efS = 100 #20万数据  # 10万数据,40 

query_time_params = {'efSearch': efS}
print('Setting query-time parameters', query_time_params)
index.setQueryTimeParams(query_time_params)


# Querying
query_qty = query_matrix.shape[0]
start = time.time() 
nbrs = index.knnQueryBatch(query_matrix, k = K, num_threads = num_threads)
end = time.time() 
print('kNN time total=%f (sec), per query=%f (sec), per query adjusted for thread number=%f (sec)' % 
      (end-start, float(end-start)/query_qty, num_threads*float(end-start)/query_qty)) 

# # Computing gold-standard data 正确的knn数据 
print('Computing gold-standard data')

start = time.time()
sindx = NearestNeighbors(n_neighbors=K, metric='cosine', algorithm='brute').fit(data_matrix)
end = time.time()

print('Brute-force preparation time %f' % (end - start))

start = time.time() 
gs = sindx.kneighbors(query_matrix)
end = time.time()

print('brute-force kNN time total=%f (sec), per query=%f (sec)' % 
      (end-start, float(end-start)/query_qty) )

# # Finally computing recall
recall=0.0
for i in range(0, query_qty):
  correct_set = set(gs[1][i])
  ret_set = set(nbrs[i][0])
  recall = recall + float(len(correct_set.intersection(ret_set))) / len(correct_set)
recall = recall / query_qty
print('kNN recall %f' % recall)


# # Save a meta index, but no data!
#index.saveIndex('dense_index_optim.bin', save_data=False)
index.saveIndex('dense_index_glove_10.bin', save_data=False)