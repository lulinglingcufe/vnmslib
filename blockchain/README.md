# Overview

We need to obtain the time spent recording the ADS Merkle root on the blockchain, specifically in the 3rd step of the ADS construction protocol, which involves recording the $VO_{chain}$ on the blockchain.

We can obtain the time data of recording the ADS Merkle root ($VO_{chain}$) on the blockchain via the following commands.

---

* We offer the contract code in [chaincode directory](/blockchain/fabric-samples-2.2.0/asset-transfer-basic/chaincode-go)

* We offer the blockchain network configration in [test-network  directory](/blockchain/fabric-samples-2.2.0/test-network). 

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

#### 1.Start the blockchain docker network:

To start the network, go to the directory **blockchain/fabric-samples-2.2.0/test-network** and type:

```shell
./network.sh createChannel 
```

To close and clean the network, you can type:

```
./network.sh down
```

##### 2.Complie the chaincode written in Go.

```
cd /home/ubuntu/lulingling/vnmslib/blockchain/fabric-samples-2.2.0/asset-transfer-basic/chaincode-go
GO111MODULE=on go mod vendor
```

##### 3.Package  the chaincode written in Go.

```
cd /home/ubuntu/lulingling/vnmslib/blockchain/fabric-samples-2.2.0/test-network
export PATH=${PWD}/../bin:$PATH
export FABRIC_CFG_PATH=$PWD/../config/

peer lifecycle chaincode package basic.tar.gz --path ../asset-transfer-basic/chaincode-go/ --lang golang --label basic_1.0
```

You can get the chaincode as basic.tar.gz

##### 4.Install the chaincode in the peer node in org1.

```
export CORE_PEER_TLS_ENABLED=true
export CORE_PEER_LOCALMSPID="Org1MSP"
export CORE_PEER_TLS_ROOTCERT_FILE=${PWD}/organizations/peerOrganizations/org1.example.com/peers/peer0.org1.example.com/tls/ca.crt
export CORE_PEER_MSPCONFIGPATH=${PWD}/organizations/peerOrganizations/org1.example.com/users/Admin@org1.example.com/msp
export CORE_PEER_ADDRESS=localhost:7051

peer lifecycle chaincode install basic.tar.gz
```

If you successfully install the chaincode, you will see the output:

```
2023-07-06 17:42:16.427 CST [cli.lifecycle.chaincode] submitInstallProposal -> INFO 001 Installed remotely: response:<status:200 payload:"\nJbasic_1.0:a3f02ce052586adb70ab5d2ab4bf30f7f380149f464ae81c542fbde16c7dac1c\022\tbasic_1.0" > 
2023-07-06 17:42:16.427 CST [cli.lifecycle.chaincode] submitInstallProposal -> INFO 002 Chaincode code package identifier: basic_1.0:a3f02ce052586adb70ab5d2ab4bf30f7f380149f464ae81c542fbde16c7dac1c
```

a3f02ce052586adb70ab5d2ab4bf30f7f380149f464ae81c542fbde16c7dac1c is the chaincode ID.

Similarly, install the chaincode in the peer node in org2.

```
export CORE_PEER_LOCALMSPID="Org2MSP"
export CORE_PEER_TLS_ROOTCERT_FILE=${PWD}/organizations/peerOrganizations/org2.example.com/peers/peer0.org2.example.com/tls/ca.crt
export CORE_PEER_MSPCONFIGPATH=${PWD}/organizations/peerOrganizations/org2.example.com/users/Admin@org2.example.com/msp
export CORE_PEER_ADDRESS=localhost:9051

peer lifecycle chaincode install basic.tar.gz
```

Use the command to see if the contract has been successfully installed:

```
peer lifecycle chaincode queryinstalled

ubuntu@ubuntu:fabric-samples-2.2.0/test-network$ peer lifecycle chaincode queryinstalled
Installed chaincodes on peer:
Package ID: basic_1.0:a3f02ce052586adb70ab5d2ab4bf30f7f380149f464ae81c542fbde16c7dac1c, Label: basic_1.0
```

##### 5.Approve a chaincode definition in the organizations level

Approve the chaincode in Organization 2. 

```
export CC_PACKAGE_ID=basic_1.0:a3f02ce052586adb70ab5d2ab4bf30f7f380149f464ae81c542fbde16c7dac1c

peer lifecycle chaincode approveformyorg -o localhost:7050 --ordererTLSHostnameOverride orderer.example.com --channelID mychannel --name basic --version 1.0 --package-id $CC_PACKAGE_ID --sequence 1 --tls --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem"
```

You can see the output:

```
2023-07-06 18:02:40.030 CST [chaincodeCmd] ClientWait -> INFO 001 txid [ff5b64107d3a883846c3e31adc32d43027d547197d87a06f3c62802c6b8c3b2c] committed with status (VALID) at 
```

Approve the chaincode in Organization 1.

```
export CORE_PEER_LOCALMSPID="Org1MSP"
export CORE_PEER_MSPCONFIGPATH=${PWD}/organizations/peerOrganizations/org1.example.com/users/Admin@org1.example.com/msp
export CORE_PEER_TLS_ROOTCERT_FILE=${PWD}/organizations/peerOrganizations/org1.example.com/peers/peer0.org1.example.com/tls/ca.crt
export CORE_PEER_ADDRESS=localhost:7051


peer lifecycle chaincode approveformyorg -o localhost:7050 --ordererTLSHostnameOverride orderer.example.com --channelID mychannel --name basic --version 1.0 --package-id $CC_PACKAGE_ID --sequence 1 --tls --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem"
```

