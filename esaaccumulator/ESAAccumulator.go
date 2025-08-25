package esaaccumulator

import (
	"crypto/rand"
	"math/big"

	bls12381 "github.com/kilic/bls12-381"
)

type ESAAccumulator struct {
	DA []*bls12381.G1
}

// PublicKey 表示公钥结构
type PublicKey struct {
	Pub                           *PubKey
	gAlpha, gBeta, gGamma, gDelta *bls12381.G1
}

// PubKey 表示双线性配对的公钥结构
type PubKey struct {
	p, G, g *bls12381.G1
	GT      *bls12381.GT
}

// PrivateKey 表示私钥结构
type PrivateKey struct {
	s, r, alpha, beta, gamma, delta, q *big.Int
}

// Genkey 生成公钥和私钥
func Genkey(k int) (*PrivateKey, *PublicKey) {
	q := new(big.Int).Exp(big.NewInt(2), big.NewInt(int64(k)), nil) // q = 2^k

	sk := &PrivateKey{
		s:     randomBigZp(q),
		r:     randomBigZp(q),
		alpha: randomBigZp(q),
		beta:  randomBigZp(q),
		gamma: randomBigZp(q),
		delta: randomBigZp(q),
		q:     q,
	}

	pub := bilinearPairingGenerator()

	pk := &PublicKey{
		Pub:    pub,
		gAlpha: scalarBaseMult(pub.g, sk.alpha),
		gBeta:  scalarBaseMult(pub.g, sk.beta),
		gGamma: scalarBaseMult(pub.g, sk.gamma),
		gDelta: scalarBaseMult(pub.g, sk.delta),
	}

	return sk, pk
}

// bilinearPairingGenerator 生成双线性配对参数
func bilinearPairingGenerator() *PubKey {
	// 获取配对参数
	G := bls12381.NewG1()
	GT := bls12381.NewGT()
	g := G

	// 返回公钥
	return &PubKey{
		p:  G,
		G:  G,
		GT: GT,
		g:  g,
	}
}

// scalarBaseMult 计算标量乘法
func scalarBaseMult(g *bls12381.G1, scalar *big.Int) *bls12381.G1 {
	var result bls12381.G1
	result = *g              // 将g的值复制到result中
	result.MulScalar(scalar) // 执行标量乘法
	return &result
}

// randomBigZp 生成一个在Zp中的随机大整数
func randomBigZp(q *big.Int) *big.Int {
	randBytes := make([]byte, (q.BitLen()+7)/8)
	_, err := rand.Read(randBytes)
	if err != nil {
		panic("Failed to generate random number: " + err.Error())
	}
	r := new(big.Int).SetBytes(randBytes)
	r.Mod(r, q)
	r.Add(r, big.NewInt(1))
	return r
}
