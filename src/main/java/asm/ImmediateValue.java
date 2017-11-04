package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class ImmediateValue extends Operand {
    private Literal expr;

    public ImmediateValue(long n) {
        this(new IntegerLiteral(n));
    }

    public ImmediateValue(Literal expr) {
        this.expr = expr;
    }

    public Literal expr() {
        return expr;
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof ImmediateValue)) return false;
        ImmediateValue immediateValue = (ImmediateValue) obj;
        return expr.equals(immediateValue.expr());
    }

    @Override
    public String toSource(SymbolTable table) {
        return "$" + expr.toSource(table);
    }

    @Override
    public String dump() {
        return "(ImmediateValue " + expr.dump() + ")";
    }
}
