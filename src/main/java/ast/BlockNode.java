package ast;

import entity.DefinedVariable;
import entity.LocalScope;
import lexer.Token;

import java.util.List;

/**
 * ({...})
 * Created by sulvto on 16-12-8.
 */
public class BlockNode extends StmtNode {
    private List<DefinedVariable> variables;
    private List<StmtNode> stmts;
    private LocalScope scope;

    public BlockNode(Token token, List<DefinedVariable> variables, List<StmtNode> stmts) {
        super(token);
        this.variables = variables;
        this.stmts = stmts;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("variables", variables);
        dumper.printMember("stmts", stmts);
    }

    public List<DefinedVariable> getVariables() {
        return variables;
    }

    public List<StmtNode> getStmts() {
        return stmts;
    }

    public void setScope(LocalScope scope) {
        this.scope = scope;
    }
}
