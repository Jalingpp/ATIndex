package storage

import (
	"ATIndex/acctrie"
	"sync"
)

// StorageNode 存储节点
type StorageNode struct {
	ID      string
	Address string
	Trie    *acctrie.AccTrie
	mutex   sync.RWMutex
}

// NewStorageNode 创建新的存储节点
func NewStorageNode(id, address string) *StorageNode {
	return &StorageNode{
		ID:      id,
		Address: address,
		Trie:    acctrie.NewAccTrie(),
	}
}

// GetAllLeaves 获取所有叶子节点
func (n *StorageNode) GetAllLeaves() []*acctrie.TrieNode {
	n.mutex.RLock()
	defer n.mutex.RUnlock()

	return n.Trie.GetAllLeaves()
}

// countTotalValues 计算总的值数量
func (n *StorageNode) countTotalValues(leaves []*acctrie.TrieNode) int {
	total := 0
	for _, leaf := range leaves {
		total += len(leaf.Values)
	}
	return total
}
