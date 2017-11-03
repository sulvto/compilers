package compiler;

import ast.*;
import entity.DefinedVariable;
import exception.SemanticError;
import exception.SemanticException;
import type.TypeTable;
import util.ErrorHandler;

/**
 * Created by sulvto on 17-11-2.
 */
public class DereferenceChecker extends Visitor {
    private final TypeTable typeTable;
    private final ErrorHandler errorHandler;

    public DereferenceChecker(TypeTable typeTable, ErrorHandler errorHandler) {
        this.typeTable = typeTable;
        this.errorHandler = errorHandler;
    }

    public void check(AST ast) throws SemanticException {
        ast.definedVariables().forEach(this::checkToplevelVariable);
        ast.definedFunctions().forEach(function -> check(function.body()));

        if (errorHandler.errorOccured()) {
            throw new SemanticException("compile failed.");
        }
    }

    private void check(StmtNode stmtNode) {
        stmtNode.accept(this);
    }

    private void check(ExprNode exprNode) {
        exprNode.accept(this);
    }

    private void checkToplevelVariable(DefinedVariable definedVariable) {
        checkVariable(definedVariable);
        if (definedVariable.hasInitializer()) {
            try {
                checkConstant(definedVariable.initializer());
            } catch (SemanticError error) {

            }
        }
    }

    private void checkConstant(ExprNode exprNode) {
        if (!exprNode.isConstant()) {
            errorHandler.error(exprNode.location(), "not a constant");
        }
    }

    private void checkVariable(DefinedVariable definedVariable) {
        if (definedVariable.hasInitializer()) {
            try {
                check(definedVariable.initializer());
            } catch (SemanticError error) {

            }
        }
    }


    @Override
    public Void visit(BlockNode node) {
        node.variables().forEach(this::checkVariable);
        for (StmtNode stmtNode : node.stmts()) {
            try {
                check(stmtNode);
            } catch (SemanticError semanticError) {

            }
        }

        return null;
    }

    @Override
    public Void visit(AssignNode node) {
        super.visit(node);
        checkAssignment(node);
        return null;
    }

    @Override
    public Void visit(OpAssignNode node) {
        super.visit(node);
        checkAssignment(node);
        return null;
    }

    private void checkAssignment(AbstractAssignNode node) {
        if (node.lhs().isAssignable()) {
            semanticError(node.location(), "invalid lhs expression");
        }
    }
}
