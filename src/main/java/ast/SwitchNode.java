package ast;

import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class SwitchNode extends StmtNode {
    private ExprNode cond;
    private List<CaseNode> cases;

    public SwitchNode(Location location, ExprNode cond, List<CaseNode> caseNodes) {
        super(location);
        this.cond = cond;
        this.cases = caseNodes;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("cond", cond);
        dumper.printMember("cases", cases);
    }
}
