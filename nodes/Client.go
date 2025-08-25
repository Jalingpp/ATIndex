package nodes

type Client struct {
	ClientID string                        //客户端ID
	ACRPC    pb.ACServiceClient            //审计方RPC对象，用于调用审计方的方法
	SNRPCs   map[string]pb.SNServiceClient //存储节点RPC对象列表，key:存储节点地址
}
