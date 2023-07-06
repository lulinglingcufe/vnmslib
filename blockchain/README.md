# Overview

We need to obtain the time spent recording the ADS Merkle root on the blockchain, specifically in the 3rd step of the ADS construction protocol, which involves recording the $VO_{chain}$ on the blockchain.


We obtained the data on the time spent through the following commands.

---

* We offer the contract code in [source directory](source)

* We offer the blockchain network configration in [blockchain  directory](/blockchain). 

* You can also refer to the contract deployment methods in the Fabric documentation for conducting the experiments: [Deploying a smart contract to a channel &mdash; hyperledger-fabricdocs main documentation](https://hyperledger-fabric.readthedocs.io/en/release-2.2/deploy_chaincode.html)  

# Quick Start

Close the docker container:
```
docker stop $(docker ps -aq)
```
Delete the stopped docker container:
```
docker rm $(docker ps -aq -f status=exited)
docker rm -f $(docker ps -aq)
```




#### 0.To close the docker container:

```
git clone https://github.com/lulinglingcufe/vnmslib.git
```

Make sure you satisfy the above prerequisites. To compile, go to the directory **source/similarity_search** and type:

```shell
cmake .
make 
```

##### 2.Then, you must compile the customized nmslib package we developed to use it within Python3.

To compile, go to the directory **source/python_bindings*** and type:

```
rm -R -f build
rm -R -f dist
rm -R -f nmslib.egg-info
rm -R -f similarity_search
cp -R ./similarity_search/  ./python_bindings/
pip3 uninstall  nmslib  -y
python3 setup.py install
```

Delete the old compilation intermediate files, then copy the '**source/similarity_search**' folder to the '**source/python_bindings**' folder. Make sure that **pip3** has removed the old version of nmslib. Finally, use the **setup.py** script to install the customized version of nmslib.

##### 3.Finally, you can use vnmslib in python.

Go to the directory **experiment** and type:

```
python3   hnsw_6_19_glove.py
```