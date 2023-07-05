# Overview

ANNProof project is a verifiable outsourced approximate nearest neighbor search (K-ANNS) framework. We offer a generation and validation tool for outsourced ANN queries. We build the ANNProof framework on blockchain to ensure dataset provider (DP) delivers reliable datasets to service providers (SP). SPs are responsible for answering queries for users. Users can verify the correctness of query results with $VO_{SP}$ from SP and $VO_{chain}$ from the blockchain. 

We build a permissioned blockchain network utilizing **Hyperledger Fabric v2.2**.We employ an endorsement policy adhering to a ``three out of four" requirement, signifying that consistent endorsement must be provided by at least three out of the four SPs.
We developed a **nmslib library variant**, **v-nmslib**, which SP employs to construct indexes and ADS on individual nodes. The query user operates on a single node. 
We implement the query processing and the result verification programs in C++. 
We also use the **Keccak-256** cryptographic hash function for VO generation. 

---

# Architecture

* In step 1, SP receives the dataset from DP and builds the hierarchical navigable small world graphs (HNSW) index on the dataset.

* In step 2, SP invokes the ADS Construction Smart Contract to build local ADS.

* In step 3, we store  ADS Merkle root, denoted as $VO_{chain}$, on the blockchain as the execution result of the ADS construction smart contract. 

* In step 4, $SP$ offers a verifiable query API to users. Given a user query $Q$, $SP$ searches the index to obtain $I_q$ and $R$, then generates $VO_{sp}$ for $I_q$ based on the ADS. Upon receiving $\{R, I_q, VO_{sp}\}$ from $SP$, the client user retrieves $VO_{chain}$. Combining the two cryptographic proofs, $VO_{sp}$ and $VO_{chain}$, the user can verify $I_q$'s correctness and subsequently execute a local search algorithm to validate $R$.

![image](</picture/architecture.JPG>)

# Quick Start

First you can install vnmslib from source and compile the source code.

Then you can use vnmslib in python.









1.**Start the distributed Fabric network**：we have a cluster of IP 226,227,228,229,230

The network configuration is in [baseline directory](/baseline) 

```shell
230
cd /home/ubuntu/local-user-name/fabric-v1.4/2org1peercouchdb
docker-compose -f org1-230-couch.yaml up -d
229
cd /home/ubuntu/local-user-name/fabric-v1.4/2org1peercouchdb
docker-compose -f org1-229-1.4.0.yaml up -d
228
cd /home/ubuntu/local-user-name/fabric-v1.4/2org1peercouchdb
docker-compose -f org2-228-1.4.0.yaml up -d
227 
cd /home/ubuntu/local-user-name
docker-compose -f org2-227-couch.yaml up -d
226
cd /home/local-user-name/caliper/packages/caliper-samples/network/fabric-v1.4/2org1peercouchdb
docker-compose -f ca-orderer.yaml up -d
```

2.**Create mychannel**:

```shell
229
docker exec -e "CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp" peer0.org1.example.com peer channel create -o orderer.example.com:7050 -c mychannel -f /etc/hyperledger/configtx/mychannel.tx

docker exec -e "CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp" peer0.org1.example.com peer channel fetch config -o orderer.example.com:7050 -c mychannel mychannel.block

docker exec -e "CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp" peer0.org1.example.com peer  channel join -b mychannel.block

228
docker exec -e "CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org2.example.com/msp" peer0.org2.example.com peer channel fetch config -o orderer.example.com:7050 -c mychannel mychannel.block

docker exec -e "CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org2.example.com/msp" peer0.org2.example.com peer  channel join -b mychannel.block
```