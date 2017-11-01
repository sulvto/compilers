package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-17.
 */
public class VariableNode extends LHSNode {
    private String name;

    public VariableNode(Token token, String name) {
        super(token);
        this.name = name;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("name", name);
    }
}
