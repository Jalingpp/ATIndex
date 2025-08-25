package nodes

import (
	pb "ATIndex/proto"
	"context"
)

type Manager struct {
	IpAddr string //管理者ip地址
	pb.UnimplementedMAServiceServer    // 嵌入匿名字段
	SNAddrMap   map[string]string      //存储节点的地址表，key:存储节点id，value:存储节点地址
}

// 新建一个审计方，持续监听消息
func NewManager(ipaddr string, snaddrfilename string) *Manager {
	snaddrmap := util.ReadSNAddrFile(snaddrfilename)
	//设置连接存储节点服务器的地址
	snrpcs := make(map[string]pb.SNMAServiceClient)
	for key, value := range *snaddrmap {
		snconn, err := grpc.Dial(value, grpc.WithTransportCredentials(insecure.NewCredentials()), grpc.WithBlock())
		if err != nil {
			log.Fatalf("did not connect to storage node: %v", err)
		}
		sc := pb.NewSNMAServiceClient(snconn)
		snrpcs[key] = sc
	}
	manager := &Manager{ipaddr,pb.UnimplementedMAServiceServer{},*snaddrmap}
	//设置监听地址
	port := strings.Split(ipaddr, ":")[1]
	newipaddr := "0.0.0.0:" + port
	lis, err := net.Listen("tcp", newipaddr)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	pb.RegisterMAServiceServer(s, manager)
	// log.Println("Server listening on port 50051")
	go func() {
		if err := s.Serve(lis); err != nil {
			log.Fatalf("failed to serve: %v", err)
		}
	}()
	return manager
}

// 【供Client使用的RPC】选择存储节点，给客户端回复消息
func (ma *Manager) InsertKV(ctx context.Context, sreq *pb.PutFileRequest) (*pb.PutFileResponse, error) {
	//1-将<fileId,keywords>转换为<keyword1,fileId>,<keyword2,fileId>,...

	//2-更新一致性哈希表并分配给不同SN

	//3-向SN发送插入KV请求

	//4-验证插入证明

	//5-更新一致性哈希表

	//6-返回插入证明给客户端


	return &pb.PutFileResponse{FileId: sreq.FileId,Proof: }, nil
}
