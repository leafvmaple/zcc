#pragma once

#include "ast/ast.h"
#include "ir/ir.h"

class Generator {
public:
    Generator(Env *env) : env(env) {};

    void Generate(CompUnitAST& ast) {
        for (auto&& funcDef : ast.funcDef) {
            Generate(std::move(funcDef));
        }
    }
    void Generate(std::unique_ptr<FuncDefAST> funcDef) {
        std::vector<void*> types;
        std::vector<std::string> names;
        
        for (auto& param : funcDef->params) {
            types.push_back(param->btype->Codegen(env));
            names.push_back(param->ident);
        }

        auto* type = env->CreateFuncType(funcDef->funcType->Codegen(env), types);
        auto* func = env->CreateFunction(type, funcDef->ident, names);
        auto* bb = env->CreateBasicBlock("entry", func);

        env->SetInserPointer(bb);

        for (size_t i = 0; i < funcDef->params.size(); ++i) {
            env->CreateStore(env->GetFunctionArg(i), funcDef->params[i]->Codegen(env));
        }

        Generate(std::move(funcDef->block));
    }
    void Generate(std::unique_ptr<BlockAST> block) {
        env->EnterScope();
        for (auto&& item : block->items)
            Generate(std::move(item));
        env->ExitScope();
    }
    void Generate(std::unique_ptr<BlockItemAST> item) {
        if (item->decl) {
            Generate(std::move(item->decl));
        } else if (item->stmt) {
            Generate(std::move(item->stmt));
        }
    }
    void Generate(std::unique_ptr<DeclAST> decl) {
        decl->Codegen(env);
    }
    void Generate(std::unique_ptr<StmtAST> stmt) {
        if (stmt->type == StmtAST::Type::Assign) {
            env->CreateStore(stmt->expr2->Codegen(env), stmt->expr1->Codegen(env));
        } else if (stmt->type == StmtAST::Type::Expr) {
            if (stmt->expr1)
                stmt->expr1->Codegen(env);
        } else if (stmt->type == StmtAST::Type::Block) {
            stmt->block->Codegen(env);
        } else if (stmt->type == StmtAST::Type::If) {
            auto* cond = stmt->expr1->Codegen(env);
            auto* func = env->GetFunction();
            auto* thenBB = env->CreateBasicBlock("then", func);
            void* endBB{};

            if (stmt->expr3) {
                auto* elseBB = env->CreateBasicBlock("else", func);
                endBB = env->CreateBasicBlock("end", func);
                env->CreateCondBr(cond, thenBB, elseBB);

                env->SetInserPointer(elseBB);
                stmt->expr3->Codegen(env);
                env->CreateBr(endBB);
            } else {
                endBB = env->CreateBasicBlock("end", func);
                env->CreateCondBr(cond, thenBB, endBB);
            }
            
            env->SetInserPointer(thenBB);
            stmt->expr2->Codegen(env);
            env->CreateBr(endBB);

            env->SetInserPointer(endBB);

        } else if (stmt->type == StmtAST::Type::Ret) {
            env->CreateRet(stmt->expr1->Codegen(env));
        } else if (stmt->type == StmtAST::Type::While) {
            void* func = env->GetFunction();
            void* condBB = env->CreateBasicBlock("while_entry", func);
            void* bodyBB = env->CreateBasicBlock("while_body", func);
            void* endBB = env->CreateBasicBlock("end", func);

            env->EnterWhile(condBB, endBB);

            env->CreateBr(condBB);
            env->SetInserPointer(condBB);

            env->CreateCondBr(stmt->expr1->Codegen(env), bodyBB, endBB);

            env->SetInserPointer(bodyBB);
            stmt->expr2->Codegen(env);
            env->CreateBr(condBB);

            env->SetInserPointer(endBB);

            env->ExitWhile();
        } else if (stmt->type == StmtAST::Type::Break) {
            env->CreateBr(env->GetWhileEnd());
        } else if (stmt->type == StmtAST::Type::Continue) {
            env->CreateBr(env->GetWhileEntry());
        }
    }

private:
    Env* env;
};