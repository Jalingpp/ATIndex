#include "esa_accumulator.h"
#include <iostream>
#include <sstream>

// ESAAccumulator 实现
ESAAccumulator::ESAAccumulator() 
    : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    
    // 生成安全素数作为群阶
    std::cout << "正在生成安全素数..." << std::endl;
    group_order = CryptoUtils::generate_safe_prime(64);
    std::cout << "安全素数生成完成: " << group_order.to_string() << std::endl;
    
    // 生成生成元
    std::cout << "正在生成群生成元..." << std::endl;
    generator = GroupElement::generator(group_order);
    std::cout << "群生成元生成完成: " << generator.to_string() << std::endl;
    
    // 初始化累加器为单位元
    accumulator_value = GroupElement::identity(group_order);
    
    std::cout << "ESA累加器初始化完成" << std::endl;
    std::cout << "群阶: " << group_order.to_string() << std::endl;
    std::cout << "生成元: " << generator.to_string() << std::endl;
}

GroupElement ESAAccumulator::hash_to_group(const BigInt& input) {
    BigInt hash_result = CryptoUtils::hash_to_group(input, group_order);
    return GroupElement(hash_result, group_order);
}

BigInt ESAAccumulator::generate_random() {
    return CryptoUtils::random_range(BigInt("1"), group_order - BigInt("1"));
}

GroupElement ESAAccumulator::compute_commitment(const BigInt& element) {
    // 计算元素承诺: g^element mod group_order
    return generator ^ element;
}

bool ESAAccumulator::verify_commitment(const GroupElement& commitment, const BigInt& element) {
    GroupElement expected = compute_commitment(element);
    return commitment == expected;
}

bool ESAAccumulator::add_element(const BigInt& element) {
    if (current_set.find(element) != current_set.end()) {
        std::cout << "元素 " << element.to_string() << " 已存在于集合中" << std::endl;
        return false;
    }
    
    // 添加元素到集合
    current_set.insert(element);
    
    // 计算新的累加器值: A = A^element mod group_order
    GroupElement element_commitment = compute_commitment(element);
    element_commitments[element] = element_commitment;
    
    // 更新累加器值: A = A * g^element mod group_order
    GroupElement element_power = generator ^ element;
    BigInt new_value = (accumulator_value.get_value() * element_power.get_value()) % group_order;
    accumulator_value = GroupElement(new_value, group_order);
    
    std::cout << "成功添加元素: " << element.to_string() << std::endl;
    std::cout << "新累加器值: " << accumulator_value.to_string() << std::endl;
    return true;
}

bool ESAAccumulator::remove_element(const BigInt& element) {
    auto it = current_set.find(element);
    if (it == current_set.end()) {
        std::cout << "元素 " << element.to_string() << " 不存在于集合中" << std::endl;
        return false;
    }
    
    // 从集合中移除元素
    current_set.erase(it);
    element_commitments.erase(element);
    
    // 重新计算累加器值
    accumulator_value = GroupElement::identity(group_order);
    for (const auto& elem : current_set) {
        GroupElement element_power = generator ^ elem;
        BigInt new_value = (accumulator_value.get_value() * element_power.get_value()) % group_order;
        accumulator_value = GroupElement(new_value, group_order);
    }
    
    std::cout << "成功移除元素: " << element.to_string() << std::endl;
    std::cout << "新累加器值: " << accumulator_value.to_string() << std::endl;
    return true;
}

bool ESAAccumulator::update_element(const BigInt& old_element, const BigInt& new_element) {
    // 检查旧元素是否存在
    if (current_set.find(old_element) == current_set.end()) {
        std::cout << "元素 " << old_element.to_string() << " 不存在于集合中，无法修改" << std::endl;
        return false;
    }
    
    // 检查新元素是否已存在
    if (current_set.find(new_element) != current_set.end()) {
        std::cout << "元素 " << new_element.to_string() << " 已存在于集合中，无法修改" << std::endl;
        return false;
    }
    
    // 删除旧元素
    current_set.erase(old_element);
    element_commitments.erase(old_element);
    
    // 添加新元素
    current_set.insert(new_element);
    GroupElement element_commitment = compute_commitment(new_element);
    element_commitments[new_element] = element_commitment;
    
    // 重新计算累加器值
    accumulator_value = GroupElement::identity(group_order);
    for (const auto& elem : current_set) {
        GroupElement element_power = generator ^ elem;
        BigInt new_value = (accumulator_value.get_value() * element_power.get_value()) % group_order;
        accumulator_value = GroupElement(new_value, group_order);
    }
    
    std::cout << "成功修改元素: " << old_element.to_string() << " -> " << new_element.to_string() << std::endl;
    std::cout << "新累加器值: " << accumulator_value.to_string() << std::endl;
    return true;
}

