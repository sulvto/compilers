package compiler;

import ast.*;
import entity.*;
import exception.SemanticException;
import lexer.Token;
import util.ErrorHandler;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by sulvto on 17-11-1.
 */
public class LocalResolver extends Visitor {
    private final LinkedList<Scope> scopeStack;
    private final ConstantTable constantTable;
    private final ErrorHandler errorHandler;

    public LocalResolver(ErrorHandler errorHandler) {
        this.errorHandler = errorHandler;
        this.constantTable = new ConstantTable();
        this.scopeStack = new LinkedList<>();
    }


    public void resolve(AST ast) throws SemanticException {
        ToplevelScope toplevel = new ToplevelScope();
        scopeStack.add(toplevel);

        // handle
        for (Entity entity : ast.declarations()) {
            toplevel.declareEntity(entity);
        }
        for (Entity entity : ast.definitions()) {
            toplevel.defineEntity(entity);
        }
        resolveGvarInitializers(ast.definedVariables());
        resolveConstantValues(ast.constants());
        resolveFuncstions(ast.definedFunctions());
        if (errorHandler.errorOccured()) {
            throw new SemanticException("compile failed");
        }

        // save
        ast.setScope(toplevel);
        ast.setConstantTable(constantTable);
    }

    private void resolveGvarInitializers(List<DefinedVariable> gvars) {
        gvars.stream().filter(DefinedVariable::hasInitializer).forEach(definedVariable -> resolve(definedVariable.initializer()));
    }

    private void resolveConstantValues(List<Constant> constants) {
        constants.forEach(constant -> resolve(constant.value()));
    }

    private void resolveFuncstions(List<DefinedFunction> definedFunctions) {

        definedFunctions.forEach(fun -> {
            pushScope(fun.parameter());
            resolve(fun.getBody());
            fun.setScope(popScope());
        });
    }

    private void resolve(StmtNode node) {
        node.accept(this);
    }

    private void resolve(ExprNode node) {
        node.accept(this);
    }

    private LocalScope popScope() {
        return (LocalScope) scopeStack.removeLast();
    }

    private void pushScope(List<? extends DefinedVariable> vars) {
        LocalScope scope = new LocalScope(currentScope());
        for (DefinedVariable variable : vars) {
            if (scope.isDefinedLocally(variable.getName())) {
                error(variable.location(), "duplicated variable in scope:" + variable.getName());
            } else {
                scope.defineVariable(variable);
            }
        }
        scopeStack.addLast(scope);
    }

    private Scope currentScope() {
        return scopeStack.getLast();
    }

    @Override
    public Void visit(BlockNode node) {
        pushScope(node.getVariables());
        super.visit(node);
        node.setScope(popScope());
        return null;
    }

    @Override
    public Void visit(VariableNode node) {
        try {
            Entity ent = currentScope().get(node.name());
            ent.refered();
            node.setEntity(ent);
        } catch (SemanticException e) {
            error(node, e.getMessage());
        }
        return null;
    }

    private void error(String location, String message) {
        errorHandler.error(location, message);
    }

    private void error(Node node, String message) {
        error(node.token, message);
    }

    private void error(Token token, String message) {
        errorHandler.error(token, message);
    }
}
