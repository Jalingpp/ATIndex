package acctrie

import (
	"crypto/sha256"
	"fmt"
	"math/big"
)

// AccTrie 累加器前缀树
type AccTrie struct {
	root Node
}

// Node 树节点接口
type Node interface {
	IsLeaf() bool
}

// InternalNode 非叶节点：包含指针数组
type InternalNode struct {
	children []Node // 指针数组，索引对应字符
}

// LeafNode 叶子节点：包含后缀、值集合、累加器值、前后向指针
type LeafNode struct {
	suffix string        // 后缀
	values []interface{} // 值集合
	acc    *big.Int      // 累加器值
	prev   *LeafNode     // 前向指针
	next   *LeafNode     // 后向指针
	key    string        // 从根节点到叶节点的完整键
}

// NewAccTrie 创建新的累加器前缀树
func NewAccTrie() *AccTrie {
	return &AccTrie{
		root: &InternalNode{
			children: make([]Node, 256), // 假设字符集大小为256
		},
	}
}

// IsLeaf 判断是否为叶子节点
func (n *InternalNode) IsLeaf() bool {
	return false
}

// IsLeaf 判断是否为叶子节点
func (n *LeafNode) IsLeaf() bool {
	return true
}

// NewInternalNode 创建新的内部节点
func NewInternalNode() *InternalNode {
	return &InternalNode{
		children: make([]Node, 256),
	}
}

// NewLeafNode 创建新的叶子节点
func NewLeafNode(suffix string, values []interface{}, key string) *LeafNode {
	// 计算累加器值（这里使用简单的哈希作为示例）
	acc := calculateAccumulator(values)

	return &LeafNode{
		suffix: suffix,
		values: values,
		acc:    acc,
		key:    key,
	}
}

// calculateAccumulator 计算累加器值
func calculateAccumulator(values []interface{}) *big.Int {
	// 这里使用简单的哈希计算作为示例
	// 实际实现中应该使用更复杂的累加器算法
	hasher := sha256.New()
	for _, value := range values {
		hasher.Write([]byte(fmt.Sprintf("%v", value)))
	}
	hash := hasher.Sum(nil)
	return new(big.Int).SetBytes(hash)
}
