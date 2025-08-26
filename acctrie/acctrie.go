package acctrie

import (
	"crypto/sha256"
	"encoding/hex"
	"sync"
)

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

// getPreviousLeaf 获取前序叶子节点
func (t *AccTrie) getPreviousLeaf(current *TrieNode) *TrieNode {
	return current.Prev
}

// getNextLeaf 获取后序叶子节点
func (t *AccTrie) getNextLeaf(current *TrieNode) *TrieNode {
	return current.Next
}

// findOrCreateLeaf 找到或创建叶子节点
func (t *AccTrie) findOrCreateLeaf(key string) *TrieNode {
	current := t.Root
	path := ""

	for i := 0; i < len(key); i++ {
		char := key[i]
		path += string(char)

		if current.Children[char] == nil {
			// 创建新节点
			current.Children[char] = &TrieNode{
				IsLeaf:   i == len(key)-1,
				Children: make(map[byte]*TrieNode),
				Key:      path,
			}
		}

		current = current.Children[char]

		// 如果是叶子节点，设置后缀
		if i == len(key)-1 {
			current.Suffix = key
			current.Values = make([]string, 0)
			current.Acc = ""
		}
	}

	return current
}

// updateLeafList 更新叶子节点链表
func (t *AccTrie) updateLeafList(leaf *TrieNode) {
	// 如果叶子节点已经在链表中，先移除
	if leaf.Prev != nil {
		leaf.Prev.Next = leaf.Next
	}
	if leaf.Next != nil {
		leaf.Next.Prev = leaf.Prev
	}

	// 如果是第一个节点，直接设置为头节点
	if t.LeafList == nil {
		t.LeafList = leaf
		t.LeafTail = leaf
		leaf.Prev = nil
		leaf.Next = nil
		return
	}

	// 找到正确的插入位置（按字典序）
	current := t.LeafList
	var insertAfter *TrieNode = nil

	for current != nil {
		if current.Key > leaf.Key {
			break
		}
		insertAfter = current
		current = current.Next
	}

	// 插入节点
	if insertAfter == nil {
		// 插入到头部
		leaf.Next = t.LeafList
		leaf.Prev = nil
		t.LeafList.Prev = leaf
		t.LeafList = leaf
	} else {
		// 插入到中间或尾部
		leaf.Next = insertAfter.Next
		leaf.Prev = insertAfter
		insertAfter.Next = leaf

		if leaf.Next != nil {
			leaf.Next.Prev = leaf
		} else {
			t.LeafTail = leaf
		}
	}
}

// findNode 查找节点
func (t *AccTrie) findNode(key string) *TrieNode {
	current := t.Root

	for i := 0; i < len(key); i++ {
		char := key[i]
		if current.Children[char] == nil {
			return nil
		}
		current = current.Children[char]
	}

	return current
}

// getAllLeaves 获取所有叶子节点
func (t *AccTrie) getAllLeaves() []*TrieNode {
	t.mutex.RLock()
	defer t.mutex.RUnlock()

	leaves := make([]*TrieNode, 0)
	current := t.LeafList

	for current != nil {
		leaves = append(leaves, current)
		current = current.Next
	}

	return leaves
}

// calculateHash 计算字符串的哈希值
func (t *AccTrie) calculateHash(content string) string {
	hash := sha256.Sum256([]byte(content))
	return hex.EncodeToString(hash[:])
}
