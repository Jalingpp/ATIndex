#include "esa_accumulator.h"
#include <iostream>

void demonstrate_basic_operations() {
    std::cout << "=== 基本操作演示 ===" << std::endl;
    
    ESAAccumulator acc;
    
    // 添加元素
    acc.add_element(BigInt("1001"));  // 用户ID
    acc.add_element(BigInt("1002"));  // 用户ID
    acc.add_element(BigInt("1003"));  // 用户ID
    
    std::cout << "集合大小: " << acc.size() << std::endl;
    std::cout << "1001在集合中: " << (acc.contains(BigInt("1001")) ? "是" : "否") << std::endl;
    std::cout << "1005在集合中: " << (acc.contains(BigInt("1005")) ? "是" : "否") << std::endl;
}

void demonstrate_set_operations() {
    std::cout << "\n=== 集合操作演示 ===" << std::endl;
    
    ESAAccumulator acc1, acc2;
    
    // 创建两个集合
    acc1.add_element(BigInt("1"));
    acc1.add_element(BigInt("2"));
    acc1.add_element(BigInt("3"));
    
    acc2.add_element(BigInt("2"));
    acc2.add_element(BigInt("3"));
    acc2.add_element(BigInt("4"));
    
    // 计算交集
    SetOperationResult intersection = acc1.compute_intersection(acc2.get_current_set());
    std::cout << "交集大小: " << intersection.result_set.size() << std::endl;
    
    // 计算并集
    SetOperationResult union_result = acc1.compute_union(acc2.get_current_set());
    std::cout << "并集大小: " << union_result.result_set.size() << std::endl;
}

void demonstrate_witness_system() {
    std::cout << "\n=== 见证系统演示 ===" << std::endl;
    
    ESAAccumulator acc;
    acc.add_element(BigInt("100"));
    acc.add_element(BigInt("200"));
    acc.add_element(BigInt("300"));
    
    // 生成见证
    BigInt witness = acc.generate_witness(BigInt("100"));
    std::cout << "见证生成成功" << std::endl;
    
    // 验证见证
    bool witness_valid = acc.verify_witness(witness, BigInt("100"));
    std::cout << "见证验证: " << (witness_valid ? "成功" : "失败") << std::endl;
    
    // 更新见证
    acc.add_element(BigInt("400"));
    acc.update_witness(witness, BigInt("400"), true);
    std::cout << "见证更新成功" << std::endl;
}


void demonstrate_complement_operations() {
    std::cout << "\n=== 补集操作演示 ===" << std::endl;
    
    ESAAccumulator acc1, acc2;
    
    // 创建两个集合
    acc1.add_element(BigInt("1"));
    acc1.add_element(BigInt("2"));
    acc1.add_element(BigInt("3"));
    acc1.add_element(BigInt("4"));
    
    acc2.add_element(BigInt("2"));
    acc2.add_element(BigInt("3"));
    acc2.add_element(BigInt("5"));
    
    // 计算补集
    SetOperationResult complement = acc1.compute_complement(acc2.get_current_set());
    std::cout << "补集大小: " << complement.result_set.size() << std::endl;
    
    // 验证补集证明
    bool complement_valid = acc1.verify_set_operation_proof(complement);
    std::cout << "补集证明验证: " << (complement_valid ? "成功" : "失败") << std::endl;
}

void demonstrate_element_update() {
    std::cout << "\n=== 元素修改演示 ===" << std::endl;
    
    ESAAccumulator acc;
    
    // 添加元素
    acc.add_element(BigInt("100"));
    acc.add_element(BigInt("200"));
    acc.add_element(BigInt("300"));
    
    std::cout << "修改前集合大小: " << acc.size() << std::endl;
    
    // 修改元素
    bool update_success = acc.update_element(BigInt("200"), BigInt("250"));
    std::cout << "元素修改: " << (update_success ? "成功" : "失败") << std::endl;
    
    std::cout << "修改后集合大小: " << acc.size() << std::endl;
    std::cout << "200在集合中: " << (acc.contains(BigInt("200")) ? "是" : "否") << std::endl;
    std::cout << "250在集合中: " << (acc.contains(BigInt("250")) ? "是" : "否") << std::endl;
}

int main() {
    std::cout << "=== ESA累加器功能演示 ===" << std::endl;
    
    try {
        demonstrate_basic_operations();
        demonstrate_set_operations();
        demonstrate_witness_system();
        demonstrate_complement_operations();
        demonstrate_element_update();
        
        std::cout << "\n=== 所有功能演示完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
