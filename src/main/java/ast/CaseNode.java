package ast;

import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class CaseNode extends StmtNode {
    private List<ExprNode> values;
    private BlockNode body;


    public CaseNode(Location location, List<ExprNode> values, BlockNode body) {
        super(location);
        this.values = values;
        this.body = body;
    }

    @Override
    public Location location() {
        return null;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("values", values);
        dumper.printMember("body", body);
    }
}
