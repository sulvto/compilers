package ast;

import entity.DefinedVariable;

import java.util.List;

/**
 * ({...})
 * Created by sulvto on 16-12-8.
 */
public class BlockNode extends StmtNode {

    private List<DefinedVariable> variables;
    private List<StmtNode> stmts;

    public BlockNode(List<DefinedVariable> variables, List<StmtNode> stmts) {
        this.variables = variables;
        this.stmts = stmts;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
