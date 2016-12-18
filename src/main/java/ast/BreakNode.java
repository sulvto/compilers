package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class BreakNode  extends StmtNode{

    public BreakNode(Location location) {
        super(location);
    }

    @Override
    protected void doDump(Dumper dumper) {
    }
}
