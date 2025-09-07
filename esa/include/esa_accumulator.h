#ifndef ESA_ACCUMULATOR_H
#define ESA_ACCUMULATOR_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <random>
#include <chrono>
#include <openssl/sha.h>
#include <openssl/evp.h>

// 大整数类型
class BigInt {
private:
    BIGNUM* value;
    
public:
    BigInt();
    BigInt(const std::string& str, int base = 10);
    BigInt(const BigInt& other);
    BigInt(BigInt&& other) noexcept;
    ~BigInt();
    
    BigInt& operator=(const BigInt& other);
    BigInt& operator=(BigInt&& other) noexcept;
    
    // 算术运算
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;
    BigInt operator^(const BigInt& other) const; // 模幂运算
    
    // 比较运算
    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;
    
    // 赋值运算
    BigInt& operator+=(const BigInt& other);
    BigInt& operator-=(const BigInt& other);
    BigInt& operator*=(const BigInt& other);
    BigInt& operator%=(const BigInt& other);
    
    // 工具函数
    std::string to_string(int base = 10) const;
    size_t bit_length() const;
    bool is_zero() const;
    bool is_one() const;
    
    // 获取内部BIGNUM指针（用于OpenSSL函数）
    BIGNUM* get_bn() const { return value; }
    const BIGNUM* get_const_bn() const { return value; }
    
    // 为std::unordered_set提供哈希函数
    struct Hash {
        std::size_t operator()(const BigInt& bn) const {
            return std::hash<std::string>{}(bn.to_string());
        }
    };
    
    // 静态函数
    static BigInt random(size_t bits);
    static BigInt random_range(const BigInt& min, const BigInt& max);
    static BigInt from_hex(const std::string& hex);
    static BigInt from_bytes(const std::vector<uint8_t>& bytes);
    std::vector<uint8_t> to_bytes() const;
};

// 群元素类型
class GroupElement {
private:
    BigInt value;
    BigInt modulus;
    bool is_valid;
    
    // 私有静态辅助函数
    static bool is_primitive_root(const BigInt& g, const BigInt& p, const BigInt& phi, 
                                  const std::vector<BigInt>& prime_factors);
    static std::vector<BigInt> get_prime_factors(const BigInt& n);
    
public:
    GroupElement() : is_valid(false) {}
    GroupElement(const BigInt& val, const BigInt& mod) : value(val % mod), modulus(mod), is_valid(true) {}
    
    // 群运算
    GroupElement operator*(const GroupElement& other) const;
    GroupElement operator^(const BigInt& exponent) const; // 模幂运算
    GroupElement inverse() const;
    
    // 比较运算
    bool operator==(const GroupElement& other) const;
    bool operator!=(const GroupElement& other) const;
    
    // 获取器
    const BigInt& get_value() const { return value; }
    const BigInt& get_modulus() const { return modulus; }
    bool valid() const { return is_valid; }
    
    // 工具函数
    std::string to_string() const;
    static GroupElement generator(const BigInt& modulus);
    static GroupElement identity(const BigInt& modulus);
};

// 证明类型枚举
enum class ProofType {
    MEMBERSHIP,      // 成员关系证明
    NON_MEMBERSHIP,  // 非成员关系证明
    UNION,           // 并集证明
    INTERSECTION,    // 交集证明
    DIFFERENCE,      // 差集证明
    SUBSET,          // 子集关系证明
    BATCH_MEMBERSHIP // 批量成员关系证明
};

// 零知识证明结构
class ZeroKnowledgeProof {
private:
    ProofType type;
    GroupElement commitment;
    BigInt challenge;
    BigInt response;
    std::vector<GroupElement> auxiliary_data;
    BigInt randomness;
    bool is_valid;
    
public:
    ZeroKnowledgeProof(ProofType t) : type(t), is_valid(false) {}
    
    // 设置器
    void set_commitment(const GroupElement& comm) { commitment = comm; }
    void set_challenge(const BigInt& chall) { challenge = chall; }
    void set_response(const BigInt& resp) { response = resp; }
    void set_randomness(const BigInt& rand) { randomness = rand; }
    void add_auxiliary_data(const GroupElement& aux) { auxiliary_data.push_back(aux); }
    void set_valid(bool valid) { is_valid = valid; }
    
    // 获取器
    ProofType get_type() const { return type; }
    const GroupElement& get_commitment() const { return commitment; }
    const BigInt& get_challenge() const { return challenge; }
    const BigInt& get_response() const { return response; }
    const BigInt& get_randomness() const { return randomness; }
    const std::vector<GroupElement>& get_auxiliary_data() const { return auxiliary_data; }
    bool valid() const { return is_valid; }
    
