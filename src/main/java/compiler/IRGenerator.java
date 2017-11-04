package compiler;

import asm.Label;
import ast.*;
import entity.DefinedFunction;
import entity.LocalScope;
import exception.JumpError;
import exception.SemanticException;
import ir.*;
import type.TypeTable;
import util.ErrorHandler;

import java.util.*;

/**
 * Created by sulvto on 17-11-4.
 */
public class IRGenerator implements ASTVisitor<Void, Expr> {
    private final TypeTable typeTable;
    private final ErrorHandler errorHandler;

    public IRGenerator(TypeTable typeTable, ErrorHandler errorHandler) {
        this.typeTable = typeTable;
        this.errorHandler = errorHandler;
    }

    public IR generate(AST ast) throws SemanticException {
        ast.definedVariables().forEach(definedVariable -> {
            if (definedVariable.hasInitializer()) {
                definedVariable.setIR(transformExpr(definedVariable.initializer()));
            }
        });
        ast.definedFunctions().forEach(function -> {
            function.setIR(compileFunctionBody(function));
        });

        if (errorHandler.errorOccured()) {
            throw new SemanticException("IR generation failed");
        }
        return ast.ir();
    }


    List<Stmt> stmts;
    Stack<LocalScope> scopeStack;
    Stack<Label> breakStack;
    Stack<Label> continueStack;
    Map<String, JumpEntry> jumpMap;

    private List<Stmt> compileFunctionBody(DefinedFunction function) {
        stmts = new ArrayList<>();
        scopeStack = new Stack<>();
        breakStack = new Stack<>();
        continueStack = new Stack<>();
        jumpMap = new HashMap();
        transformStmt(function.body());
        checkJumpLinks(jumpMap);
        return stmts;
    }

    private void transformStmt(StmtNode node) {
        node.accept(this);
    }

    private void transformStmt(ExprNode node) {
        node.accept(this);
    }

    private int exprNestLevel = 0;

    private Expr transformExpr(ExprNode node) {
        exprNestLevel++;
        Expr expr = node.accept(this);
        exprNestLevel--;
        return expr;
    }

    private boolean isStatement() {
        return exprNestLevel == 0;
    }


    @Override
    public Void visit(BlockNode node) {
        return null;
    }

    @Override
    public Void visit(ExprStmtNode node) {
        return null;
    }

    @Override
    public Void visit(IfNode node) {
        Label thenLabel = new Label();
        Label elseLabel = new Label();
        Label endLabel = new Label();
        Expr cond = transformExpr(node.cond());
        if (node.elseBody() == null) {
            cjump(node.location(), cond, thenLabel, endLabel);
            label(thenLabel);
            transformStmt(node.thenBody());
            label(endLabel);
        } else {
            cjump(node.location(), cond, thenLabel, elseLabel);
            label(thenLabel);
            transformStmt(node.thenBody());
            jump(endLabel);
            label(elseLabel);
            transformStmt(node.elseBody());
            label(endLabel);
        }
        return null;
    }


    @Override
    public Void visit(SwitchNode node) {
        return null;
    }

    @Override
    public Void visit(CaseNode node) {
        return null;
    }

    @Override
    public Void visit(WhileNode node) {
        Label begLabel = new Label();
        Label bodyLabel = new Label();
        Label endLabel = new Label();

        label(begLabel);
        cjump(node.location(), transformExpr(node.cond()), bodyLabel, endLabel);
        label(bodyLabel);
        pushContinue(begLabel);
        pushBreak(endLabel);
        transformStmt(node.body());
        popBreak();
        popContinue();
        jump(begLabel);
        label(endLabel);

        return null;
    }

    private void popBreak() {
        breakStack.pop();
    }

    private void popContinue() {
        continueStack.pop();
    }

    private void pushBreak(Label label) {
        breakStack.push(label);
    }

    private void pushContinue(Label label) {
        continueStack.push(label);
    }

    private Label currentBreakTarget() {
        if (breakStack.empty()) {
            throw new JumpError("break from out on loop");
        }
        return breakStack.peek();
    }

    private Label currentContinueTarget() {
        if (continueStack.empty()) {
            throw new JumpError("continue from out on loop");
        }

        return continueStack.peek();
    }

    @Override
    public Void visit(DoWhileNode node) {
        return null;
    }

    @Override
    public Void visit(ForNode node) {
        return null;
    }

    @Override
    public Void visit(BreakNode node) {
        try {
            jump(node.location(), currentBreakTarget());
        } catch (JumpError error) {
            error(node, error.getMessage());
        }
        return null;
    }

    @Override
    public Void visit(ContinueNode node) {
        try {
            jump(node.location(), currentContinueTarget());
        } catch (JumpError error) {
            error(node, error.getMessage());
        }
        return null;
    }

    @Override
    public Void visit(ReturnNode node) {
        return null;
    }

    @Override
    public Expr visit(AssignNode node) {
        return null;
    }

    @Override
    public Expr visit(OpAssignNode node) {
        return null;
    }

    @Override
    public Expr visit(CondExprNode node) {
        return null;
    }

    @Override
    public Expr visit(LogicalAndNode node) {
        return null;
    }

    @Override
    public Expr visit(LogicalOrNode node) {
        return null;
    }

    @Override
    public Expr visit(BinaryOpNode node) {
        return null;
    }

    @Override
    public Expr visit(UnaryOpNode node) {
        return null;
    }

    @Override
    public Expr visit(FuncallNode node) {
        return null;
    }

    @Override
    public Expr visit(VariableNode node) {
        return null;
    }

    @Override
    public Expr visit(IntegerLiteralNode node) {
        return null;
    }

    @Override
    public Expr visit(StringLiteralNode node) {
        return null;
    }


    private void label(Label label) {
        stmts.add(new LabelStmt(null, label));
    }

    private void jump(Label label) {
        jump(null, label);
    }

    private void jump(String location, Label label) {
        stmts.add(new Jump(location, label));
    }

    private void cjump(String location, Expr cond, Label thenLabel, Label elseLabel) {
        stmts.add(new Cjump(location, cond, thenLabel, elseLabel));
    }


    private void error(Node node, String message) {
        errorHandler.error(node.location(), message);
    }
}
