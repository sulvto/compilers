package ir;

import asm.Type;

/**
 * Created by sulvto on 17-11-4.
 */
public class Int extends Expr {
    private long value;

    Int(Type type, long value) {
        super(type);
        this.value = value;
    }

    public long value() {
        return value;
    }

    @Override
    public boolean isConstant() {
        return true;
    }

    @Override
    public <S, E> E accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }

    @Override
    protected void duDump(Dumper dumper) {
        dumper.printMember("value", value);
    }
}