You can see the output:

```
2023-07-06 18:05:56.490 CST [chaincodeCmd] ClientWait -> INFO 001 txid [abd3e38a47701491beae3972ca3fe56f1025332b6f785d5031c5b1a14c1233a3] committed with status (VALID) at 
```

##### 6.Committing the chaincode definition to the channel

After a sufficient number of organizations have approved a chaincode definition, one organization can commit the chaincode definition to the channel. If a majority of channel members have approved the definition, the commit transaction will be successful and the parameters agreed to in the chaincode definition will be implemented on the channel.

You can use the peer lifecycle chaincode checkcommitreadiness command to **check whether channel members have approved the same chaincode definition.**

The command will produce a JSON map that displays if a channel member has approved the parameters that were specified in the checkcommitreadiness command:

```
peer lifecycle chaincode checkcommitreadiness --channelID mychannel --name basic --version 1.0 --sequence 1 --tls --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem" --output json
```

You can see the output:

```
ubuntu@ubuntu:/fabric-samples-2.2.0/test-network$ peer lifecycle chaincode checkcommitreadiness --channelID mychannel --name basic --version 1.0 --sequence 1 --tls --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem" --output json
{
    "approvals": {
        "Org1MSP": true,
        "Org2MSP": true
    }
}
```

Since both organizations that are members of the channel have approved the same parameters, the chaincode definition is ready to be committed to the channel. You can use the peer lifecycle chaincode commit command to **commit the chaincode definition to the channel.** The commit command also needs to be submitted by an organization admin.

```
peer lifecycle chaincode commit -o localhost:7050 --ordererTLSHostnameOverride orderer.example.com --channelID mychannel --name basic --version 1.0 --sequence 1 --tls --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem" --peerAddresses localhost:7051 --tlsRootCertFiles "${PWD}/organizations/peerOrganizations/org1.example.com/peers/peer0.org1.example.com/tls/ca.crt" --peerAddresses localhost:9051 --tlsRootCertFiles "${PWD}/organizations/peerOrganizations/org2.example.com/peers/peer0.org2.example.com/tls/ca.crt"
```

You can see the output:

```
ubuntu@ubuntu:fabric-samples-2.2.0/test-network$ peer lifecycle chaincode commit -o localhost:7050 --ordererTLSHostnameOverride orderer.example.com --channelID mychannel --name basic --version 1.0 --sequence 1 --tls --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem" --peerAddresses localhost:7051 --tlsRootCertFiles "${PWD}/organizations/peerOrganizations/org1.example.com/peers/peer0.org1.example.com/tls/ca.crt" --peerAddresses localhost:9051 --tlsRootCertFiles "${PWD}/organizations/peerOrganizations/org2.example.com/peers/peer0.org2.example.com/tls/ca.crt"
2023-07-06 18:13:30.343 CST [chaincodeCmd] ClientWait -> INFO 001 txid [4661d2fddadaf0abb896d2527ee215d7573517a2482a2c86302a6e1c58ab880c] committed with status (VALID) at localhost:7051
2023-07-06 18:13:30.343 CST [chaincodeCmd] ClientWait -> INFO 002 txid [4661d2fddadaf0abb896d2527ee215d7573517a2482a2c86302a6e1c58ab880c] committed with status (VALID) at localhost:9051
```

You can use the peer lifecycle chaincode querycommitted command to **confirm that the chaincode definition has been committed to the channel.** 

```
peer lifecycle chaincode querycommitted --channelID mychannel --name basic --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem"
```

You can see the output:

```
ubuntu@ubuntu:fabric-samples-2.2.0/test-network$ peer lifecycle chaincode querycommitted --channelID mychannel --name basic --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem"
Committed chaincode definition for chaincode 'basic' on channel 'mychannel':
Version: 1.0, Sequence: 1, Endorsement Plugin: escc, Validation Plugin: vscc, Approvals: [Org1MSP: true, Org2MSP: true]
```

##### 7. Invoking the chaincode

```
peer chaincode invoke -o localhost:7050 --ordererTLSHostnameOverride orderer.example.com --tls --cafile "${PWD}/organizations/ordererOrganizations/example.com/orderers/orderer.example.com/msp/tlscacerts/tlsca.example.com-cert.pem" -C mychannel -n basic --peerAddresses localhost:7051 --tlsRootCertFiles "${PWD}/organizations/peerOrganizations/org1.example.com/peers/peer0.org1.example.com/tls/ca.crt" --peerAddresses localhost:9051 --tlsRootCertFiles "${PWD}/organizations/peerOrganizations/org2.example.com/peers/peer0.org2.example.com/tls/ca.crt" -c '{"function":"InitLedger","Args":[]}'
```

##### 8.  Obtain the time data of recording the ADS Merkle root ($VO_{chain}$) on the blockchain

```
docker ps -a   //obtain the docker ID of chaincode
export CC_DOCKER_ID=4fcf294f4f0b  // 4fcf294f4f0b is the docker ID of chaincode

docker logs -f $CC_DOCKER_ID
```