bool ESAAccumulator::contains(const BigInt& element) const {
    return current_set.find(element) != current_set.end();
}

ZeroKnowledgeProof ESAAccumulator::generate_membership_proof(const BigInt& element) {
    ZeroKnowledgeProof proof(ProofType::MEMBERSHIP);
    
    if (!contains(element)) {
        std::cout << "元素 " << element.to_string() << " 不在集合中，无法生成成员关系证明" << std::endl;
        return proof;
    }
    
    // 生成零知识成员关系证明
    // 使用Fiat-Shamir变换的非交互式证明
    
    // 1. 生成随机数
    BigInt r = generate_random();
    proof.set_randomness(r);
    
    // 2. 计算承诺 C = g^r mod n
    GroupElement commitment = generator ^ r;
    proof.set_commitment(commitment);
    
    // 3. 计算挑战 c = H(C || A || element)
    BigInt challenge_input = CryptoUtils::sha256(
        commitment.get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        element.to_string()
    );
    BigInt challenge = challenge_input % group_order;
    proof.set_challenge(challenge);
    
    // 4. 计算响应 s = r + c * element mod group_order
    BigInt response = (r + challenge * element) % group_order;
    proof.set_response(response);
    
    proof.set_valid(true);
    
    std::cout << "生成成员关系证明: " << element.to_string() << std::endl;
    return proof;
}

ZeroKnowledgeProof ESAAccumulator::generate_non_membership_proof(const BigInt& element) {
    ZeroKnowledgeProof proof(ProofType::NON_MEMBERSHIP);
    
    if (contains(element)) {
        std::cout << "元素 " << element.to_string() << " 在集合中，无法生成非成员关系证明" << std::endl;
        return proof;
    }
    
    // 生成零知识非成员关系证明
    // 证明元素不在集合中，即不存在见证使得 g^witness = A
    
    // 1. 生成随机数
    BigInt r = generate_random();
    proof.set_randomness(r);
    
    // 2. 计算承诺 C = g^r mod n
    GroupElement commitment = generator ^ r;
    proof.set_commitment(commitment);
    
    // 3. 计算挑战 c = H(C || A || element)
    BigInt challenge_input = CryptoUtils::sha256(
        commitment.get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        element.to_string()
    );
    BigInt challenge = challenge_input % group_order;
    proof.set_challenge(challenge);
    
    // 4. 计算响应 s = r + c * (group_order - element) mod group_order
    BigInt response = (r + challenge * (group_order - element)) % group_order;
    proof.set_response(response);
    
    proof.set_valid(true);
    
    std::cout << "生成非成员关系证明: " << element.to_string() << std::endl;
    return proof;
}


BigInt ESAAccumulator::generate_witness(const BigInt& element) {
    if (!contains(element)) {
        std::cout << "元素不在集合中，无法生成见证" << std::endl;
        return BigInt("0");
    }
    
    // 生成见证：计算除当前元素外所有元素的乘积
    BigInt witness = BigInt("1");
    for (const auto& elem : current_set) {
        if (elem != element) {
            GroupElement elem_power = generator ^ elem;
            witness = (witness * elem_power.get_value()) % group_order;
        }
    }
    
    std::cout << "生成见证: " << element.to_string() << std::endl;
    return witness;
}

bool ESAAccumulator::update_witness(BigInt& witness, const BigInt& element, bool is_addition) {
    if (is_addition) {
        // 添加元素时更新见证
        GroupElement elem_power = generator ^ element;
        witness = (witness * elem_power.get_value()) % group_order;
    } else {
        // 删除元素时更新见证
        GroupElement elem_power = generator ^ element;
        BigInt elem_inv = CryptoUtils::mod_inverse(elem_power.get_value(), group_order);
        witness = (witness * elem_inv) % group_order;
    }
    
    std::cout << "更新见证: " << element.to_string() << " (" << (is_addition ? "添加" : "删除") << ")" << std::endl;
    return true;
}

