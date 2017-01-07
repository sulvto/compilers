package interpreters;

import ast.AST;
import ast.BlockNode;
import ast.StmtNode;
import entity.DefinedFunction;
import entity.DefinedVariable;

import java.util.List;
import java.util.Optional;

/**
 * Created by sulvto on 17-1-6.
 */
public class Interpreters {
    private AST root;

    public Object exec(AST ast) {
        DefinedFunction function = mainFun(ast);
        BlockNode body = function.getBody();
        return block(body);
    }

    public Object block(BlockNode body) {
        List<DefinedVariable> variables = body.getVariables();
        List<StmtNode> stmts = body.getStmts();
//        stmt()
    }

    private Object stmt(StmtNode stmtNode) {
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