    // 序列化
    std::string serialize() const;
    static ZeroKnowledgeProof deserialize(const std::string& data);
};

// 集合操作结果
struct SetOperationResult {
    std::unordered_set<BigInt, BigInt::Hash> result_set;
    ZeroKnowledgeProof proof;
    bool is_valid;
    
    SetOperationResult() : proof(ProofType::UNION), is_valid(false) {}
};

// ESA累加器主类
class ESAAccumulator {
private:
    // 群参数
    GroupElement generator;
    GroupElement accumulator_value;
    BigInt group_order;
    
    // 当前集合
    std::unordered_set<BigInt, BigInt::Hash> current_set;
    
    // 辅助数据结构
    std::unordered_map<BigInt, GroupElement, BigInt::Hash> element_commitments;
    std::mt19937_64 rng;
    
    // 内部方法
    GroupElement hash_to_group(const BigInt& input);
    BigInt generate_random();
    GroupElement compute_commitment(const BigInt& element);
    bool verify_commitment(const GroupElement& commitment, const BigInt& element);
    
public:
    // 构造函数
    ESAAccumulator();
    ~ESAAccumulator() = default;
    
    // 基本操作
    bool add_element(const BigInt& element);
    bool remove_element(const BigInt& element);
    bool contains(const BigInt& element) const;
    
    // 零知识证明生成
    ZeroKnowledgeProof generate_membership_proof(const BigInt& element);
    ZeroKnowledgeProof generate_non_membership_proof(const BigInt& element);
    ZeroKnowledgeProof generate_subset_proof(const std::unordered_set<BigInt, BigInt::Hash>& subset);
    ZeroKnowledgeProof generate_batch_membership_proof(const std::vector<BigInt>& elements);
    
    // 见证生成和更新
    BigInt generate_witness(const BigInt& element);
    bool update_witness(BigInt& witness, const BigInt& element, bool is_addition);
    
    // 集合操作
    SetOperationResult compute_union(const std::unordered_set<BigInt, BigInt::Hash>& other_set);
    SetOperationResult compute_intersection(const std::unordered_set<BigInt, BigInt::Hash>& other_set);
    SetOperationResult compute_difference(const std::unordered_set<BigInt, BigInt::Hash>& other_set);
    
    // 证明验证
    bool verify_membership_proof(const ZeroKnowledgeProof& proof, const BigInt& element);
    bool verify_non_membership_proof(const ZeroKnowledgeProof& proof, const BigInt& element);
    bool verify_subset_proof(const ZeroKnowledgeProof& proof, const std::unordered_set<BigInt, BigInt::Hash>& subset);
    bool verify_batch_membership_proof(const ZeroKnowledgeProof& proof, const std::vector<BigInt>& elements);
    bool verify_set_operation_proof(const SetOperationResult& result);
    
    // 见证验证
    bool verify_witness(const BigInt& witness, const BigInt& element);
    
    // 获取器
    const std::unordered_set<BigInt, BigInt::Hash>& get_current_set() const { return current_set; }
    GroupElement get_accumulator_value() const { return accumulator_value; }
    size_t size() const { return current_set.size(); }
    
    // 调试和测试
    void print_state() const;
};

// 密码学工具函数
namespace CryptoUtils {
    // 哈希函数
    BigInt sha256(const BigInt& input);
    BigInt sha3_256(const BigInt& input);
    BigInt hash_to_group(const BigInt& input, const BigInt& modulus);
    
    // 素数相关
    bool is_prime(const BigInt& n, int rounds = 40);
    bool miller_rabin(const BigInt& n, int rounds);
    BigInt generate_prime(size_t bits);
    BigInt generate_safe_prime(size_t bits);
    
    // 模运算
    BigInt mod_inverse(const BigInt& a, const BigInt& m);
    BigInt mod_pow(const BigInt& base, const BigInt& exp, const BigInt& mod);
    BigInt mod_sqrt(const BigInt& a, const BigInt& p);
    
    // 随机数生成
    BigInt random_bits(size_t bits);
    BigInt random_range(const BigInt& min, const BigInt& max);
    
    // 编码/解码
    std::string to_hex(const BigInt& value);
    BigInt from_hex(const std::string& hex);
    std::vector<uint8_t> to_bytes(const BigInt& value);
    BigInt from_bytes(const std::vector<uint8_t>& bytes);
    
    // 群运算
    GroupElement hash_to_elliptic_curve(const BigInt& input, const BigInt& p, const BigInt& a, const BigInt& b);
    bool is_quadratic_residue(const BigInt& a, const BigInt& p);
}

#endif // ESA_ACCUMULATOR_H
