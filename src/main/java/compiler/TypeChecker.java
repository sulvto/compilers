package compiler;

import ast.AST;
import ast.ExprNode;
import ast.StmtNode;
import entity.DefinedFunction;
import entity.DefinedVariable;
import exception.SemanticException;
import type.TypeTable;
import util.ErrorHandler;

/**
 * Created by sulvto on 17-11-3.
 */
public class TypeChecker extends Visitor {
    private final TypeTable typeTable;
    private final ErrorHandler errorHandler;

    public TypeChecker(TypeTable typeTable, ErrorHandler errorHandler) {
        this.typeTable = typeTable;
        this.errorHandler = errorHandler;
    }

    DefinedFunction currentFunction;

    public void check(AST ast) throws SemanticException {
        ast.definedVariables().forEach(this::checkVariable);
        ast.definedFunctions().forEach(function -> {
            currentFunction = function;
            checkReturnType(function);
            checkParamTypes(function);
            check(function.body());
        });

        if (errorHandler.errorOccured()) {
            throw new SemanticException("compile failed.");
        }
    }

    private void check(StmtNode stmtNode) {
    }

    private void check(ExprNode exprNode) {
    }

    private void checkParamTypes(DefinedFunction function) {

    }

    private void checkReturnType(DefinedFunction function) {

    }

    private void checkVariable(DefinedVariable variable) {

    }
}