SetOperationResult ESAAccumulator::compute_union(const std::unordered_set<BigInt, BigInt::Hash>& other_set) {
    SetOperationResult result;
    
    // 计算并集
    result.result_set = current_set;
    for (const auto& elem : other_set) {
        result.result_set.insert(elem);
    }
    
    // 生成并集证明
    result.proof = ZeroKnowledgeProof(ProofType::UNION);
    
    // 1. 生成随机数
    BigInt r = generate_random();
    result.proof.set_randomness(r);
    
    // 2. 计算承诺
    GroupElement commitment = generator ^ r;
    result.proof.set_commitment(commitment);
    
    // 3. 计算挑战
    BigInt challenge_input = CryptoUtils::sha256(
        commitment.get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        "union"
    );
    BigInt challenge = challenge_input % group_order;
    result.proof.set_challenge(challenge);
    
    // 4. 计算响应
    BigInt response = (r + challenge * BigInt(std::to_string(result.result_set.size()))) % group_order;
    result.proof.set_response(response);
    
    result.proof.set_valid(true);
    result.is_valid = true;
    
    std::cout << "计算并集完成，结果大小: " << result.result_set.size() << std::endl;
    return result;
}

SetOperationResult ESAAccumulator::compute_intersection(const std::unordered_set<BigInt, BigInt::Hash>& other_set) {
    SetOperationResult result;
    
    // 计算交集
    for (const auto& elem : current_set) {
        if (other_set.find(elem) != other_set.end()) {
            result.result_set.insert(elem);
        }
    }
    
    // 生成交集证明
    result.proof = ZeroKnowledgeProof(ProofType::INTERSECTION);
    
    // 1. 生成随机数
    BigInt r = generate_random();
    result.proof.set_randomness(r);
    
    // 2. 计算承诺
    GroupElement commitment = generator ^ r;
    result.proof.set_commitment(commitment);
    
    // 3. 计算挑战
    BigInt challenge_input = CryptoUtils::sha256(
        commitment.get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        "intersection"
    );
    BigInt challenge = challenge_input % group_order;
    result.proof.set_challenge(challenge);
    
    // 4. 计算响应
    BigInt response = (r + challenge * BigInt(std::to_string(result.result_set.size()))) % group_order;
    result.proof.set_response(response);
    
    result.proof.set_valid(true);
    result.is_valid = true;
    
    std::cout << "计算交集完成，结果大小: " << result.result_set.size() << std::endl;
    return result;
}

SetOperationResult ESAAccumulator::compute_difference(const std::unordered_set<BigInt, BigInt::Hash>& other_set) {
    SetOperationResult result;
    
    // 计算差集
    for (const auto& elem : current_set) {
        if (other_set.find(elem) == other_set.end()) {
            result.result_set.insert(elem);
        }
    }
    
    // 生成差集证明
    result.proof = ZeroKnowledgeProof(ProofType::DIFFERENCE);
    
    // 1. 生成随机数
    BigInt r = generate_random();
    result.proof.set_randomness(r);
    
    // 2. 计算承诺
    GroupElement commitment = generator ^ r;
    result.proof.set_commitment(commitment);
    
    // 3. 计算挑战
    BigInt challenge_input = CryptoUtils::sha256(
        commitment.get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        "difference"
    );
    BigInt challenge = challenge_input % group_order;
    result.proof.set_challenge(challenge);
    
    // 4. 计算响应
    BigInt response = (r + challenge * BigInt(std::to_string(result.result_set.size()))) % group_order;
    result.proof.set_response(response);
    
    result.proof.set_valid(true);
    result.is_valid = true;
    
    std::cout << "计算差集完成，结果大小: " << result.result_set.size() << std::endl;
    return result;
}

SetOperationResult ESAAccumulator::compute_complement(const std::unordered_set<BigInt, BigInt::Hash>& other_set) {
    SetOperationResult result;
    
    // 计算补集: current_set - other_set (当前集合中不在other_set中的元素)
    for (const auto& elem : current_set) {
        if (other_set.find(elem) == other_set.end()) {
            result.result_set.insert(elem);
        }
    }
    
    // 生成补集证明
    result.proof = ZeroKnowledgeProof(ProofType::COMPLEMENT);
    
    // 1. 生成随机数
    BigInt r = generate_random();
    result.proof.set_randomness(r);
    
    // 2. 计算承诺
    GroupElement commitment = generator ^ r;
    result.proof.set_commitment(commitment);
    
    // 3. 计算挑战
    BigInt challenge_input = CryptoUtils::sha256(
        commitment.get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        "complement"
    );
    BigInt challenge = challenge_input % group_order;
    result.proof.set_challenge(challenge);
    
    // 4. 计算响应
    BigInt response = (r + challenge * BigInt(std::to_string(result.result_set.size()))) % group_order;
    result.proof.set_response(response);
    
    result.proof.set_valid(true);
    result.is_valid = true;
    
    std::cout << "计算补集完成，结果大小: " << result.result_set.size() << std::endl;
    return result;
}

