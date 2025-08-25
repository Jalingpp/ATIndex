package acctrie

import "sync"

// TrieNode AccTrie树节点
type TrieNode struct {
	IsLeaf   bool
	Children map[byte]*TrieNode // 非叶节点的指针数组
	Suffix   string             // 叶子节点的后缀
	Values   []string           // 叶子节点的值集合
	Acc      string             // 累加器值
	Prev     *TrieNode          // 前向指针
	Next     *TrieNode          // 后向指针
	Key      string             // 从根到叶的完整路径
}

// AccTrie AccTrie树
type AccTrie struct {
	Root     *TrieNode
	LeafList *TrieNode // 叶子节点链表头
	LeafTail *TrieNode // 叶子节点链表尾
	mutex    sync.RWMutex
}

// NewAccTrie 创建新的AccTrie树
func NewAccTrie() *AccTrie {
	return &AccTrie{
		Root: &TrieNode{
			IsLeaf:   false,
			Children: make(map[byte]*TrieNode),
		},
	}
}
