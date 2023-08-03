/**
 * Non-metric Space Library
 *
 * Main developers: Bilegsaikhan Naidan, Leonid Boytsov, Yury Malkov, Ben Frederickson, David Novak
 *
 * For the complete list of contributors and further details see:
 * https://github.com/nmslib/nmslib
 *
 * Copyright (c) 2013-2018
 *
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */
/*
*
* A Hierarchical Navigable Small World (HNSW) approach.
*
* The main publication is (available on arxiv: http://arxiv.org/abs/1603.09320):
* "Efficient and robust approximate nearest neighbor search using Hierarchical Navigable Small World graphs" by Yu. A. Malkov, D. A. Yashunin
* This code was contributed by Yu. A. Malkov. It also was used in tests from the paper.
*
*
*/
#include "unistd.h"

#include "ztimer.h"
#include "Keccak256.h"
#include "method/hnsw.h"
#include "method/hnsw_distfunc_opt_impl_inline.h"
#include "knnquery.h"
#include "ported_boost_progress.h"
#include "rangequery.h"

#include "portable_prefetch.h"
#include "space.h"

#include "sort_arr_bi.h"
#define MERGE_BUFFER_ALGO_SWITCH_THRESHOLD 100

#include <algorithm> // std::min
#include <limits>
#include <vector>

#include <iomanip>
using std::uint8_t;
using std::setfill;
using std::setw;
using std::set;


//#define DIST_CALC
namespace similarity {

double totalSearchTime_hnsw = 0;
double totalSearchTime_hnsw_save = 0;
double totalSearchTime_hnsw_load = 0;




