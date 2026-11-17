#pragma once
#include <memory>
#include <string>
#include <vector>
#include <utility>
namespace llvm { class Type; }
class CodeGen;
struct BaseType;
using TypeRef = std::shared_ptr<BaseType>;

// Source types survive LLVM's opaque pointers so loads and indexing retain C widths.
struct BaseType {
    enum class TYPE { INT, CHAR, LONG, VOID, POINTER, ARRAY, STRUCT };
    explicit BaseType(TYPE type) : type(type) {}
    BaseType(TYPE type, TypeRef element, int count = 0) : type(type), element(std::move(element)), count(count) {}
    explicit BaseType(std::string tag) : type(TYPE::STRUCT), tag(std::move(tag)) {}
    TYPE GetType() const { return type; }
    TypeRef Copy() const { return std::make_shared<BaseType>(*this); }
    llvm::Type* Codegen(CodeGen* cg) const;
    static TypeRef Pointer(TypeRef element) { return std::make_shared<BaseType>(TYPE::POINTER, std::move(element)); }
    static TypeRef Array(TypeRef element, int count) { return std::make_shared<BaseType>(TYPE::ARRAY, std::move(element), count); }
    TYPE type;
    TypeRef element;
    int count{};
    std::string tag;
};
struct StructField { std::string name; TypeRef type; };
