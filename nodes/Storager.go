package nodes

import (
	pb "ATIndex/proto"
	"context"
	"log"
	"net"
	"strings"

	"google.golang.org/grpc"
)

type Storager struct {
	SNId                              string //存储节点id
	SNAddr                            string //存储节点ip地址
	pb.UnimplementedSNMAServiceServer        // 面向审计方的服务器嵌入匿名字段
}

// 新建存储分片
func NewStorager(snid string, snaddr string) *Storager {
	sn := &Storager{snid, snaddr, pb.UnimplementedSNMAServiceServer{}}
	//设置监听地址
	//将监听地址设置为localhost:port
	port := strings.Split(snaddr, ":")[1]
	newsnaddr := "0.0.0.0:" + port
	lis, err := net.Listen("tcp", newsnaddr)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	pb.RegisterSNMAServiceServer(s, sn)
	log.Println("Server listening on " + newsnaddr)
	go func() {
		if err := s.Serve(lis); err != nil {
			log.Fatalf("failed to serve: %v", err)
		}
	}()
	return sn
}

// 【供Manager使用的RPC】插入KV到本地AccTrie树中，返回插入证明
func (sn *Storager) InsertKVToSN(ctx context.Context, sreq *pb.InsertKVRequest) (*pb.InsertKVResponse, error) {

	return &pb.InsertKVResponse{}, nil
}

// 【供Manager使用的RPC】在本地AccTrie树中删除KV，返回删除证明
func (sn *Storager) DeleteKVOnSN(ctx context.Context, sreq *pb.DeleteKVRequest) (*pb.DeleteKVResponse, error) {

	return &pb.DeleteKVResponse{}, nil
}

// 【供Manager使用的RPC】在本地AccTrie树中执行关键词查询，返回查询结果及其证明或不存在证明
func (sn *Storager) SingleKeywordQueryOnSN(ctx context.Context, sreq *pb.SKQSNRequest) (*pb.SKQSNResponse, error) {

	return &pb.SKQSNResponse{}, nil
}
