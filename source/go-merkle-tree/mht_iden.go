package main

import (
	"fmt"
	"time"
	//"encoding/binary"
	//"math"
	merkletree "github.com/wealdtech/go-merkletree"
	"github.com/wealdtech/go-merkletree/keccak256"
)

func main() {

    var  vetctorLen int = 128

	byteArray := make([]byte, vetctorLen)
	for i := 0; i < len(byteArray); i++ {
		byteArray[i] = 12
	}

	result := append([]byte("dsdisjdijsiifsjofjodfj"), byteArray...)



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
    fmt.Println("SP construct merkle tree Time : ", elapsed)
	

}
