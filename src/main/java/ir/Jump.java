package ir;

import asm.Label;

/**
 * Created by sulvto on 17-11-3.
 */
public class Jump extends Stmt {
    private Label label;

    public Jump(String location, Label label) {
        super(location);
        this.label = label;
    }

    public Label label() {
        return label;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("label", label);
    }

    @Override
    public <S, E> S accept(IRVisitor<S, E> visitor) {
        return null;
    }
}
