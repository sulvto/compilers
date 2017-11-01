package compiler;

import ast.*;
import entity.DefinedVariable;

import java.util.List;

/**
 * Created by sulvto on 17-11-2.
 */
public abstract class Visitor implements ASTVisitor<Void, Void> {
    public Visitor() {

    }

    public void visitStmt(StmtNode stmtNode) {
        stmtNode.accept(this);
    }

    public void visitStmts(List<? extends StmtNode> stmts) {
        for (StmtNode stmt : stmts) {
            visitStmt(stmt);
        }
    }

    public void visitExpr(ExprNode expr) {
        expr.accept(this);
    }

    public void visitExprs(List<? extends ExprNode> exprs) {
        for (ExprNode expr : exprs) {
            visitExpr(expr);
        }
    }


    @Override
    public Void visit(BlockNode node) {
        for (DefinedVariable variable : node.getVariables()) {
            if (variable.hasInitializer()) {
                visitExpr(variable.initializer());
            }
        }
        visitStmts(node.getStmts());
        return null;
    }

    @Override
    public Void visit(ExprStmtNode node) {
        visitExpr(node.expr());
        return null;
    }

    @Override
    public Void visit(IfNode node) {
        visitExpr(node.cond());
        visitStmt(node.thenBody());
        if (node.elseBody() != null) {
            visitStmt(node.elseBody());
        }
        return null;
    }


    @Override
    public Void visit(SwitchNode node) {
        // TODO
        return null;
    }

    @Override
    public Void visit(CaseNode node) {
        visitExprs(node.values());
        visitStmt(node.body());
        return null;
    }

    @Override
    public Void visit(WhileNode node) {
        visitExpr(node.cond());
        visitStmt(node.body());
        return null;
    }

    @Override
    public Void visit(DoWhileNode node) {
        visitExpr(node.cond());
        visitStmt(node.body());
        return null;
    }

    @Override
    public Void visit(ForNode node) {
//        visitExpr();
        // TODO
        return null;
    }

    @Override
    public Void visit(BreakNode node) {
        return null;
    }

    @Override
    public Void visit(ContinueNode node) {
        return null;
    }

    @Override
    public Void visit(ReturnNode node) {
        if (node.expr() != null) {
            visitExpr(node.expr());
        }
        return null;
    }


    @Override
    public Void visit(CondExprNode node) {
        visitExpr(node.cond());
        visitExpr(node.thenExpr());
        if (node.elseExpr() != null) {
            visitExpr(node.elseExpr());
        }
        return null;
    }


    @Override
    public Void visit(LogicalOrNode node) {
        visitExpr(node.left());
        visitExpr(node.right());
        return null;
    }

    @Override
    public Void visit(LogicalAndNode node) {
        visitExpr(node.left());
        visitExpr(node.right());
        return null;
    }

    @Override
    public Void visit(AssignNode node) {
        visitExpr(node.lhs());
        visitExpr(node.rhs());
        return null;
    }

    @Override
    public Void visit(OpAssignNode node) {
        visitExpr(node.lhs());
        visitExpr(node.rhs());
        return null;
    }

    @Override
    public Void visit(BinaryOpNode node) {
        visitExpr(node.left());
        visitExpr(node.right());
        return null;
    }

    @Override
    public Void visit(UnaryOpNode node) {
        visitExpr(node.expr());
        return null;
    }

    @Override
    public Void visit(FuncallNode node) {
        visitExpr(node.expr());
        visitExprs(node.args());
        return null;
    }

    @Override
    public Void visit(VariableNode node) {
        return null;
    }

    @Override
    public Void visit(StringLiteralNode node) {
        return null;
    }

    @Override
    public Void visit(IntegerLiteralNode node) {
        return null;
    }
}