    template <typename dist_t>
    void
    Hnsw<dist_t>::SearchOld(KNNQuery<dist_t> *query, bool normalize)
    {
        LOG(LIB_INFO) << "This is SearchOld: ";
                int nodeCount = 0;

        float *pVectq = (float *)((char *)query->QueryObject()->data());
        TMP_RES_ARRAY(TmpRes);
        size_t qty = query->QueryObject()->datalength() >> 2;

        if (normalize) {
            NormalizeVect(pVectq, qty);
        }

        VisitedList *vl = visitedlistpool->getFreeVisitedList();
        vl_type *massVisited = vl->mass;
        vl_type currentV = vl->curV;

        int maxlevel1 = maxlevel_;
        int curNodeNum = enterpointId_;
        dist_t curdist = (fstdistfunc_(
            pVectq, (float *)(data_level0_memory_ + enterpointId_ * memoryPerObject_ + offsetData_ + 16), qty, TmpRes));

        for (int i = maxlevel1; i > 0; i--) {
            bool changed = true;
            while (changed) {
                changed = false;
                int *data = (int *)(linkLists_[curNodeNum] + (maxM_ + 1) * (i - 1) * sizeof(int));
                int size = *data;
                for (int j = 1; j <= size; j++) {
                    PREFETCH(data_level0_memory_ + (*(data + j)) * memoryPerObject_ + offsetData_, _MM_HINT_T0);
                }//预先把朋友的data值取出来


#ifdef DIST_CALC
                query->distance_computations_ += size;
#endif

                for (int j = 1; j <= size; j++) {
                    int tnum = *(data + j);//获得朋友的id

                    dist_t d = (fstdistfunc_(
                        pVectq, (float *)(data_level0_memory_ + tnum * memoryPerObject_ + offsetData_ + 16), qty, TmpRes)); //获取朋友的data值，来和query计算距离


                    if (d < curdist) {
                        curdist = d;
                        curNodeNum = tnum;
                        changed = true;
                    }
                }





            }
        }

        priority_queue<EvaluatedMSWNodeInt<dist_t>> candidateQueuei; // the set of elements which we can use to evaluate

        priority_queue<EvaluatedMSWNodeInt<dist_t>> closestDistQueuei; // The set of closest found elements
        // EvaluatedMSWNodeInt<dist_t> evi(curdist, curNodeNum);
        candidateQueuei.emplace(-curdist, curNodeNum);

        closestDistQueuei.emplace(curdist, curNodeNum);

        // query->CheckAndAddToResult(curdist, new Object(data_level0_memory_ + (curNodeNum)*memoryPerObject_ + offsetData_));
        query->CheckAndAddToResult(curdist, data_rearranged_[curNodeNum]);

        //打印hash


		std::uint8_t actualHash[Keccak256::HASH_LEN];
        //2022-11-22-Keccak256方法.md: 把buffer 指针 转换为uint8_t 指针，(uint8_t *)
		Keccak256::getHash(  (uint8_t *)data_rearranged_[curNodeNum]->buffer(), data_rearranged_[curNodeNum]->bufferlength(), actualHash);

        LOG(LIB_INFO) << "actualHash: " << actualHash;



        massVisited[curNodeNum] = currentV;

        while (!candidateQueuei.empty()) {
            EvaluatedMSWNodeInt<dist_t> currEv = candidateQueuei.top(); // This one was already compared to the query

            dist_t lowerBound = closestDistQueuei.top().getDistance();
            if ((-currEv.getDistance()) > lowerBound) {
                break;
            }

            candidateQueuei.pop();
            curNodeNum = currEv.element;
            int *data = (int *)(data_level0_memory_ + curNodeNum * memoryPerObject_ + offsetLevel0_);
            int size = *data;
            PREFETCH((char *)(massVisited + *(data + 1)), _MM_HINT_T0);
            PREFETCH((char *)(massVisited + *(data + 1) + 64), _MM_HINT_T0);
            PREFETCH(data_level0_memory_ + (*(data + 1)) * memoryPerObject_ + offsetData_, _MM_HINT_T0);
            PREFETCH((char *)(data + 2), _MM_HINT_T0);

            for (int j = 1; j <= size; j++) {
                int tnum = *(data + j);
                PREFETCH((char *)(massVisited + *(data + j + 1)), _MM_HINT_T0);
                PREFETCH(data_level0_memory_ + (*(data + j + 1)) * memoryPerObject_ + offsetData_, _MM_HINT_T0);
                if (!(massVisited[tnum] == currentV)) {
#ifdef DIST_CALC
                    query->distance_computations_++;
#endif
                    nodeCount++;  //这里可以看到访问的node到底有多少个
                    massVisited[tnum] = currentV;
                    char *currObj1 = (data_level0_memory_ + tnum * memoryPerObject_ + offsetData_);
                    dist_t d = (fstdistfunc_(pVectq, (float *)(currObj1 + 16), qty, TmpRes));
                    if (closestDistQueuei.top().getDistance() > d || closestDistQueuei.size() < ef_) {
                        candidateQueuei.emplace(-d, tnum);
                        PREFETCH(data_level0_memory_ + candidateQueuei.top().element * memoryPerObject_ + offsetLevel0_,
                                     _MM_HINT_T0);
                        // query->CheckAndAddToResult(d, new Object(currObj1));
                        query->CheckAndAddToResult(d, data_rearranged_[tnum]);
                        closestDistQueuei.emplace(d, tnum);

                        if (closestDistQueuei.size() > ef_) {
                            closestDistQueuei.pop();
                        }
                    }
                }
            }
        }
        visitedlistpool->releaseVisitedList(vl);
        LOG(LIB_INFO) << "nodeCount: " << nodeCount;
    }

int toal_query_number =0;
int total_tum_set_size = 0;

