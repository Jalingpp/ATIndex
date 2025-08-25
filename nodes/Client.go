package nodes

import (
	pb "ATIndex/proto"
	"context"
	"log"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type Client struct {
	ClientID string                  //客户端ID
	ACRPC    pb.ManagerServiceClient //审计方RPC对象，用于调用审计方的方法

}

// 新建客户端，dn和pn分别是数据块和校验块的数量
func NewClient(id string, ma_addr string) *Client {
	// 设置连接审计方服务器的地址
	conn, err := grpc.Dial(ma_addr, grpc.WithTransportCredentials(insecure.NewCredentials()), grpc.WithBlock())
	if err != nil {
		log.Println("grpc.Dial err")
		log.Fatalf("did not connect to auditor: %v", err)
	}
	// defer conn.Close()
	marpc := pb.NewManagerServiceClient(conn)
	return &Client{id, marpc}
}

// 客户端存放文件, 返回证明大小（单位：字节）
func (client *Client) PutFile(fileId string, keywords []string) int {
	//1-构建存储请求消息
	stor_req := &pb.PutFileRequest{
		ClientId: client.ClientID,
		FileId:   fileId,
		Keywords: keywords,
	}

	// 2-发送存储请求给审计方
	pf_res, err := client.ACRPC.InsertKV(context.Background(), stor_req)
	if err != nil {
		log.Fatalf("auditor could not process request: %v", err)
	}

	// 3-验证存储证明

	// 4-统计存储证明大小

	return 0
}
