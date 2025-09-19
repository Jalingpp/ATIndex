package acctrie

import (
	"crypto/sha256"
	"encoding/hex"
	"strings"
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

// InsertResult 插入操作的结果
type InsertResult struct {
	LN        *TrieNode // 插入的叶子节点
	KeyP      string    // 前序叶子节点的key
	LNP       *TrieNode // 前序叶子节点
	KeyN      string    // 后序叶子节点的key
	LNN       *TrieNode // 后序叶子节点
	OldLNNAcc string    // 旧的后序叶子节点累加器
	NewLNNAcc string    // 新的后序叶子节点累加器
	OldLNAcc  string    // 旧的叶子节点累加器
	NewLNAcc  string    // 新的叶子节点累加器
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

// GetAllLeaves 获取所有叶子节点
func (t *AccTrie) GetAllLeaves() []*TrieNode {
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

// updateAccumulator 更新累加器值
func (n *TrieNode) updateAccumulator() {
	if len(n.Values) == 0 {
		n.Acc = ""
		return
	}

	// 创建累加器内容
	accContent := make([]string, 0)

	// 添加所有值
	accContent = append(accContent, n.Values...)

	// 添加前序叶子节点的累加器值
	if n.Prev != nil {
		accContent = append(accContent, n.Prev.Acc)
	}

	// 添加前序和后序叶子节点的key
	if n.Prev != nil {
		accContent = append(accContent, n.Prev.Key)
	}
	if n.Next != nil {
		accContent = append(accContent, n.Next.Key)
	}

	// 计算哈希
	hash := sha256.Sum256([]byte(strings.Join(accContent, "|")))
	n.Acc = hex.EncodeToString(hash[:])
}

// addToAccumulator 向累加器添加元素
func (t *AccTrie) addToAccumulator(acc, element string) string {
	if acc == "" {
		return t.calculateHash(element)
	}
	return t.calculateHash(acc + "|" + element)
}

// removeFromAccumulator 从累加器移除元素
func (t *AccTrie) removeFromAccumulator(acc, element string) string {
	if acc == "" {
		return ""
	}

	return t.calculateHash(strings.Replace(acc, element, "", -1))
}

// Insert 插入(key, values)到AccTrie树
func (t *AccTrie) Insert(key string, values []string) (*InsertResult, error) {
	t.mutex.Lock()
	defer t.mutex.Unlock()

	// 1. 找到或创建叶子节点
	leafNode := t.findOrCreateLeaf(key)

	// 2. 更新叶子节点的Values和Acc
	oldAcc := leafNode.Acc
	leafNode.Values = append(leafNode.Values, values...)
	leafNode.updateAccumulator()
	newAcc := leafNode.Acc

	// 3. 获取前序和后序叶子节点
	prevLeaf := t.getPreviousLeaf(leafNode)
	nextLeaf := t.getNextLeaf(leafNode)

	// 4. 更新前序叶子节点的累加器
	var keyP string
	if prevLeaf != nil {
		keyP = prevLeaf.Key
		prevLeaf.Acc = t.addToAccumulator(prevLeaf.Acc, key)
	} else {
		keyP = "NoPrev"
		leafNode.Acc = t.addToAccumulator(leafNode.Acc, "NoPrev")
		newAcc = leafNode.Acc
	}

	// 5. 更新后序叶子节点的累加器
	var keyN, oldNextAcc, newNextAcc string
	if nextLeaf != nil {
		keyN = nextLeaf.Key
		oldNextAcc = nextLeaf.Acc
		// 从后序叶子节点中删除前序叶子节点的key
		nextLeaf.Acc = t.removeFromAccumulator(nextLeaf.Acc, keyP)
		// 添加当前key到后序叶子节点
		nextLeaf.Acc = t.addToAccumulator(nextLeaf.Acc, key)
		newNextAcc = nextLeaf.Acc
	} else {
		keyN = "NoNext"
		leafNode.Acc = t.addToAccumulator(leafNode.Acc, "NoNext")
		newAcc = leafNode.Acc
	}

	// 6. 更新叶子节点链表
	t.updateLeafList(leafNode)

	return &InsertResult{
		LN:        leafNode,
		KeyP:      keyP,
		LNP:       prevLeaf,
		KeyN:      keyN,
		LNN:       nextLeaf,
		OldLNNAcc: oldNextAcc,
		NewLNNAcc: newNextAcc,
		OldLNAcc:  oldAcc,
		NewLNAcc:  newAcc,
	}, nil
}

// Get 获取key对应的值
func (t *AccTrie) Get(key string) ([]string, bool) {
	t.mutex.RLock()
	defer t.mutex.RUnlock()

	node := t.findNode(key)
	if node == nil || !node.IsLeaf {
		return nil, false
	}

	return node.Values, true
}

// calculateHash 计算字符串的哈希值
func (t *AccTrie) calculateHash(content string) string {
	hash := sha256.Sum256([]byte(content))
	return hex.EncodeToString(hash[:])
}
