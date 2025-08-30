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

// Insert 在存储节点中插入数据
func (n *StorageNode) Insert(key string, values []string) (*acctrie.InsertResult, error) {
	n.mutex.Lock()
	defer n.mutex.Unlock()

	return n.Trie.Insert(key, values)
}

// Get 从存储节点获取数据
func (n *StorageNode) Get(key string) ([]string, bool) {
	n.mutex.RLock()
	defer n.mutex.RUnlock()

	return n.Trie.Get(key)
}

// GetAllLeaves 获取所有叶子节点
func (n *StorageNode) GetAllLeaves() []*acctrie.TrieNode {
	n.mutex.RLock()
	defer n.mutex.RUnlock()

	return n.Trie.GetAllLeaves()
}

// GetNodeInfo 获取节点信息
func (n *StorageNode) GetNodeInfo() map[string]interface{} {
	n.mutex.RLock()
	defer n.mutex.RUnlock()

	leaves := n.Trie.GetAllLeaves()
	return map[string]interface{}{
		"id":           n.ID,
		"address":      n.Address,
		"leaf_count":   len(leaves),
		"total_values": n.countTotalValues(leaves),
	}
}

// countTotalValues 计算总的值数量
func (n *StorageNode) countTotalValues(leaves []*acctrie.TrieNode) int {
	total := 0
	for _, leaf := range leaves {
		total += len(leaf.Values)
	}
	return total
}