    template <typename dist_t>
    void
    Hnsw<dist_t>::SearchV1Merge(KNNQuery<dist_t> *query, bool normalize)
    {
        //计时器
        WallClockTimer wtm;
        wtm.reset();

        toal_query_number++;
        set<int> tum_set;
        int nodeCount = 0;

        float *pVectq = (float *)((char *)query->QueryObject()->data());
        TMP_RES_ARRAY(TmpRes);
        size_t qty = query->QueryObject()->datalength() >> 2;

        if (normalize) {
            NormalizeVect(pVectq, qty);
        }

        VisitedList *vl = visitedlistpool->getFreeVisitedList();
        vl_type *massVisited = vl->mass;
        vl_type currentV = vl->curV;  //这个是啥来着?我忘了。

        int maxlevel1 = maxlevel_;
        int curNodeNum = enterpointId_;


        tum_set.insert(enterpointId_);
        nodeCount++;

        dist_t curdist = (fstdistfunc_(
            pVectq, (float *)(data_level0_memory_ + enterpointId_ * memoryPerObject_ + offsetData_ + 16), qty, TmpRes));

        for (int i = maxlevel1; i > 0; i--) {
            bool changed = true;
            while (changed) {
                changed = false;
                int *data = (int *)(linkLists_[curNodeNum] + (maxM_ + 1) * (i - 1) * sizeof(int));
                //也就是说data是一个int数组？每一个值都是enterpointId_节点在（某）层的朋友？
                int size = *data;  //enterpointId_节点在（某）层的朋友的数量。
                for (int j = 1; j <= size; j++) {
                    PREFETCH(data_level0_memory_ + (*(data + j)) * memoryPerObject_ + offsetData_, _MM_HINT_T0);
                }//预先把朋友的data值取出来。
#ifdef DIST_CALC
                query->distance_computations_ += size;
#endif

                for (int j = 1; j <= size; j++) {
                    int tnum = *(data + j);
                    //tnum就是enterpointId_节点的朋友的id。
                    tum_set.insert(tnum);
                    nodeCount++;

                    dist_t d = (fstdistfunc_(
                        pVectq, (float *)(data_level0_memory_ + tnum * memoryPerObject_ + offsetData_ + 16), qty, TmpRes)); //计算朋友们和query节点之间的距离

                    if (d < curdist) {
                        curdist = d;
                        curNodeNum = tnum;
                        changed = true;
                    }
                }
            }
        }

        SortArrBI<dist_t, int> sortedArr(max<size_t>(ef_, query->GetK()));
        sortedArr.push_unsorted_grow(curdist, curNodeNum);

        int_fast32_t currElem = 0;

        typedef typename SortArrBI<dist_t, int>::Item QueueItem;
        vector<QueueItem> &queueData = sortedArr.get_data(); //队列
        vector<QueueItem> itemBuff(1 + max(maxM_, maxM0_));

        massVisited[curNodeNum] = currentV;  //设置编号为curNodeNum的节点已经访问过了。

        //当candidates里不存在任何元素，循环结束???
        while (currElem < min(sortedArr.size(), ef_)) {
            auto &e = queueData[currElem];
            CHECK(!e.used);
            e.used = true;
            curNodeNum = e.data;
            ++currElem;

            size_t itemQty = 0;
            dist_t topKey = sortedArr.top_key();

            int *data = (int *)(data_level0_memory_ + curNodeNum * memoryPerObject_ + offsetLevel0_);
            int size = *data;
            PREFETCH((char *)(massVisited + *(data + 1)), _MM_HINT_T0);
            PREFETCH((char *)(massVisited + *(data + 1) + 64), _MM_HINT_T0);
            PREFETCH(data_level0_memory_ + (*(data + 1)) * memoryPerObject_ + offsetData_, _MM_HINT_T0);
            PREFETCH((char *)(data + 2), _MM_HINT_T0);

            for (int j = 1; j <= size; j++) {
                int tnum = *(data + j);
                //把当前候选者这个level的所有friends取出来 [level][]；tnum是一个朋友的id

                PREFETCH((char *)(massVisited + *(data + j + 1)), _MM_HINT_T0);
                PREFETCH(data_level0_memory_ + (*(data + j + 1)) * memoryPerObject_ + offsetData_, _MM_HINT_T0);
                if (!(massVisited[tnum] == currentV)) {  //如果id为tnum的朋友没有被访问过：
#ifdef DIST_CALC
                    query->distance_computations_++;
#endif
                    massVisited[tnum] = currentV;  //设置编号为tnum的节点已经访问过了。
                    tum_set.insert(tnum);
                    nodeCount++;

                    char *currObj1 = (data_level0_memory_ + tnum * memoryPerObject_ + offsetData_);
                    dist_t d = (fstdistfunc_(pVectq, (float *)(currObj1 + 16), qty, TmpRes));
                    //计算编号为tnum的节点与query节点之间的距离。

                    if (d < topKey || sortedArr.size() < ef_) {
                        CHECK_MSG(itemBuff.size() > itemQty,
                                  "Perhaps a bug: buffer size is not enough " + 
                                  ConvertToString(itemQty) + " >= " + ConvertToString(itemBuff.size()));
                        itemBuff[itemQty++] = QueueItem(d, tnum);
                    }
                }
            }

            if (itemQty) {
                PREFETCH(const_cast<const char *>(reinterpret_cast<char *>(&itemBuff[0])), _MM_HINT_T0);
                std::sort(itemBuff.begin(), itemBuff.begin() + itemQty);

                size_t insIndex = 0;
                if (itemQty > MERGE_BUFFER_ALGO_SWITCH_THRESHOLD) {
                    insIndex = sortedArr.merge_with_sorted_items(&itemBuff[0], itemQty);

                    if (insIndex < currElem) {
                        currElem = insIndex;
                    }
                } else {
                    for (size_t ii = 0; ii < itemQty; ++ii) {
                        size_t insIndex = sortedArr.push_or_replace_non_empty_exp(itemBuff[ii].key, itemBuff[ii].data);
                        if (insIndex < currElem) {
                            currElem = insIndex;
                        }
                    }
                }
                // because itemQty > 1, there would be at least item in sortedArr
                PREFETCH(data_level0_memory_ + sortedArr.top_item().data * memoryPerObject_ + offsetLevel0_, _MM_HINT_T0);
            }
            // To ensure that we either reach the end of the unexplored queue or currElem points to the first unused element
            while (currElem < sortedArr.size() && queueData[currElem].used == true)
                ++currElem;
        }

        for (int_fast32_t i = 0; i < query->GetK() && i < sortedArr.size(); ++i) {
            int tnum = queueData[i].data;
            //LOG(LIB_INFO) << "int tnum = queueData[i].data  :  "<< tnum;
            // char *currObj = (data_level0_memory_ + tnum*memoryPerObject_ + offsetData_);
            // query->CheckAndAddToResult(queueData[i].key, new Object(currObj));
            query->CheckAndAddToResult(queueData[i].key, data_rearranged_[tnum]);
        }
        visitedlistpool->releaseVisitedList(vl);
        //LOG(LIB_INFO) << "nodeCount: " << nodeCount;
	    nodeCount = 0;

        ////LOG(LIB_INFO) << "toal_query_number:         " << toal_query_number;
        total_tum_set_size+=tum_set.size();
        // //遍历set中的元素。打印出来，可以做proof的实验。//开始!!!!!

        // LOG(LIB_INFO) << "tum_set.size():         " << tum_set.size();
        // printf("\n");

        // printf("{");
        // set<int>::iterator it;
        // for (it = tum_set.begin(); it != tum_set.end(); it++){
        //     printf("%d",*it);
        //     printf(",");
        // }
        // printf("}\n"); 

        // //遍历set中的元素。打印出来，可以做proof的实验。
        // //LOG(LIB_INFO) << "Iterator Set:         " << *it; //结束!!!!!!!!

        if(toal_query_number == 100){
         LOG(LIB_INFO) << "average total_tum_set_size:         " << total_tum_set_size/100.0<<"\n\n\n";
        }

        
        wtm.split();
        const double SearchTime  = double(wtm.elapsed())/1e3;
        totalSearchTime_hnsw += SearchTime; 
        if(toal_query_number == 100){
            LOG(LIB_INFO) << ">>>> average Search time:         " << totalSearchTime_hnsw/(toal_query_number*1.0)<<"\n\n\n";
        }


        //-------------------在这里加上存储朋友的代码。代码逻辑开始。
        WallClockTimer wtmsave;
        wtmsave.reset();

  std::string location = "/home/ubuntu/lulingling/nmslib/similarity_search/hnswvo/hnsw_node_and_friend_"+std::to_string(toal_query_number);
  std::ofstream output(location, std::ios::binary);
  CHECK_MSG(output, "Cannot open file '" + location + "' for writing");
  output.exceptions(ios::badbit | ios::failbit);

        set<int>::iterator it;
        for (it = tum_set.begin(); it != tum_set.end(); it++){
            const HnswNode& node = *ElList_[*it];
            unsigned currlevel = node.level;
            CHECK(currlevel + 1 == node.allFriends_.size());
            /*
             * This check strangely fails ...
            CHECK_MSG(maxlevel_ >= currlevel, ""
                    "maxlevel_ (" + ConvertToString(maxlevel_) + ") < node.allFriends_.size() (" + ConvertToString(currlevel));
                    */
            writeBinaryPOD(output, currlevel);
            for (unsigned level = 0; level <= currlevel; ++level) {
                const auto& friends = node.allFriends_[level];
                unsigned friendQty = friends.size();
                writeBinaryPOD(output, friendQty); //这个好像是一层里面朋友的数量？
                for (unsigned k = 0; k < friendQty; ++k) {
                    IdType friendId = friends[k]->id_;  //朋友的id。
                    writeBinaryPOD(output, friendId);
                }
            }
        }
        output.close(); 


        wtmsave.split();
        const double SearchTime_save  = double(wtmsave.elapsed())/1e3;
        totalSearchTime_hnsw_save += SearchTime_save; 
        if(toal_query_number == 100){
            LOG(LIB_INFO) << ">>>> SP generate VO: average  totalSearchTime_hnsw_save time:         " << totalSearchTime_hnsw_save/(toal_query_number*1.0);
        }        
      //----------------存储朋友的逻辑结束。


       //----------------读取朋友的逻辑开始。
        WallClockTimer wtmload;
        wtmload.reset();

        std::ifstream input(location, 
                            std::ios::binary); /* text files can be opened in binary mode as well */
        CHECK_MSG(input, "Cannot open file '" + location + "' for reading");

        input.exceptions(ios::badbit | ios::failbit);

         set<int>::iterator itload;
        for (itload = tum_set.begin(); itload != tum_set.end(); itload++){
            HnswNode& node = *ElList_[*itload];
            unsigned currlevel;
            readBinaryPOD(input, currlevel);
            node.level = currlevel;
            node.allFriends_.resize(currlevel + 1);
            for (unsigned level = 0; level <= currlevel; ++level) {
                auto& friends = node.allFriends_[level];
                unsigned friendQty;
                readBinaryPOD(input, friendQty);
                friends.resize(friendQty);
                for (unsigned k = 0; k < friendQty; ++k) {
                    IdType friendId;
                    readBinaryPOD(input, friendId);
                    // CHECK_MSG(friendId >= 0 && friendId < totalElementsStored_,
                    //          "Invalid friendId = " + ConvertToString(friendId) + " for node id: " + ConvertToString(*itload));
                    friends[k] = ElList_[friendId];
                }

            }
        }
            input.close();


        wtmload.split();
        const double SearchTimeload  = double(wtmload.elapsed())/1e3;
        totalSearchTime_hnsw_load += SearchTimeload; 
        if(toal_query_number == 100){
        LOG(LIB_INFO) << ">>>> User verifies VO: average totalSearchTime_hnsw_load :         " << totalSearchTime_hnsw_load/(toal_query_number*1.0)<<"\n\n\n";;
        }
       //----------------读取朋友的逻辑结束。 


     


    }

    template class Hnsw<float>;
    template class Hnsw<int>;
}
