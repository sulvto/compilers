package interpreters;

import ast.*;
import entity.DefinedFunction;
import entity.DefinedVariable;

import java.util.List;
import java.util.Optional;

/**
 * Created by sulvto on 17-1-6.
 */
public class Interpreters {
    private AST root;
    Scope currentScope;

    class ReturnValue extends Error {
        public Object value;
    }

    private final ReturnValue RETURN_VALUE = new ReturnValue();


    LocalScope popScope() {
        Scope tmp = currentScope;
        currentScope = currentScope.parent();
        return (LocalScope) tmp;
    }

    void pushFunctionScope(List<DefinedVariable> definedVariables) {
        currentScope = new FunctionScope(currentScope.toplevel());
        definedVariables.forEach(((FunctionScope) currentScope)::defineVariable);
    }

    void pushLocalScope(List<DefinedVariable> definedVariables) {
        currentScope = new LocalScope(currentScope);
        definedVariables.forEach(((LocalScope) currentScope)::defineVariable);
    }

    public Object exec(Node node) {
//        switch (node.type()) {
//            case Type.:
//                block((BlockNode) node);
//                break;
//            case RETURN:
//                ret((ReturnNode) node);
//                break;
//            case ASSIGN:
//                assign(node);
//                break;
//            case CALL:
//                return call((FuncallNode) node);
//        }

        return null;
    }

    private void assign(Node node) {
        // TODO
    }

    private Object call(FuncallNode node) {
        // node variables
//        pushScope(variables);

        return null;
    }

    private void ret(ReturnNode node) {
        RETURN_VALUE.value = exec(node.getExpr());
        throw RETURN_VALUE;
    }

    public void block(BlockNode body) {
        List<DefinedVariable> variables = body.getVariables();
        pushLocalScope(variables);
        List<StmtNode> stmts = body.getStmts();
//        stmt()
    }

    private void stmt(StmtNode stmtNode) {
        // TODO
    }

    private DefinedFunction mainFun(AST ast) {
        Optional<DefinedFunction> main = ast.definedFunctions().stream().filter(definedFunction -> "main".equals(definedFunction.getName())).findFirst();
        if (main.isPresent()) {
            return main.get();
        }
        // TODO error info
        throw new Error("");
    }
}
