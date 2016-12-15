package ast;

/**
 * (a = 1)
 * Created by sulvto on 16-12-8.
 */
public class AssignNode extends ExprNode {
    private String name;
    private ExprNode rhs;

    public AssignNode(String name, ExprNode rhs) {
        this.name = name;
        this.rhs = rhs;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