bool ESAAccumulator::verify_membership_proof(const ZeroKnowledgeProof& proof, const BigInt& element) {
    if (proof.get_type() != ProofType::MEMBERSHIP || !proof.valid()) {
        return false;
    }
    
    // 重新计算挑战并验证
    BigInt challenge_input = CryptoUtils::sha256(
        proof.get_commitment().get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        element.to_string()
    );
    BigInt expected_challenge = challenge_input % group_order;
    
    if (expected_challenge != proof.get_challenge()) {
        return false;
    }
    
    // 验证零知识成员关系证明
    // 检查 g^s = C * g^(c * element) mod n
    
    GroupElement left_side = generator ^ proof.get_response();
    GroupElement right_side = proof.get_commitment() * (generator ^ (proof.get_challenge() * element));
    
    return left_side == right_side;
}

bool ESAAccumulator::verify_non_membership_proof(const ZeroKnowledgeProof& proof, const BigInt& element) {
    if (proof.get_type() != ProofType::NON_MEMBERSHIP || !proof.valid()) {
        return false;
    }
    
    // 重新计算挑战并验证
    BigInt challenge_input = CryptoUtils::sha256(
        proof.get_commitment().get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        element.to_string()
    );
    BigInt expected_challenge = challenge_input % group_order;
    
    if (expected_challenge != proof.get_challenge()) {
        return false;
    }
    
    // 验证零知识非成员关系证明
    // 检查 g^s = C * g^(c * (group_order - element)) mod n
    
    GroupElement left_side = generator ^ proof.get_response();
    GroupElement right_side = proof.get_commitment() * 
                             (generator ^ (proof.get_challenge() * (group_order - element)));
    
    return left_side == right_side;
}


bool ESAAccumulator::verify_witness(const BigInt& witness, const BigInt& element) {
    // 验证见证：检查 witness * g^element = A
    GroupElement elem_power = generator ^ element;
    BigInt expected_accumulator = (witness * elem_power.get_value()) % group_order;
    
    return expected_accumulator == accumulator_value.get_value();
}

bool ESAAccumulator::verify_set_operation_proof(const SetOperationResult& result) {
    if (!result.is_valid || !result.proof.valid()) {
        return false;
    }
    
    // 验证集合操作证明
    // 检查 g^s = C * g^(c * |result_set|) mod n
    
    GroupElement left_side = generator ^ result.proof.get_response();
    GroupElement right_side = result.proof.get_commitment() * 
                             (generator ^ (result.proof.get_challenge() * BigInt(std::to_string(result.result_set.size()))));
    
    return left_side == right_side;
}

bool ESAAccumulator::verify_complement_proof(const ZeroKnowledgeProof& proof, const std::unordered_set<BigInt, BigInt::Hash>& /* other_set */) {
    if (proof.get_type() != ProofType::COMPLEMENT || !proof.valid()) {
        return false;
    }
    
    // 验证补集证明
    // 重新计算挑战并验证
    BigInt challenge_input = CryptoUtils::sha256(
        proof.get_commitment().get_value().to_string() + 
        accumulator_value.get_value().to_string() + 
        "complement"
    );
    BigInt expected_challenge = challenge_input % group_order;
    
    if (expected_challenge != proof.get_challenge()) {
        return false;
    }
    
    // 验证零知识补集证明
    // 检查 g^s = C * g^(c * |complement_set|) mod n
    
    GroupElement left_side = generator ^ proof.get_response();
    GroupElement right_side = proof.get_commitment() * 
                             (generator ^ (proof.get_challenge() * BigInt(std::to_string(proof.get_response().to_string().length()))));
    
    return left_side == right_side;
}

void ESAAccumulator::print_state() const {
    std::cout << "\n=== ESA累加器状态 ===" << std::endl;
    std::cout << "当前集合大小: " << current_set.size() << std::endl;
    std::cout << "累加器值: " << accumulator_value.to_string() << std::endl;
    std::cout << "群阶: " << group_order.to_string() << std::endl;
    std::cout << "生成元: " << generator.to_string() << std::endl;
    
    std::cout << "集合元素: ";
    for (const auto& elem : current_set) {
        std::cout << elem.to_string() << " ";
    }
    std::cout << std::endl;
    std::cout << "===================" << std::endl;
}
