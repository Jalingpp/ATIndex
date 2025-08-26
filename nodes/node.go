package storage

import (
	"sync"

	"ATIndex/acctrie"
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
