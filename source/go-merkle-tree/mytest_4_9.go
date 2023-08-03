package main

import (
	 "fmt"
	"math"
	"bytes"
	"encoding/gob"	
	"os"
	"math/rand"
	 merkletree "github.com/wealdtech/go-merkletree"
	 "github.com/wealdtech/go-merkletree/keccak256"
	 "time"
	 "strconv"
)


const _letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
const _letterslen = len(_letters)

func _randomString(n int) string {
	res := make([]byte, n)
	for i := range res {
		res[i] = _letters[rand.Int63()%int64(_letterslen)]
	}
	return string(res)
}

// Example using the Merkle tree to generate and verify proofs.
func main() {
	var total_indices_number int =      704 //indices的长度

	var indices=[704]uint64{443,1769,2876,2932,3063,3151,5405,6369,6526,8645,9193,9483,9953,10413,11099,11951,12035,12204,12476,12537,12731,12781,12819,12991,13247,13913,14144,14397,14577,15063,15389,15503,15504,15727,16349,16371,16973,17433,17594,18707,18733,19533,19858,19937,20517,21359,21673,21882,21895,22175,22237,22260,22908,22995,23106,23168,23206,23210,23350,23819,24596,25073,25250,25453,25564,25926,25938,26010,26035,27155,27323,27475,27569,27735,28002,28191,28254,28294,28676,29312,29469,29711,30116,30173,30249,30338,30555,30813,30882,31690,31714,32558,32899,33069,33261,33877,33980,34068,34313,34473,34892,35464,35581,35634,35654,35721,36222,36385,36566,38070,38157,38343,38514,38804,39739,40449,40704,40993,41085,41184,41399,41518,41520,42047,42175,42342,42377,42697,42967,43029,43844,44012,44150,45191,45631,45893,47553,47911,48882,49048,49174,49222,49452,49524,49528,49771,50180,50242,50549,50578,50600,50745,50938,51099,51357,52405,52557,52681,52850,52862,52976,52993,53025,53284,53860,54411,54891,55422,56694,56707,56986,57184,57864,58505,59031,59084,59273,59378,59769,61053,61069,61280,61659,61773,61936,62432,62602,62669,63089,63458,63718,64230,64318,64514,64547,64613,65188,65283,65506,66968,67706,67944,68167,68364,69387,70505,70519,70954,71303,71323,72323,72668,72980,73163,73236,73673,73960,74143,74688,75525,75552,75788,76147,76294,77025,77450,77504,78068,78237,78279,78616,78651,78696,78810,79252,79919,80149,80268,80344,80559,81400,82461,82704,82808,82901,82964,82983,82995,83027,83053,83064,83077,83083,83162,83176,83389,83472,83554,83629,84307,84424,85552,85874,86125,86133,86385,86437,86626,86629,86672,86764,86817,86970,87348,87647,87699,87713,87867,87924,88235,88322,88567,89407,89482,89711,89756,89757,90068,90079,90092,90588,90705,90738,90757,90880,90888,91429,91505,91580,91733,92142,92201,92409,92463,92584,92684,92826,94232,95271,96581,97704,98432,98722,98866,98990,98993,99364,99756,99937,100021,100026,100171,100216,100243,100365,100655,100713,101129,101835,101961,102246,102279,102366,102978,103716,103727,104489,104554,104576,104696,105479,105499,105644,105724,106705,106720,106840,107055,107119,107215,107276,107466,107543,107695,108361,108364,108365,108411,108439,108483,108556,108582,108652,108698,108719,109621,110080,110118,110246,110375,110477,110724,110905,111058,111068,111541,111681,111821,111827,112598,112696,112754,112773,112802,113005,113159,113311,113588,113685,113815,113834,113849,113858,114075,114816,114819,115413,115590,115845,116468,116668,116827,116922,116934,117039,117130,117215,118421,119017,119111,119419,119509,119539,120514,120708,121659,121776,121914,121995,122094,122318,122388,122567,122923,123613,124118,124558,124829,126388,126449,126452,126509,126535,126580,126642,127112,127189,127219,127268,127393,127765,127958,127995,128181,128264,128485,128758,128964,129399,129873,130090,130211,130384,130876,131356,131781,131797,131890,132771,132942,133121,133176,133195,133407,133588,133648,133650,133800,134106,134126,134131,134957,135344,135685,135723,136231,136383,136562,137189,137496,137510,137596,137668,138566,138584,138748,139131,139448,139822,139893,140192,140228,140480,140529,140540,140710,140718,140720,140757,140828,140830,141063,141180,141240,141614,141812,142098,142603,142860,143170,143253,143891,144691,145452,147145,147971,148570,148924,149034,149623,149888,150735,151746,151928,152065,152770,154701,154748,155823,156830,156851,156943,157571,157591,157716,158079,158126,158860,158985,159779,160334,160380,160727,160850,161181,161466,161778,161850,161884,161982,162054,162220,162440,163241,163357,163475,163601,163609,163665,163733,163750,164437,164747,164784,165010,165785,165923,165927,165928,165938,166155,166230,166345,166488,166745,167913,167914,168137,168138,168263,168481,168559,169408,169581,169618,169923,170066,170394,170400,170467,171246,171417,171684,171915,172463,172479,173068,173096,174018,174367,174368,174728,174995,175365,175774,176062,176313,176398,176811,176885,176900,176917,176918,177161,177343,177822,178078,178397,178500,179010,179031,179187,179225,180117,180714,182167,182882,183439,183981,184005,184096,184193,184214,184713,184785,185048,185059,185329,185865,185912,186100,186172,186207,186416,186437,186954,187891,188518,189466,189608,189797,190021,190559,190704,191070,191679,191829,192017,192019,192124,192271,192426,192493,192847,192865,192923,193310,193353,193674,193734,194013,194099,194214,194472,194668,194865,194910,195206,195222,195603,195697,195986,196103,196128,196322,196355,196714,196963,197086,197090,197267,197635,197722,197864,197891,197929,197976,198728,199239,199248,199266,199287,199523,199951}




   //我给出了indice。但是需要把这个indices按照。首先我需要对indice进行划分。这样才能找到合适的分片tree。
   //我们假设：在遍历C++ set的时候，已经对indice进行划分了。indice访问的树编号，放在一个列表里面。一个树里面的indice节点，放在一个list里面。



	var  dataItems int = 200000//4096 1000000
	var  shard_granularity uint64 = 10000  // 200000 //分片粒度 5000   10000   5000 20000 5000

	//proofs := 10//1234//291   查询位置的数量
	data := make([][]byte, dataItems) //数据初始化
	for i := 0; i < dataItems; i++ {
		data[i] = []byte(_randomString(700))
	}
	//不同的数据集，这里string的长度应该是不同的把？向量本身+朋友的长度。这个值怎么算出来的呢？

	var indice_tree_number []uint64 //indice的分片树编号
	//indice_tree_number := make([]uint64, shard_number)
	//indice_tree_number[1]=0
	var per_tree_indices [][]uint64 //indice的分片树某个编号中的indices
	//per_tree_indices := make([][]uint64, shard_number)
	// var test1=[]uint64{1,2,3}
	// per_tree_indices = append(per_tree_indices, test1)

    //通过set的遍历，把 indice_tree_number 和 per_tree_indices 初始化好。
    //首先要找到第一颗树的范围。
    var  tree_number uint64 = uint64(math.Floor(float64(indices[0])/float64(shard_granularity)))
	//分片树的编号
	var bound uint64 = shard_granularity*(1+tree_number)//分片树编号代表的节点范围
	var indice_number int = 0   //分片中indice的数量
	indice_tree_number = append(indice_tree_number, tree_number)

	//var total_element_number int = 0
	for i := 0; i < total_indices_number; i++ { //1234
		if(indices[i]< bound ){
			indice_number++
			//fmt.Printf("This is indice_number++ : %v \n",indice_number)

			if(i == (total_indices_number-1)){ //遍历到最后一个indice元素，且没有增加新的树范围：
				copyData := make([]uint64, indice_number)  //初始化切片数组
				copy(copyData, indices[i+1-indice_number:])//复制切片数组
				//fmt.Printf("This is indices[i-indice_number+1:i] : %v %v \n",i+1-indice_number,i)
				per_tree_indices = append(per_tree_indices, copyData)//把数组放到 per_tree_indices 里面
				//total_element_number = total_element_number + indice_number
				//fmt.Printf("This is indice_number : %v \n",indice_number)
			}

		} else {
			//遍历到最后一个indice元素，需要增加新的树范围：
			if(i == (total_indices_number-1)){ 
            //(1)把前面的数据放好。 
			copyData := make([]uint64, indice_number)  //初始化切片数组
			copy(copyData, indices[i-indice_number:i])//复制切片数组
			per_tree_indices = append(per_tree_indices, copyData)//把数组放到 per_tree_indices 里面


			//(2)重新确定下一个元素所在的分片树的编号
			tree_number = uint64(math.Floor(float64(indices[i])/float64(shard_granularity)))
			indice_tree_number = append(indice_tree_number, tree_number)
			indice_number = 1
			//把自己放入下一个分片树的数据里面。
			copyData2 := make([]uint64, indice_number)  //初始化切片数组
			copyData2[0] = indices[i]
			per_tree_indices = append(per_tree_indices, copyData2)//把数组放到 per_tree_indices 里面

			} else { 
			//不是最后一个元素。
			copyData := make([]uint64, indice_number)  //初始化切片数组
			copy(copyData, indices[i-indice_number:i])//复制切片数组
			//fmt.Printf("This is indices[i-indice_number:i] : %v %v \n",i-indice_number,i)
			per_tree_indices = append(per_tree_indices, copyData)//把数组放到 per_tree_indices 里面

			tree_number = uint64(math.Floor(float64(indices[i])/float64(shard_granularity)))
			//重新确定下一个元素所在的分片树的编号
			indice_tree_number = append(indice_tree_number, tree_number)
			//把 分片树的编号放到 indice_tree_number里面。

			// fmt.Printf("This is tree_number : %v \n",tree_number)
			// fmt.Printf("This is indice_tree_number : %v \n",indice_tree_number)
			bound = shard_granularity*(1+tree_number)
			//fmt.Printf("This is bound : %v \n",bound)
			//total_element_number = total_element_number + indice_number
			indice_number = 1
			//fmt.Printf("This is indice_number : %v \n",indice_number)
		}

		}


	}
    
	//var total_element_number int = 0
	for i, copyData := range per_tree_indices {
		decrese_for_all_element := indice_tree_number[ indice_tree_number[i] ]*shard_granularity
		//每个元素要减去相应的：树编号*分片粒度
        for j := range copyData{
			copyData[j] = copyData[j]-decrese_for_all_element
			//total_element_number++
		}
	  }


	// fmt.Printf("This is indice_tree_number : %v \n",indice_tree_number)
	// fmt.Printf("This is per_tree_indices : %v \n",per_tree_indices)

	//fmt.Printf("This is total_element_number : %v \n",total_element_number)


    //构造hash tree，这个是一开始create index之后就要做的事情。
	//如果进行分片，那么需要构造 多棵树。我用一个列表 tree_ptr 放这些树。
	shard_number  := uint64(math.Ceil( float64(dataItems)/float64(shard_granularity))) //分片的组数
	tree_ptr := make([]*merkletree.MerkleTree, shard_number)

	var temp_j uint64 = 0
	for j := temp_j; j < shard_number; j++ {
		tree, err := merkletree.NewUsing(data[j* shard_granularity:(j+1)* shard_granularity],keccak256.New(), false)
		if err != nil {
			panic(err)
		}
		tree_ptr[j] = tree
		//fmt.Printf("This is j : %v \n",j)
	}


   start := time.Now() // 获取当前时间：SP生成VO开始计时
   indice_proof_number := make([]int, len(indice_tree_number)) //indice的分片树proof中的byte数量

   //这里需要构造一个循环。对每一个 indice数组 构造VO。
   for i, copyData := range per_tree_indices {
	//对于第i个indice数组
	increase_for_all_element := indice_tree_number[ indice_tree_number[i] ]*shard_granularity

	//(1) 每个元素要加上相应的：树编号*分片粒度，从而得到proofdata的位置
	proofData := make([][]byte, len(copyData) )
	//copyData就是{1,2,3}的indice数组。
    
	//（2）按照copyData生成proofData
	for j := 0; j < len(copyData); j++ {
		//fmt.Printf("This is copyData[j] + increase_for_all_element : %v \n",copyData[j]   +  increase_for_all_element)
		proofData[j] = data[ copyData[j]   +  increase_for_all_element ]
	}
	
	// fmt.Printf("This is i : %v \n",i)
	// fmt.Printf("This is increase_for_all_element : %v \n",increase_for_all_element)
    //（3）生成multiProof
	multiProof, err := tree_ptr[i].GenerateMultiProof(proofData)
	if err != nil {
		fmt.Println("tree_ptr[i].GenerateMultiProof failure ", err.Error())
		return
	}
	//fmt.Printf("This is multiProof : %x \n", multiProof)

    //（4）把 multiProof 写入到 文件中。
	var b bytes.Buffer
	enc := gob.NewEncoder(&b)
	enc.Encode(multiProof)
	i_str := strconv.Itoa(i)
    s := "hnswvo/output.bin_" + i_str
    file, err := os.Create(s) //这里要按照循环次数，进行命名 "output.bin"

	if err != nil {
		fmt.Println("File creation failure ", err.Error())
		return
	}
	defer file.Close() 
	b_bytes := b.Bytes()
	indice_proof_number[i] = len(b_bytes)  //把proof的byte长度存储到一个数组中
	//fmt.Printf("This is b_bytes len : %v \n",len(b_bytes))

	_, err = file.Write(b_bytes)
	if err != nil {
		fmt.Println("Encoding failure", err.Error())
		return
	}
  }


	elapsed := time.Since(start)
    fmt.Println("VO generate and sotre Time : ", elapsed)
	
	////读取VO


	start_read := time.Now() // 获取当前时间：用户验证VO开始计时
   //通过一个循环来读取VO。
   for i, copyData := range per_tree_indices {

	//start_read_proof := time.Now() 
	//对于第i个indice数组
	increase_for_all_element := indice_tree_number[ indice_tree_number[i] ]*shard_granularity
	//(1) 每个元素要加上相应的：树编号*分片粒度，从而得到proofdata的位置
	proofData := make([][]byte, len(copyData) )
	//copyData就是{1,2,3}的indice数组。
    
	//（2）按照copyData生成proofData
	for j := 0; j < len(copyData); j++ {
		//fmt.Printf("This is copyData[j] + increase_for_all_element : %v \n",copyData[j]   +  increase_for_all_element)
		proofData[j] = data[ copyData[j]   +  increase_for_all_element ]
	}
	
	// read_proof_elapsed := time.Since(start_read_proof)
    // fmt.Println("read_proof_elapsed Time in VO verfiy : ", read_proof_elapsed)


    //(3)读取第i个proof文件
	i_str := strconv.Itoa(i)
    s := "hnswvo/output.bin_" + i_str
	file_read, err := os.Open(s)
    defer file_read.Close()
	b_bytes_length := indice_proof_number[i]
	tmp_read_bytes := make([]byte, b_bytes_length)  //当时存储进文件的bytes长度
	_, err = file_read.Read(tmp_read_bytes)
	if err != nil {
		fmt.Println("Read file failure", err.Error())
		return
	}
	//（4）用proof文件生成MultiProof
	var b_read bytes.Buffer
	b_read.Write(tmp_read_bytes) //把读取出来的bytes写入buffer缓冲区
	dec := gob.NewDecoder(&b_read)
	var test_read_data merkletree.MultiProof
	err = dec.Decode(&test_read_data)
	if err != nil {
		fmt.Println("Error decoding GOB data:", err)
		return
	}
	//fmt.Printf("This is multiProof test_data : %x \n",test_read_data)

    //（5）client验证VO。 proofData就是:朋友+节点的信息。
	proven, err := merkletree.VerifyMultiProofUsing(proofData, false, &test_read_data, tree_ptr[i].Root(),keccak256.New())
	fmt.Printf("This is proven : %v \n",proven)
  }

	read_elapsed := time.Since(start_read)
    fmt.Println("VO read and verfiy Time : ", read_elapsed)








	// start := time.Now() // 获取当前时间（SP生成VO）

	// indices := make([]uint64, proofs)
	// proofData := make([][]byte, proofs)


	// for j := 0; j < proofs; j++ {
	// 	//indices[j] = uint64(rand.Int31n(int32(dataItems)))
	// 	proofData[j] = data[indices[j]]
	// }

	// //start := time.Now()
    // //SP生成VO。
	// multiProof, err := tree.GenerateMultiProof(proofData)
	// //multiProof, err := tree_ptr[1].GenerateMultiProof(proofData)


	// //fmt.Printf("This is multiProof : %x \n",multiProof)
	// // elapsed := time.Since(start)
    // // fmt.Println("VO generate Time : ", elapsed)
	
	// ////把VO写入文件。

	// //start_write := time.Now()
	// var b bytes.Buffer
	// enc := gob.NewEncoder(&b)
	// enc.Encode(multiProof)
    // file, err := os.Create("output.bin")
	// if err != nil {
	// 	fmt.Println("File creation failure ", err.Error())
	// 	return
	// }
	// defer file.Close() 
	// b_bytes := b.Bytes()
	// //fmt.Printf("This is b_bytes len : %v \n",len(b_bytes))

	// _, err = file.Write(b_bytes)
	// if err != nil {
	// 	fmt.Println("Encoding failure", err.Error())
	// 	return
	// }
	// // elapsed_write := time.Since(start_write)
    // // fmt.Println("VO sotre Time : ", elapsed_write)
	// elapsed := time.Since(start)
    // fmt.Println("VO generate and sotre Time : ", elapsed)
	
	
	// ////读取VO
	// start_read := time.Now() // 获取当前时间

	// file_read, err := os.Open("output.bin")
    // defer file_read.Close()
	// tmp_read_bytes := make([]byte, len(b_bytes))
	// _, err = file_read.Read(tmp_read_bytes)
	// if err != nil {
	// 	fmt.Println("Read file failure", err.Error())
	// 	return
	// }
	// var b_read bytes.Buffer
	// b_read.Write(tmp_read_bytes) //把读取出来的bytes写入buffer缓冲区
	// dec := gob.NewDecoder(&b_read)
	// var test_read_data merkletree.MultiProof
	// err = dec.Decode(&test_read_data)
	// if err != nil {
	// 	fmt.Println("Error decoding GOB data:", err)
	// 	return
	// }
	// //fmt.Printf("This is multiProof test_data : %x \n",test_read_data)

    // //client验证VO。 proofData就是在获取朋友+节点的信息。
	// proven, err := merkletree.VerifyMultiProofUsing(proofData, false, multiProof, tree.Root(),keccak256.New())
	// //proven, err := merkletree.VerifyMultiProofUsing(proofData, false, &test_data, tree.Root(),keccak256.New())
	
	// //VerifyMultiProofUsing([][]byte{data}, false, proof, tree.Root(), test.hashType)
	// //merkletree.VerifyMultiProof(proofData, false, multiProof, tree.Root())
	// fmt.Printf("This is proven : %v \n",proven)

	// read_elapsed := time.Since(start_read)
    // fmt.Println("VO read and verfiy Time : ", read_elapsed)
	
	// // // Data for the tree
	// // data := [][]byte{
	// // 	[]byte("Foossssssssssssssssssssssssssssssssssssssssssssss"),
	// // 	[]byte("Bar"),
	// // 	[]byte("Baz"),
	// // }

	// // // Create the tree
	// // //tree, err := merkletree.NewUsing(data, keccak256.New(), false)
	// // tree, err := merkletree.NewUsing(data, keccak256.New(), false)
	// // if err != nil {
	// // 	panic(err)
	// // }

	// // // Fetch the root hash of the tree
	// // root := tree.Root()

	// // baz := data[2]
	// // // Generate a proof for 'Baz'
	// // proof, err := tree.GenerateProof(baz, 0)//
	// // if err != nil {
	// // 	panic(err)
	// // }
	// // fmt.Printf("This is proof :%v \n",proof)

	// // // Verify the proof for 'Baz'
	// // verified, err := merkletree.VerifyProof(baz, false, proof, [][]byte{root})
	// // if err != nil {
	// // 	panic(err)
	// // }
	// // if !verified {
	// // 	panic("failed to verify proof for Baz")
	// // }
	// // fmt.Printf("This is verify : %v \n",verified)
}
