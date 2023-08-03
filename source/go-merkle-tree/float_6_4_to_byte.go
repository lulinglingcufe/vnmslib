package main

import (
	"fmt"
	"time"
	"encoding/binary"
	"math"
	merkletree "github.com/wealdtech/go-merkletree"
	"github.com/wealdtech/go-merkletree/keccak256"
	"encoding/json"
	"io/ioutil"
)

// 将tree结构体存储为文件
func saveMerkleTreeToFile(tree *merkletree.MerkleTree, filePath string) error {
	// 将tree结构体转换为JSON格式
	jsonData, err := json.Marshal(tree)
	if err != nil {
		return err
	}

	// 将JSON数据写入文件
	err = ioutil.WriteFile(filePath, jsonData, 0644)
	if err != nil {
		return err
	}

	return nil
}

func main() {

    var  vetctorLen int = 100

	arr := make([]float32, vetctorLen)

	for i := range arr {
		arr[i] = -0.0680247
	}

	var  dataFloat int = 7

	//arr := []float32{-0.1333, -0.1333, -0.1333, -0.1333, -0.1333, -0.1333}
	bytes := make([]byte, len(arr)*dataFloat)

	for i, v := range arr {
		bits := math.Float32bits(v)
		binary.LittleEndian.PutUint32(bytes[i*dataFloat:(i+1)*dataFloat], bits)
	}

	dataString := []byte("dsdisjdijsiifsjofjodfj")
	result := append(dataString, bytes...)
	//fmt.Println(bytes)
	//fmt.Println(result)

	// 打印结果
	//fmt.Println(bytes)

	var  dataItems int = 10000 //4096 1000000  550000
	data := make([][]byte, dataItems) //数据初始化

	for i := 0; i < dataItems; i++ {
		data[i] = result
	}

	start := time.Now() // 获取当前时间（构建索引的同时，SP构建merkle tree）

	tree, err := merkletree.NewUsing(data, keccak256.New(), false)
	if err != nil {
		panic(err)
	}
	root := tree.Root()
	fmt.Println(root)
	elapsed := time.Since(start)

	basePath := "/home/ubuntu/zgc/projects/src/hashtest/ads/ads_"
	filePath := fmt.Sprintf("%s%d", basePath, 1)
	saveMerkleTreeToFile(tree,filePath)


    fmt.Println("SP construct merkle tree Time : ", elapsed)
	

}
