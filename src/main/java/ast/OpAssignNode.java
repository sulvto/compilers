package ast;

/**
 * (a += 1)
 * Created by sulvto on 16-12-8.
 */
public class OpAssignNode extends AbstractAssignNode {
    private String oparator;

    public OpAssignNode(ExprNode left, String oparator, ExprNode right) {
        super(left, right);
        this.oparator = oparator;
    }

    @Override
    protected void doDump(Dumper dumper) {

    }
}
