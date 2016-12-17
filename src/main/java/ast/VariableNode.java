package ast;

/**
 * Created by sulvto on 16-12-17.
 */
public class VariableNode extends LHSNode {
    private String name;

    public VariableNode(String name) {
        this.name = name;
    }

    @Override
    protected void doDump(Dumper dumper) {

    }
}
